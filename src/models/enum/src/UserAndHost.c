#include "UserAndHost.h"
#include <stdio.h>
#include "../color/colorPrint.h"
#include <windows.h>
#include <lm.h>  // NetUserEnum

#pragma comment(lib, "Netapi32.lib")

#define UNLEN 256
#define MAX_COMPUTERNAME_LENGTH 15

void getUser() {
    wchar_t username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;

    if (GetUserNameW(username, &username_len)) {
        wchar_t buffer[256];
        swprintf(buffer, 256, L"[+] Username Login : %ls\n", username);
        printSuccess(buffer);
    } else {
        wchar_t buffer[256];
        swprintf(buffer, 256, L"[-] Failed to get username. Error: %lu\n", GetLastError());
        printError(buffer);
    }
}

void getHost() {
    wchar_t hostname[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD hostname_len = MAX_COMPUTERNAME_LENGTH + 1;

    if (GetComputerNameW(hostname, &hostname_len)) {
        wchar_t buffer[256];
        swprintf(buffer, 256, L"[+] Hostname : %ls\n", hostname);
        printSuccess(buffer);
    } else {
        wchar_t buffer[256];
        swprintf(buffer, 256, L"[-] Failed to get hostname. Error: %lu\n", GetLastError());
        printError(buffer);
    }
}

// -----------------------------------------------------
// 🔵 FUNCTION: getAllUsers()
// -----------------------------------------------------
void getAllUsers() {
    LPUSER_INFO_0 pBuf = NULL;
    LPUSER_INFO_0 pTmpBuf;
    DWORD entriesRead = 0;
    DWORD totalEntries = 0;
    DWORD resumeHandle = 0;
    DWORD result;

    printInfo(L"[+] Listing all system users:");

    do {
        result = NetUserEnum(
            NULL,                 // local machine
            0,                    // USER_INFO_0
            FILTER_NORMAL_ACCOUNT,// Only real users
            (LPBYTE*)&pBuf,
            MAX_PREFERRED_LENGTH,
            &entriesRead,
            &totalEntries,
            &resumeHandle
        );

        if ((result == NERR_Success || result == ERROR_MORE_DATA) && pBuf != NULL) {
            pTmpBuf = pBuf;

            for (DWORD i = 0; i < entriesRead; i++) {
                if (pTmpBuf == NULL)
                    break;

                wchar_t buffer[300];
                swprintf(buffer, 300, L"    - %ls", pTmpBuf->usri0_name);
                printGreen(buffer);

                pTmpBuf++;
            }
        }

        if (pBuf != NULL) {
            NetApiBufferFree(pBuf);
            pBuf = NULL;
        }

    } while (result == ERROR_MORE_DATA);
}
