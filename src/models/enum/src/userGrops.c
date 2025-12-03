#include <windows.h>
#include <lm.h>
#include "UserAndHost.h"
#include "colorPrint.h"
#include <stdio.h>
#pragma comment(lib, "Netapi32.lib")


#define UNLEN 256

void getUserGroups() {
    wchar_t username[UNLEN + 1];
    DWORD username_len = UNLEN + 1;

    if (!GetUserNameW(username, &username_len)) {
        wchar_t buffer[256];
        swprintf(buffer, 256, L"[-] Failed to get username. Error: %lu", GetLastError());
        printError(buffer);
        return;
    }

    wchar_t buffer[256];
    swprintf(buffer, 256, L"[+] Group memberships for user: %ls", username);
    printInfo(buffer);

    LPLOCALGROUP_USERS_INFO_0 pBuf = NULL;
    DWORD entriesRead = 0, totalEntries = 0;

    NET_API_STATUS nStatus = NetUserGetLocalGroups(
        NULL,              // local computer
        username,          // username
        0,                 // level
        LG_INCLUDE_INDIRECT, // include nested groups
        (LPBYTE*)&pBuf,
        MAX_PREFERRED_LENGTH,
        &entriesRead,
        &totalEntries
    );

    if (nStatus == NERR_Success) {
        for (DWORD i = 0; i < entriesRead; i++) {
            wchar_t buf[256];
            swprintf(buf, 256, L"    - %ls", pBuf[i].lgrui0_name);
            printGreen(buf);
        }
        NetApiBufferFree(pBuf);
    } else {
        wchar_t buf[256];
        swprintf(buf, 256, L"[-] Failed to get groups. Status: %lu", nStatus);
        printError(buf);
    }
}


void getUsersAndGroups() {
    LPUSER_INFO_0 pBuf = NULL;
    LPUSER_INFO_0 pTmpBuf;
    DWORD entriesRead = 0;
    DWORD totalEntries = 0;
    DWORD resumeHandle = 0;
    DWORD result;

    printf("\n");
    printInfo(L"[+] Listing all users and their local group memberships:");

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
                if (pTmpBuf == NULL) break;

                wchar_t username[UNLEN + 1];
                wcscpy(username, pTmpBuf->usri0_name);

                wchar_t buf[256];
                swprintf(buf, 256, L"\n[+] User: %ls", username);
                printGreen(buf);

                // Get groups for this user
                LPLOCALGROUP_USERS_INFO_0 pGroups = NULL;
                DWORD groupsRead = 0, groupsTotal = 0;
                NET_API_STATUS nStatus = NetUserGetLocalGroups(
                    NULL, username, 0, LG_INCLUDE_INDIRECT,
                    (LPBYTE*)&pGroups, MAX_PREFERRED_LENGTH,
                    &groupsRead, &groupsTotal
                );

                if (nStatus == NERR_Success && pGroups != NULL) {
                    for (DWORD j = 0; j < groupsRead; j++) {
                        wchar_t gbuf[256];
                        swprintf(gbuf, 256, L"    - %ls", pGroups[j].lgrui0_name);
                        printBlue(gbuf);
                    }
                    NetApiBufferFree(pGroups);
                } else {
                    wchar_t gbuf[256];
                    swprintf(gbuf, 256, L"    [-] Failed to get groups. Status: %lu", nStatus);
                    printRed(gbuf);
                }

                pTmpBuf++;
            }
        }

        if (pBuf != NULL) {
            NetApiBufferFree(pBuf);
            pBuf = NULL;
        }

    } while (result == ERROR_MORE_DATA);
}
