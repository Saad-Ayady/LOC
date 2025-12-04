#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#include <shlwapi.h>
#include "serMis.h"
#include "../color/colorPrint.h"

#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "shlwapi.lib")

// ======================== Struct ========================
typedef struct {
    wchar_t name[256];
    wchar_t binaryPath[MAX_PATH];
    DWORD   startType;
    BOOL    canModify;
} SERVICE_MISCONFIG;

SERVICE_MISCONFIG services[1024];
int serviceCount = 0;

// ======================== Helpers ========================

void ExtractServicePath(const wchar_t* fullPath, wchar_t* pathOut) {
    wchar_t temp[MAX_PATH];
    wcscpy_s(temp, MAX_PATH, fullPath);

    // remove leading/trailing quotes
    PathUnquoteSpacesW(temp);

    // remove arguments after .exe
    wchar_t* exe = StrStrIW(temp, L".exe");
    if (exe) {
        exe[4] = L'\0'; // keep ".exe"
    }

    wcscpy_s(pathOut, MAX_PATH, temp);
}


BOOL IsServicePathWritable(const wchar_t* path) {
    wchar_t dir[MAX_PATH];
    wcscpy_s(dir, MAX_PATH, path);
    wchar_t* slash = wcsrchr(dir, L'\\');
    if (slash) *slash = L'\0';
    else return FALSE;

    HANDLE hFile;
    wchar_t testFile[MAX_PATH];
    swprintf_s(testFile, MAX_PATH, L"%s\\__test__.tmp", dir);

    hFile = CreateFileW(testFile, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS,
                        FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile != INVALID_HANDLE_VALUE) {
        CloseHandle(hFile);
        DeleteFileW(testFile);
        return TRUE;
    }
    return FALSE;
}

// ======================== Scan Services ========================

void ScanServiceMisconfigs() {
    SC_HANDLE hSCM = OpenSCManagerW(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!hSCM) {
        printError(L"[-] Failed to open Service Control Manager.");
        return;
    }

    DWORD needed = 0, returned = 0;
    EnumServicesStatusExW(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                          NULL, 0, &needed, &returned, NULL, NULL);

    BYTE* buffer = (BYTE*)malloc(needed);
    if (!buffer ||
        !EnumServicesStatusExW(hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32, SERVICE_STATE_ALL,
                               buffer, needed, &needed, &returned, NULL, NULL)) {
        printError(L"[-] Failed to enumerate services.");
        if (buffer) free(buffer);
        CloseServiceHandle(hSCM);
        return;
    }

    LPENUM_SERVICE_STATUS_PROCESSW svcArray = (LPENUM_SERVICE_STATUS_PROCESSW)buffer;

    for (DWORD i = 0; i < returned; i++) {
        SC_HANDLE hSvc = OpenServiceW(hSCM, svcArray[i].lpServiceName, SERVICE_QUERY_CONFIG);
        if (!hSvc) continue;

        QUERY_SERVICE_CONFIGW* config = (QUERY_SERVICE_CONFIGW*)malloc(4096);
        DWORD bytesNeeded;
        if (QueryServiceConfigW(hSvc, config, 4096, &bytesNeeded)) {

            wchar_t cleanPath[MAX_PATH];
            ExtractServicePath(config->lpBinaryPathName, cleanPath);

            BOOL writable = IsServicePathWritable(cleanPath);


            SERVICE_MISCONFIG* s = &services[serviceCount++];
            wcscpy_s(s->name, 256, svcArray[i].lpServiceName);
            wcscpy_s(s->binaryPath, MAX_PATH, config->lpBinaryPathName);
            s->startType = config->dwStartType;
            s->canModify = writable;
        }

        free(config);
        CloseServiceHandle(hSvc);
    }

    free(buffer);
    CloseServiceHandle(hSCM);
}

// ======================== Print Results ========================

void PrintServiceReport() {
    if (serviceCount == 0) {
        printInfo(L"No services found.");
        return;
    }

    for (int i = 0; i < serviceCount; i++) {
        if (services[i].canModify){
            printInfo(L"[+] Service Report\n");
            wprintf(L"    Name        : ");
            printBold(services[i].name);
            wprintf(L"\n");

            wprintf(L"    Binary Path : ");
            printYellow(services[i].binaryPath);
            wprintf(L"\n");

            wprintf(L"    Writable?   : ");
            printRed(L"YES (potential misconfig)");

            wprintf(L"\n");
            printInfo(L"-------------------------------------\n");
        } else {
            continue;
        }
       
    }
}
