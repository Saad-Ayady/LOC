#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <shlwapi.h>
#include "colorPrint.h"
#include "prosesRun.h"

#pragma comment(lib, "psapi.lib")
#pragma comment(lib, "shlwapi.lib")

// -----------------------------------------------------
// 🔵 FUNCTION: getProcessUserName()
// -----------------------------------------------------
wchar_t* getProcessUserName(DWORD processId) {
    static wchar_t username[256] = L"SYSTEM";
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    
    if (hProcess) {
        HANDLE hToken;
        if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
            DWORD size = sizeof(username) / sizeof(wchar_t);
            if (!GetTokenInformation(hToken, TokenUser, NULL, 0, &size)) {
                TOKEN_USER* tokenUser = (TOKEN_USER*)malloc(size);
                if (tokenUser) {
                    if (GetTokenInformation(hToken, TokenUser, tokenUser, size, &size)) {
                        SID_NAME_USE sidType;
                        wchar_t domain[256];
                        DWORD userSize = 256;
                        DWORD domainSize = 256;
                        
                        if (LookupAccountSidW(NULL, tokenUser->User.Sid, username, &userSize, 
                                             domain, &domainSize, &sidType)) {
                            // Success - username already in buffer
                        }
                    }
                    free(tokenUser);
                }
            }
            CloseHandle(hToken);
        }
        CloseHandle(hProcess);
    }
    
    return username;
}

// -----------------------------------------------------
// 🔵 FUNCTION: getProcessIntegrityLevel()
// -----------------------------------------------------
wchar_t* getProcessIntegrityLevel(DWORD processId) {
    static wchar_t integrity[64] = L"Unknown";
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    
    if (hProcess) {
        HANDLE hToken;
        if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
            DWORD size = 0;
            GetTokenInformation(hToken, TokenIntegrityLevel, NULL, 0, &size);
            
            TOKEN_MANDATORY_LABEL* tokenLabel = (TOKEN_MANDATORY_LABEL*)malloc(size);
            if (tokenLabel) {
                if (GetTokenInformation(hToken, TokenIntegrityLevel, tokenLabel, size, &size)) {
                    DWORD integrityLevel = *GetSidSubAuthority(tokenLabel->Label.Sid, 
                                                              (DWORD)(*GetSidSubAuthorityCount(tokenLabel->Label.Sid)-1));
                    
                    if (integrityLevel >= SECURITY_MANDATORY_SYSTEM_RID) {
                        wcscpy(integrity, L"System");
                    } else if (integrityLevel >= SECURITY_MANDATORY_HIGH_RID) {
                        wcscpy(integrity, L"High");
                    } else if (integrityLevel >= SECURITY_MANDATORY_MEDIUM_RID) {
                        wcscpy(integrity, L"Medium");
                    } else if (integrityLevel >= SECURITY_MANDATORY_LOW_RID) {
                        wcscpy(integrity, L"Low");
                    } else {
                        wcscpy(integrity, L"Untrusted");
                    }
                }
                free(tokenLabel);
            }
            CloseHandle(hToken);
        }
        CloseHandle(hProcess);
    }
    
    return integrity;
}

// -----------------------------------------------------
// 🔵 FUNCTION: getProcessPrivileges()
// -----------------------------------------------------
wchar_t* getProcessPrivileges(DWORD processId) {
    static wchar_t privileges[256] = L"Normal";
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    
    if (hProcess) {
        HANDLE hToken;
        if (OpenProcessToken(hProcess, TOKEN_QUERY, &hToken)) {
            // Check if token has admin privileges
            BOOL isElevated = FALSE;
            TOKEN_ELEVATION elevation;
            DWORD size = sizeof(TOKEN_ELEVATION);
            
            if (GetTokenInformation(hToken, TokenElevation, &elevation, size, &size)) {
                isElevated = elevation.TokenIsElevated;
            }
            
            if (isElevated) {
                wcscpy(privileges, L"Administrator");
            } else {
                // Check for specific privileges
                PRIVILEGE_SET privSet;
                privSet.PrivilegeCount = 1;
                privSet.Control = PRIVILEGE_SET_ALL_NECESSARY;
                
                LUID seDebugPrivilegeValue;
                if (LookupPrivilegeValueW(NULL, L"SeDebugPrivilege", &seDebugPrivilegeValue)) {
                    privSet.Privilege[0].Luid = seDebugPrivilegeValue;
                    privSet.Privilege[0].Attributes = SE_PRIVILEGE_ENABLED;
                    
                    BOOL result;
                    if (PrivilegeCheck(hToken, &privSet, &result) && result) {
                        wcscpy(privileges, L"Debug Privilege");
                    }
                }
            }
            CloseHandle(hToken);
        }
        CloseHandle(hProcess);
    }
    
    return privileges;
}

// -----------------------------------------------------
// 🔵 FUNCTION: getProcessMemoryUsage()
// -----------------------------------------------------
wchar_t* getProcessMemoryUsage(DWORD processId) {
    static wchar_t memory[128] = L"Unknown";
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    
    if (hProcess) {
        PROCESS_MEMORY_COUNTERS_EX pmc;
        if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc))) {
            // Convert to MB
            SIZE_T privateMB = pmc.PrivateUsage / (1024 * 1024);
            SIZE_T workingMB = pmc.WorkingSetSize / (1024 * 1024);
            
            swprintf(memory, 128, L"Private: %lu MB | Working Set: %lu MB", 
                    (DWORD)privateMB, (DWORD)workingMB);
        }
        CloseHandle(hProcess);
    }
    
    return memory;
}

// -----------------------------------------------------
// 🔵 FUNCTION: getProcessCommandLine()
// -----------------------------------------------------
wchar_t* getProcessCommandLine(DWORD processId) {
    static wchar_t cmdLine[1024] = L"Not available";
    
    // For Windows 10/11, we can use WMI or other methods, but here's a simpler approach
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processId);
    
    if (hProcess) {
        // Try to get executable path
        wchar_t exePath[MAX_PATH];
        if (GetModuleFileNameExW(hProcess, NULL, exePath, MAX_PATH)) {
            swprintf(cmdLine, 1024, L"Executable: %ls", exePath);
        }
        CloseHandle(hProcess);
    }
    
    return cmdLine;
}

// -----------------------------------------------------
// 🔵 FUNCTION: getProcessParentInfo()
// -----------------------------------------------------
wchar_t* getProcessParentInfo(DWORD processId) {
    static wchar_t parentInfo[512] = L"Unknown";
    
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32W pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32W);
        
        if (Process32FirstW(hSnapshot, &pe32)) {
            do {
                if (pe32.th32ProcessID == processId) {
                    // Find parent process name
                    HANDLE hParentSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
                    if (hParentSnapshot != INVALID_HANDLE_VALUE) {
                        PROCESSENTRY32W parentPe32;
                        parentPe32.dwSize = sizeof(PROCESSENTRY32W);
                        
                        if (Process32FirstW(hParentSnapshot, &parentPe32)) {
                            do {
                                if (parentPe32.th32ProcessID == pe32.th32ParentProcessID) {
                                    swprintf(parentInfo, 512, L"Parent: %ls (PID: %lu)", 
                                            parentPe32.szExeFile, parentPe32.th32ProcessID);
                                    break;
                                }
                            } while (Process32NextW(hParentSnapshot, &parentPe32));
                        }
                        CloseHandle(hParentSnapshot);
                    }
                    break;
                }
            } while (Process32NextW(hSnapshot, &pe32));
        }
        CloseHandle(hSnapshot);
    }
    
    return parentInfo;
}

// -----------------------------------------------------
// 🔵 FUNCTION: isProcess64Bit()
// -----------------------------------------------------
wchar_t* isProcess64Bit(DWORD processId) {
    static wchar_t arch[32] = L"Unknown";
    
    HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, processId);
    if (hProcess) {
        BOOL isWow64 = FALSE;
        if (IsWow64Process(hProcess, &isWow64)) {
            SYSTEM_INFO sysInfo;
            GetNativeSystemInfo(&sysInfo);
            
            if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64) {
                if (isWow64) {
                    wcscpy(arch, L"32-bit (WOW64)");
                } else {
                    wcscpy(arch, L"64-bit");
                }
            } else if (sysInfo.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL) {
                wcscpy(arch, L"32-bit");
            }
        }
        CloseHandle(hProcess);
    }
    
    return arch;
}

// -----------------------------------------------------
// 🔵 FUNCTION: getProcessSessionId()
// -----------------------------------------------------
DWORD getProcessSessionId(DWORD processId) {
    DWORD sessionId = 0;
    ProcessIdToSessionId(processId, &sessionId);
    return sessionId;
}

// -----------------------------------------------------
// 🔵 FUNCTION: getRunningProcesses()
// -----------------------------------------------------
void getDetailedRunningProcesses() {
    wchar_t buffer[1024];
    HANDLE hSnapshot;
    PROCESSENTRY32W pe32;

    printInfo(L" RUNNING PROCESSES");
    
    hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnapshot == INVALID_HANDLE_VALUE) {
        printError(L"[-] Failed to create process snapshot\n");
        return;
    }
    
    pe32.dwSize = sizeof(PROCESSENTRY32W);
    
    if (!Process32FirstW(hSnapshot, &pe32)) {
        printError(L"[-] Failed to get first process\n");
        CloseHandle(hSnapshot);
        return;
    }
    
    int processCount = 0;
    
    do {
        // Skip some system processes to keep output clean
        if (wcscmp(pe32.szExeFile, L"System") == 0 ||
            wcscmp(pe32.szExeFile, L"Idle") == 0 ||
            wcscmp(pe32.szExeFile, L"Registry") == 0 ||
            pe32.th32ProcessID == 0) {
            continue;
        }
        
        processCount++;
        
        // Process name header
        swprintf(buffer, 1024, L"    => %ls\n", pe32.szExeFile);
        printSuccess(buffer);
        
        // Get process path
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 
                                     FALSE, pe32.th32ProcessID);
        if (hProcess) {
            wchar_t processPath[MAX_PATH];
            if (GetModuleFileNameExW(hProcess, NULL, processPath, MAX_PATH)) {
                swprintf(buffer, 1024, L"             Process Path: %ls\n", processPath);
                printGreen(buffer);
            }
            CloseHandle(hProcess);
        }
        
        // Process ID
        swprintf(buffer, 1024, L"             Process ID: %lu\n", pe32.th32ProcessID);
        printGreen(buffer);
        
        // Parent Process ID
        swprintf(buffer, 1024, L"             Parent PID: %lu\n", pe32.th32ParentProcessID);
        printGreen(buffer);
        
        // Thread count
        swprintf(buffer, 1024, L"             Thread Count: %lu\n", pe32.cntThreads);
        printGreen(buffer);
        
        // Priority class
        swprintf(buffer, 1024, L"             Priority Base: %lu\n", pe32.pcPriClassBase);
        printGreen(buffer);
        
        // Session ID
        DWORD sessionId = getProcessSessionId(pe32.th32ProcessID);
        swprintf(buffer, 1024, L"             Session ID: %lu\n", sessionId);
        printGreen(buffer);
        
        // Architecture (32/64 bit)
        swprintf(buffer, 1024, L"             Architecture: %ls\n", isProcess64Bit(pe32.th32ProcessID));
        printGreen(buffer);
        
        // Running user
        swprintf(buffer, 1024, L"             Running User: %ls\n", getProcessUserName(pe32.th32ProcessID));
        printGreen(buffer);
        
        // Integrity level
        swprintf(buffer, 1024, L"             Integrity Level: %ls\n", getProcessIntegrityLevel(pe32.th32ProcessID));
        printGreen(buffer);
        
        // Privileges
        swprintf(buffer, 1024, L"             Privileges: %ls\n", getProcessPrivileges(pe32.th32ProcessID));
        printGreen(buffer);
        
        // Memory usage
        swprintf(buffer, 1024, L"             Memory Usage: %ls\n", getProcessMemoryUsage(pe32.th32ProcessID));
        printGreen(buffer);
        
        // Command line/path info
        swprintf(buffer, 1024, L"             Executable Info: %ls\n", getProcessCommandLine(pe32.th32ProcessID));
        printGreen(buffer);
        
        // Parent process info
        swprintf(buffer, 1024, L"             %ls\n", getProcessParentInfo(pe32.th32ProcessID));
        printGreen(buffer);
        
        // Separator
        if (processCount % 5 == 0) {
            printYellow(L"        --------------------\n");
        }
        
    } while (Process32NextW(hSnapshot, &pe32));
    
    CloseHandle(hSnapshot);
    
    // Summary
    swprintf(buffer, 1024, L"[+] Total Processes Found: %d\n", processCount);
    printInfo(buffer);
}