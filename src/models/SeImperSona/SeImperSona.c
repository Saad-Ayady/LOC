#include <windows.h>
#include <stdio.h>
#include "../color/colorPrint.h"

BOOL HasPrivilege(LPCSTR privName) {
    HANDLE hToken = NULL;
    DWORD size = 0;
    PTOKEN_PRIVILEGES privs = NULL;
    LUID luid;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        return FALSE;

    GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &size);
    privs = (PTOKEN_PRIVILEGES)malloc(size);
    if (!privs) {
        CloseHandle(hToken);
        return FALSE;
    }

    if (!GetTokenInformation(hToken, TokenPrivileges, privs, size, &size)) {
        free(privs);
        CloseHandle(hToken);
        return FALSE;
    }

    if (!LookupPrivilegeValueA(NULL, privName, &luid)) {
        free(privs);
        CloseHandle(hToken);
        return FALSE;
    }

    for (DWORD i = 0; i < privs->PrivilegeCount; i++) {
        if (privs->Privileges[i].Luid.LowPart == luid.LowPart &&
            privs->Privileges[i].Luid.HighPart == luid.HighPart) {
            free(privs);
            CloseHandle(hToken);
            return TRUE;
        }
    }

    free(privs);
    CloseHandle(hToken);
    return FALSE;
}

// ======================== MAIN ========================
void mainCheckSeImp() {
    printInfo(L"Checking for high-privilege tokens...\n");

    if (HasPrivilege(SE_IMPERSONATE_NAME)) {
        printSuccess(L"SeImpersonatePrivilege FOUND!\n");
        printGreen(L"You can perform privilege escalation (Juicy Potato, PrintSpoofer, etc.)\n");
        printYellow(L"This is a HIGH-VALUE token for local privilege escalation!\n");
    } else {
        printError(L"SeImpersonatePrivilege NOT found\n");
        printRed(L"Standard user token - LPE techniques requiring this privilege will fail\n");
    }
}