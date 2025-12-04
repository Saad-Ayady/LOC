#include <windows.h>
#include <stdio.h>
#include <shlwapi.h>
#include <psapi.h>  // ADD THIS LINE
#include "../color/colorPrint.h"
#include "installApps.h"

#pragma comment(lib, "shlwapi.lib")
#pragma comment(lib, "version.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "psapi.lib")  // ADD THIS LINE

// -----------------------------------------------------
// 🔵 FUNCTION: getAppInfoFromRegistry()
// -----------------------------------------------------
void getAppInfoFromRegistry(HKEY hKey, const wchar_t* subkey) {
    wchar_t buffer[1024];
    wchar_t appName[512];
    wchar_t displayName[512];
    wchar_t installLocation[1024];
    wchar_t publisher[512];
    wchar_t displayVersion[256];
    wchar_t uninstallString[1024];
    DWORD dwSize = 0;
    
    HKEY hAppKey;
    if (RegOpenKeyExW(hKey, subkey, 0, KEY_READ, &hAppKey) == ERROR_SUCCESS) {
        // Get DisplayName
        dwSize = sizeof(displayName);
        if (RegQueryValueExW(hAppKey, L"DisplayName", NULL, NULL, 
                            (LPBYTE)displayName, &dwSize) != ERROR_SUCCESS) {
            RegCloseKey(hAppKey);
            return; // Skip apps without DisplayName
        }
        
        // Skip system components and updates
        if (wcsstr(displayName, L"Update for") || 
            wcsstr(displayName, L"Security Update") ||
            wcsstr(displayName, L"Hotfix") ||
            wcsstr(displayName, L"KB") ||
            wcsstr(displayName, L"Microsoft .NET") ||
            wcsstr(displayName, L"Service Pack")) {
            RegCloseKey(hAppKey);
            return;
        }
        
        // Get other info
        dwSize = sizeof(installLocation);
        RegQueryValueExW(hAppKey, L"InstallLocation", NULL, NULL, 
                        (LPBYTE)installLocation, &dwSize);
        
        dwSize = sizeof(publisher);
        RegQueryValueExW(hAppKey, L"Publisher", NULL, NULL, 
                        (LPBYTE)publisher, &dwSize);
        
        dwSize = sizeof(displayVersion);
        RegQueryValueExW(hAppKey, L"DisplayVersion", NULL, NULL, 
                        (LPBYTE)displayVersion, &dwSize);
        
        dwSize = sizeof(uninstallString);
        RegQueryValueExW(hAppKey, L"UninstallString", NULL, NULL, 
                        (LPBYTE)uninstallString, &dwSize);
        
        // Format output
        swprintf(buffer, 1024, L"    %-40ls | Version: %-12ls | Publisher: %ls\n", 
                displayName, displayVersion[0] ? displayVersion : L"N/A", 
                publisher[0] ? publisher : L"Unknown");
        printGreen(buffer);
        
        if (installLocation[0]) {
            swprintf(buffer, 1024, L"        Path: %ls\n", installLocation);
            printGreen(buffer);
        }
        
        RegCloseKey(hAppKey);
    }
}

// -----------------------------------------------------
// 🔵 FUNCTION: getAppInfoFromExecutable()
// -----------------------------------------------------
void getAppInfoFromExecutable(const wchar_t* path) {
    wchar_t buffer[1024];
    DWORD dwHandle = 0;
    DWORD dwSize = GetFileVersionInfoSizeW(path, &dwHandle);
    
    if (dwSize > 0) {
        BYTE* pVersionInfo = (BYTE*)malloc(dwSize);
        if (GetFileVersionInfoW(path, dwHandle, dwSize, pVersionInfo)) {
            wchar_t* productName = NULL;
            wchar_t* companyName = NULL;
            wchar_t* fileVersion = NULL;
            UINT len = 0;
            
            // Get product name
            if (VerQueryValueW(pVersionInfo, L"\\StringFileInfo\\040904b0\\ProductName", 
                              (LPVOID*)&productName, &len) && productName) {
                swprintf(buffer, 1024, L"    %-40ls | Executable: %ls\n", 
                        productName, PathFindFileNameW(path));
                printGreen(buffer);
            }
            
            // Get company name
            if (VerQueryValueW(pVersionInfo, L"\\StringFileInfo\\040904b0\\CompanyName", 
                              (LPVOID*)&companyName, &len) && companyName) {
                swprintf(buffer, 1024, L"        Company: %ls\n", companyName);
                printGreen(buffer);
            }
        }
        free(pVersionInfo);
    }
}

// -----------------------------------------------------
// 🔵 FUNCTION: getProgramFilesApps()
// -----------------------------------------------------
void getProgramFilesApps() {
    wchar_t buffer[1024];
    WIN32_FIND_DATAW findData;
    HANDLE hFind;
    
    // Common Program Files paths
    const wchar_t* programPaths[] = {
        L"C:\\Program Files\\*",
        L"C:\\Program Files (x86)\\*",
        L"%ProgramFiles%\\*",
        L"%ProgramFiles(x86)%\\*"
    };
    
    for (int i = 0; i < 4; i++) {
        wchar_t expandedPath[MAX_PATH];
        ExpandEnvironmentStringsW(programPaths[i], expandedPath, MAX_PATH);
        
        hFind = FindFirstFileW(expandedPath, &findData);
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
                    if (wcscmp(findData.cFileName, L".") != 0 && 
                        wcscmp(findData.cFileName, L"..") != 0) {
                        
                        // Check for executables in the directory
                        wchar_t exePath[MAX_PATH];
                        swprintf(exePath, MAX_PATH, L"%s\\%s\\*.exe", 
                                expandedPath, findData.cFileName);
                        
                        WIN32_FIND_DATAW exeData;
                        HANDLE hExeFind = FindFirstFileW(exePath, &exeData);
                        if (hExeFind != INVALID_HANDLE_VALUE) {
                            swprintf(buffer, 1024, L"    %-40ls | Folder: %ls\n", 
                                    findData.cFileName, expandedPath);
                            printGreen(buffer);
                            FindClose(hExeFind);
                        }
                    }
                }
            } while (FindNextFileW(hFind, &findData));
            FindClose(hFind);
        }
    }
}

// -----------------------------------------------------
// 🔵 FUNCTION: getRunningProcesses()
// -----------------------------------------------------
void getRunningProcesses() {
    wchar_t buffer[1024];
    DWORD processes[1024], cbNeeded, cProcesses;
    
    printInfo(L"[+] Currently Running Applications:");
    
    if (!EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        printError(L"[-] Failed to enumerate processes\n");
        return;
    }
    
    cProcesses = cbNeeded / sizeof(DWORD);
    
    for (DWORD i = 0; i < cProcesses; i++) {
        if (processes[i] != 0) {
            HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
                                         FALSE, processes[i]);
            if (hProcess != NULL) {
                HMODULE hMod;
                DWORD cbNeededMod;
                
                if (EnumProcessModules(hProcess, &hMod, sizeof(hMod), &cbNeededMod)) {
                    wchar_t szProcessName[MAX_PATH];
                    if (GetModuleFileNameExW(hProcess, hMod, szProcessName, 
                                           sizeof(szProcessName) / sizeof(wchar_t))) {
                        wchar_t* exeName = PathFindFileNameW(szProcessName);
                        
                        // Skip system processes
                        if (wcscmp(exeName, L"svchost.exe") != 0 &&
                            wcscmp(exeName, L"csrss.exe") != 0 &&
                            wcscmp(exeName, L"wininit.exe") != 0 &&
                            wcscmp(exeName, L"services.exe") != 0 &&
                            wcscmp(exeName, L"lsass.exe") != 0 &&
                            wcscmp(exeName, L"winlogon.exe") != 0) {
                            
                            swprintf(buffer, 1024, L"    %-40ls | PID: %lu\n", 
                                    exeName, processes[i]);
                            printGreen(buffer);
                        }
                    }
                }
                CloseHandle(hProcess);
            }
        }
    }
}

// -----------------------------------------------------
// 🔵 FUNCTION: getInstalledApps()
// -----------------------------------------------------
void getInstalledApps() {
    wchar_t buffer[1024];
    HKEY hKey;
    DWORD dwIndex = 0;
    wchar_t subkey[512];
    DWORD dwSize = 512;
    
    printInfo(L" INSTALLED APPLICATIONS");
    
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                     L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        printInfo(L"[+] 64-bit Applications:");
        
        while (RegEnumKeyExW(hKey, dwIndex, subkey, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            getAppInfoFromRegistry(hKey, subkey);
            dwIndex++;
            dwSize = 512;
        }
        RegCloseKey(hKey);
    }
    
    // 32-bit applications on 64-bit Windows
    dwIndex = 0;
    if (RegOpenKeyExW(HKEY_LOCAL_MACHINE, 
                     L"SOFTWARE\\WOW6432Node\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        printInfo(L"[+] 32-bit Applications:");
        
        while (RegEnumKeyExW(hKey, dwIndex, subkey, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            getAppInfoFromRegistry(hKey, subkey);
            dwIndex++;
            dwSize = 512;
        }
        RegCloseKey(hKey);
    }
    
    // Current user applications
    dwIndex = 0;
    if (RegOpenKeyExW(HKEY_CURRENT_USER, 
                     L"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        printInfo(L"[+] Current User Applications:");
        
        while (RegEnumKeyExW(hKey, dwIndex, subkey, &dwSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            getAppInfoFromRegistry(hKey, subkey);
            dwIndex++;
            dwSize = 512;
        }
        RegCloseKey(hKey);
    }
    
    // Get Program Files applications
    printInfo(L"[+] Applications in Program Files:");
    getProgramFilesApps();
    
    // Get running processes
    getRunningProcesses();
}

// -----------------------------------------------------
// 🔵 FUNCTION: getAppPrivileges()
// -----------------------------------------------------
void getAppPrivileges(const wchar_t* appName, const wchar_t* appPath) {
    wchar_t buffer[1024];
    
    if (appPath[0]) {
        // Check if app requires admin privileges
        SHELLEXECUTEINFOW sei = {0};
        sei.cbSize = sizeof(sei);
        sei.lpVerb = L"runas";  // "runas" verb requests elevation
        sei.lpFile = appPath;
        sei.nShow = SW_HIDE;
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        
        // Try to execute with admin privileges (will fail if not allowed)
        if (!ShellExecuteExW(&sei)) {
            DWORD err = GetLastError();
            if (err == ERROR_CANCELLED) {
                swprintf(buffer, 1024, L"        Requires Admin: Yes (User cancelled)\n");
            } else if (err == ERROR_ACCESS_DENIED) {
                swprintf(buffer, 1024, L"        Requires Admin: Yes (Access denied)\n");
            } else {
                swprintf(buffer, 1024, L"        Requires Admin: Unknown (Error: %lu)\n", err);
            }
        } else {
            swprintf(buffer, 1024, L"        Requires Admin: No\n");
            if (sei.hProcess) {
                TerminateProcess(sei.hProcess, 0);
                CloseHandle(sei.hProcess);
            }
        }
        printGreen(buffer);
    }
}