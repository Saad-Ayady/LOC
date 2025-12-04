#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include "../color/colorPrint.h"

#pragma comment(lib, "advapi32.lib")


// ======================== Unquoted Path Check ========================
BOOL IsUnquotedVuln(const wchar_t* path) {
    if (!path || path[0] == L'\"') return FALSE;
    if (wcschr(path, L' ') == NULL) return FALSE;
    if (wcsstr(path, L"\\system32\\") || wcsstr(path, L"\\System32\\")) return FALSE;
    if (wcsstr(path, L"\\syswow64\\") || wcsstr(path, L"\\SysWOW64\\")) return FALSE;
    return TRUE;
}

// ======================== Enumeration ========================
typedef struct {
    wchar_t name[256];
    wchar_t path[512];
} VULN;

VULN vulns[1024];
int vulnCount = 0;

void FindUnquotedServices() {
    SC_HANDLE hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!hSCM) return;

    DWORD needed = 0, ret = 0;
    EnumServicesStatusExW(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                          NULL, 0, &needed, &ret, NULL, NULL);

    BYTE* buf = (BYTE*)malloc(needed);
    if (!EnumServicesStatusExW(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                               buf, needed, &needed, &ret, NULL, NULL)) {
        free(buf); CloseServiceHandle(hSCM); return;
    }

    LPENUM_SERVICE_STATUS_PROCESSW svc = (LPENUM_SERVICE_STATUS_PROCESSW)buf;

    for (DWORD i = 0; i < ret; i++) {
        SC_HANDLE hSvc = OpenServiceW(hSCM, svc[i].lpServiceName, SERVICE_QUERY_CONFIG);
        if (!hSvc) continue;

        DWORD sz = 0;
        QueryServiceConfigW(hSvc, NULL, 0, &sz);
        QUERY_SERVICE_CONFIGW* cfg = (QUERY_SERVICE_CONFIGW*)malloc(sz);

        if (QueryServiceConfigW(hSvc, cfg, sz, &sz)) {
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



void runUspPathCheck() {


    FindUnquotedServices();

    if (vulnCount == 0) {
        printGreen(L"No Unquoted Service Paths found - System is clean!\n");
    } else {
        for (int i = 0; i < vulnCount; i++) {
            printInfo(vulns[i].name);
            wprintf(L"              srves name : ");
            printBold(vulns[i].name);
            wprintf(L"\n");

            wprintf(L"              ur privleg in this path : ");
            if (hasImp) printGreen(L"YES (SeImpersonatePrivilege)"); 
            else        printRed(L"NO");
            wprintf(L"\n");

            wprintf(L"              serves path : ");
            printYellow(vulns[i].path);
            wprintf(L"\n");

            printf("\n");
        }

        if (hasImp)
            printBlue(L"You have SeImpersonate → Use GodPotato / RogueWinRM / PrintSpoofer = SYSTEM!\n");
        else
            printYellow(L"Get SeImpersonate first → then come back and own this box!\n");
    }

}

int main() {
    runUspPathCheck();
    return 0;
}