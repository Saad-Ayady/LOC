#include <windows.h>
#include <stdio.h>
#include <wchar.h>

#include "../color/colorPrint.h"

#pragma comment(lib, "advapi32.lib")

// ======================== Helper → Extract Directory ========================
void GetDirectoryFromPath(const wchar_t* fullPath, wchar_t* outDir) {
    wcscpy(outDir, fullPath);
    for (int i = wcslen(outDir) - 1; i >= 0; i--) {
        if (outDir[i] == L'\\') {
            outDir[i] = L'\0';
            return;
        }
    }
}

// ======================== Check Write Privilege ============================
BOOL CanWriteToPath(const wchar_t* binaryPath) {
    wchar_t dir[512];
    GetDirectoryFromPath(binaryPath, dir);

    if (wcslen(dir) == 0)
        return FALSE;

    wchar_t testFile[600];
    swprintf(testFile, 600, L"%s\\_loc_test.tmp", dir);

    HANDLE h = CreateFileW(
        testFile,
        GENERIC_WRITE,
        FILE_SHARE_READ,
        NULL,
        CREATE_ALWAYS,
        FILE_ATTRIBUTE_NORMAL,
        NULL
    );

    if (h == INVALID_HANDLE_VALUE)
        return FALSE;

    CloseHandle(h);
    DeleteFileW(testFile);
    return TRUE;
}

// ======================== Unquoted Path Check ========================
BOOL IsUnquotedVuln(const wchar_t* path) {
    if (!path || path[0] == L'\"') return FALSE;
    if (wcschr(path, L' ') == NULL) return FALSE;
    if (wcsstr(path, L"\\system32\\") || wcsstr(path, L"\\System32\\")) return FALSE;
    if (wcsstr(path, L"\\syswow64\\") || wcsstr(path, L"\\SysWOW64\\")) return FALSE;
    return TRUE;
}

// ======================== Struct & Storage ==========================
typedef struct {
    wchar_t name[256];
    wchar_t path[512];
} VULN;

static VULN vulns[1024];
static int vulnCount = 0;

// ======================== Enumeration ===============================
void FindUnquotedServices() {
    SC_HANDLE hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!hSCM) return;

    DWORD needed = 0, ret = 0;
    EnumServicesStatusExW(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                          NULL, 0, &needed, &ret, NULL, NULL);

    BYTE* buf = (BYTE*)malloc(needed);
    if (!buf) return;

    if (!EnumServicesStatusExW(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                               buf, needed, &needed, &ret, NULL, NULL)) {
        free(buf); 
        CloseServiceHandle(hSCM); 
        return;
    }

    LPENUM_SERVICE_STATUS_PROCESSW svc = (LPENUM_SERVICE_STATUS_PROCESSW)buf;

    for (DWORD i = 0; i < ret; i++) {
        SC_HANDLE hSvc = OpenServiceW(hSCM, svc[i].lpServiceName, SERVICE_QUERY_CONFIG);
        if (!hSvc) continue;

        DWORD sz = 0;
        QueryServiceConfigW(hSvc, NULL, 0, &sz);
        QUERY_SERVICE_CONFIGW* cfg = (QUERY_SERVICE_CONFIGW*)malloc(sz);

        if (cfg && QueryServiceConfigW(hSvc, cfg, sz, &sz)) {
            if (IsUnquotedVuln(cfg->lpBinaryPathName)) {
                VULN* v = &vulns[vulnCount++];
                wcscpy_s(v->name, 256, svc[i].lpServiceName);
                wcscpy_s(v->path, 512, cfg->lpBinaryPathName);
            }
        }

        free(cfg);
        CloseServiceHandle(hSvc);
    }

    free(buf);
    CloseServiceHandle(hSCM);
}

// ======================== MAIN OUTPUT ==============================
void runUspPathCheck() {

    FindUnquotedServices();

    if (vulnCount == 0) {
        printGreen(L"No Unquoted Service Paths found - System is clean!\n");
        return;
    }

    for (int i = 0; i < vulnCount; i++) {
        printInfo(vulns[i].name);
        wprintf(L"              service name : ");
        printBold(vulns[i].name);
        wprintf(L"\n");

        // ---- WRITE PRIV CHECK ----------
        BOOL canWrite = CanWriteToPath(vulns[i].path);

        wprintf(L"              writable path : ");
        if (canWrite) printGreen(L"YES (you can drop payload)");
        else          printRed(L"NO");
        wprintf(L"\n");

        wprintf(L"              service path : ");
        printYellow(vulns[i].path);
        wprintf(L"\n\n");
    }
}
