#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <shlwapi.h>
#include "../color/colorPrint.h"

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")

// ======================== Global ========================
typedef struct {
    wchar_t name[256];
    wchar_t dllPath[512];
    BOOL    writable;
} DLL_HIJACK;

DLL_HIJACK hijacks[1024];
int hijackCount = 0;


// ======================== Is Directory Writable ========================
BOOL IsDirectoryWritable(const wchar_t* dirPath) {
    if (!dirPath || dirPath[0] == 0) return FALSE;

    wchar_t testFile[MAX_PATH];
    wcscpy_s(testFile, MAX_PATH, dirPath);
    PathAddBackslashW(testFile);
    wcscat_s(testFile, MAX_PATH, L"___dllhijack_test___.tmp");

    HANDLE hFile = CreateFileW(testFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                               FILE_ATTRIBUTE_NORMAL, NULL);

    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        DeleteFileW(testFile);
        return TRUE;
    }
    return FALSE;
}


// ======================== Extract Directory ========================
void ExtractDllDirectory(const wchar_t* fullPath, wchar_t* dirOut) {
    wcscpy_s(dirOut, MAX_PATH, fullPath);

    wchar_t* slash = wcsrchr(dirOut, L'\\');
    if (slash)
        *slash = L'\0';
    else
        dirOut[0] = L'\0';
}


// ======================== Main Scan ========================
void FindDllHijackServices() {

    SC_HANDLE hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!hSCM) {
        printError(L"[-] Cannot open SCM (run as admin?)");
        return;
    }

    DWORD needed = 0, returned = 0;
    EnumServicesStatusExW(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                          NULL, 0, &needed, &returned, NULL, NULL);

    BYTE* buffer = (BYTE*)malloc(needed);
    if (!buffer ||
        !EnumServicesStatusExW(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                               buffer, needed, &needed, &returned, NULL, NULL)) {

        printError(L"[-] EnumServicesStatusExW failed");
        if (buffer) free(buffer);
        CloseServiceHandle(hSCM);
        return;
    }

    LPENUM_SERVICE_STATUS_PROCESSW services = (LPENUM_SERVICE_STATUS_PROCESSW)buffer;

    for (DWORD i = 0; i < returned; i++) {

        SC_HANDLE hSvc = OpenServiceW(hSCM, services[i].lpServiceName, SERVICE_QUERY_CONFIG);
        if (!hSvc) continue;

        HKEY hKey;
        wchar_t regPath[512];
        swprintf_s(regPath, 512,
                   L"SYSTEM\\CurrentControlSet\\Services\\%ls\\Parameters",
                   services[i].lpServiceName);

        wchar_t dllPath[MAX_PATH] = {0};
        BOOL found = FALSE;

        if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, regPath, 0, KEY_READ, &hKey) == ERROR_SUCCESS) {

            DWORD type = REG_SZ, size = sizeof(dllPath);
            if (RegQueryValueExW(hKey, L"ServiceDll", NULL, &type,
                                 (LPBYTE)dllPath, &size) == ERROR_SUCCESS &&
                dllPath[0] != L'\0') {

                found = TRUE;
            }

            RegCloseKey(hKey);
        }

        if (found) {

            ExpandEnvironmentStringsW(dllPath, dllPath, MAX_PATH);

            wchar_t dirPath[MAX_PATH] = {0};
            ExtractDllDirectory(dllPath, dirPath);

            if (dirPath[0] == L'\0') {
                CloseServiceHandle(hSvc);
                continue;
            }

            if (IsDirectoryWritable(dirPath)) {
                DLL_HIJACK* h = &hijacks[hijackCount++];
                wcscpy_s(h->name, 256, services[i].lpServiceName);
                wcscpy_s(h->dllPath, 512, dllPath);
                h->writable = TRUE;
            }
        }

        CloseServiceHandle(hSvc);
    }

    free(buffer);
    CloseServiceHandle(hSCM);
}


// ======================== Run Scan ========================
void runDllHijackCheck() {

    printInfo(L"Scanning for writable ServiceDll paths...\n\n");

    FindDllHijackServices();

    if (hijackCount == 0) {
        printError(L"No writable ServiceDll paths found.");
    }
    else {
        for (int i = 0; i < hijackCount; i++) {

            printInfo(L"[+] Vulnerable Service Found:\n");

            wprintf(L"        Service Name : ");
            printBold(hijacks[i].name);
            wprintf(L"\n");

            wprintf(L"        Service DLL  : ");
            printYellow(hijacks[i].dllPath);
            wprintf(L"\n");

            printInfo(L"---------------------------------------------\n");
        }
    }
}

