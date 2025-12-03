#include <stdio.h>
#include <windows.h>
#include <versionhelpers.h>
#include "colorPrint.h"

#pragma comment(lib, "Version.lib")

// -----------------------------------------------------
// 🔵 FUNCTION: printBasicOSInfo
// -----------------------------------------------------
void printBasicOSInfo() {
    wchar_t buffer[512];

    // OS Name from Registry
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                      L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t productName[256] = {0};
        DWORD size = sizeof(productName);
        if (RegQueryValueExW(hKey, L"ProductName", NULL, NULL, (LPBYTE)productName, &size) == ERROR_SUCCESS) {
            swprintf(buffer, 512, L"[+] OS Name: %ls", productName);
            printSuccess(buffer);
        }

        wchar_t releaseId[256] = {0};
        size = sizeof(releaseId);
        if (RegQueryValueExW(hKey, L"ReleaseId", NULL, NULL, (LPBYTE)releaseId, &size) == ERROR_SUCCESS) {
            swprintf(buffer, 512, L"[+] Release ID: %ls", releaseId);
            printInfo(buffer);
        }

        wchar_t displayVersion[256] = {0};
        size = sizeof(displayVersion);
        if (RegQueryValueExW(hKey, L"DisplayVersion", NULL, NULL, (LPBYTE)displayVersion, &size) == ERROR_SUCCESS) {
            swprintf(buffer, 512, L"[+] Display Version: %ls", displayVersion);
            printInfo(buffer);
        }

        RegCloseKey(hKey);
    } else {
        printError(L"[-] Failed to open registry for OS info");
    }

    // OS Version (deprecated but works)
    OSVERSIONINFOEXW osvi = {0};
    osvi.dwOSVersionInfoSize = sizeof(osvi);
    if (GetVersionExW((OSVERSIONINFOW*)&osvi)) {
        swprintf(buffer, 512, L"[+] OS Version: %lu.%lu.%lu", osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
        printInfo(buffer);
    }

    // OS Detection
    swprintf(buffer, 512, L"[+] Windows 10 or newer: %s", IsWindows10OrGreater() ? L"Yes" : L"No");
    printInfo(buffer);
    swprintf(buffer, 512, L"[+] Windows 8.1 or newer: %s", IsWindows8Point1OrGreater() ? L"Yes" : L"No");
    printInfo(buffer);
    swprintf(buffer, 512, L"[+] Windows 7 or newer: %s", IsWindows7OrGreater() ? L"Yes" : L"No");
    printInfo(buffer);
}

void printArchitecture() {
    wchar_t buffer[256];
    SYSTEM_INFO sysInfo;
    GetNativeSystemInfo(&sysInfo);

    switch (sysInfo.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64: swprintf(buffer, 256, L"[+] Architecture: x64 (64-bit)"); break;
        case PROCESSOR_ARCHITECTURE_INTEL: swprintf(buffer, 256, L"[+] Architecture: x86 (32-bit)"); break;
        case PROCESSOR_ARCHITECTURE_ARM: swprintf(buffer, 256, L"[+] Architecture: ARM"); break;
        case PROCESSOR_ARCHITECTURE_ARM64: swprintf(buffer, 256, L"[+] Architecture: ARM64"); break;
        case PROCESSOR_ARCHITECTURE_IA64: swprintf(buffer, 256, L"[+] Architecture: Intel Itanium"); break;
        default: swprintf(buffer, 256, L"[+] Architecture: Unknown (0x%X)", sysInfo.wProcessorArchitecture); break;
    }
    printSuccess(buffer);

    BOOL isWow64 = FALSE;
    IsWow64Process(GetCurrentProcess(), &isWow64);
    if (isWow64) printInfo(L"[+] Running under WOW64 (32-bit on 64-bit Windows)");
}

void printBuildInfo() {
    wchar_t buffer[512];
    HKEY hKey;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                      L"SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion", 
                      0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        wchar_t buildLab[256] = {0}, buildLabEx[256] = {0}, currentBuild[256] = {0}, currentBuildNumber[256] = {0};
        DWORD size;

        size = sizeof(buildLab);
        RegQueryValueExW(hKey, L"BuildLab", NULL, NULL, (LPBYTE)buildLab, &size);
        swprintf(buffer, 512, L"[+] Build Lab: %ls", buildLab); printInfo(buffer);

        size = sizeof(buildLabEx);
        RegQueryValueExW(hKey, L"BuildLabEx", NULL, NULL, (LPBYTE)buildLabEx, &size);
        swprintf(buffer, 512, L"[+] Build Lab Ex: %ls", buildLabEx); printInfo(buffer);

        size = sizeof(currentBuild);
        RegQueryValueExW(hKey, L"CurrentBuild", NULL, NULL, (LPBYTE)currentBuild, &size);
        swprintf(buffer, 512, L"[+] Current Build: %ls", currentBuild); printInfo(buffer);

        size = sizeof(currentBuildNumber);
        RegQueryValueExW(hKey, L"CurrentBuildNumber", NULL, NULL, (LPBYTE)currentBuildNumber, &size);
        swprintf(buffer, 512, L"[+] Current Build Number: %ls", currentBuildNumber); printInfo(buffer);

        RegCloseKey(hKey);
    } else {
        printError(L"[-] Failed to read build info from registry");
    }
}

void printDisplayInfo() {
    wchar_t buffer[128];
    swprintf(buffer, 128, L"[+] Desktop Resolution: %dx%d", GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN));
    printInfo(buffer);
}

