#include <windows.h>
#include <stdio.h>
#include <lm.h>
#include <iphlpapi.h>
#include <tlhelp32.h>
#include <psapi.h>
#include <tchar.h>
#include <aclapi.h>
#include <winsvc.h>
#pragma comment(lib, "netapi32.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "advapi32.lib")
#pragma comment(lib, "psapi.lib")

#define APP_V "1.0.0"
#define APP_N "LOC APP"
#define APP_A "0xdy"

#define INFO_FILE "system_info.txt"
#define MAX_SERVICES 256


// codes for attackes for privleg 
/*
    1. try get info like OSCP 
        - Username and hostname 
        - Group memberships of the current user 
        - Existing users and groups 
        - Operating system, version and architecture 
        - Network information 
        - Installed applications 
        - Running processes
    2. get info if this acc accetivate SeImpersonatePrivilege 
    3. if yes try to get SYSTEM privlege by token impersonation
    4. search `Unquoted Service Paths` misconfigurations and exploit them to get SYSTEM privlege
    5. find service DLL Hijacking vulnerabilities and exploit them to get SYSTEM privlege
    6. leveraging windows services misconfigurations to get SYSTEM privlege
    7. get information from information goldmine powershell and save passwords in file
    8. search files with sensitive info like passwords
    9. try to bruteforce password of SYSTEM and all users in server from local passwords
    10. try to exploit known kernel vulnerabilities to get SYSTEM privlege
*/

typedef struct {
    char username[256];
    char hostname[256];

    char groups[128][256];
    int group_count;

    char users[128][256];
    int user_count;

    char local_groups[128][256];
    int local_group_count;

    char os_version[128];
    char os_arch[32];

    char adapters[32][256];
    int adapter_count;

    char installed_apps[512][256];
    int installed_apps_count;

    char processes[256][256];
    int process_count;

} SystemInfo;

SystemInfo sysInfo;


// PRINTING FUNCTIONS WITH COLOR
void PrintW(const wchar_t* text) {
    DWORD written;
    WriteConsoleW(GetStdHandle(STD_OUTPUT_HANDLE), text, wcslen(text), &written, NULL);
}

void SetColor(WORD color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void PrintPanel() {
    HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);

    SetColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    puts("*====================================");
    SetColor(FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    PrintW(L"  ⠀⠀⠀⣠⠶⡒⠒⢬⡲⣮⠂⣆⣀⠀⠀⠀⠀⠀⠀⢀⣤⣴⣦⣤⡀⠀⠀⠀⠀   \n");
    PrintW(L" ⠀⠀⠀⣀⣥⠠⣿⠆⠐⣻⣾⣿⣿⢷⡄⠀⠀⠀⠀⢠⡿⠋⠉⠉⠙⢿⡄⠀⠀⠀  \n");
    PrintW(L" ⠀⠀⢘⡵⢋⠄⡙⠒⣤⣄⣉⠙⣿⣗⠑⡄⠀⠀⠀⠘⡇⠀⠀⠀⠀⠈⡇⠀⠀⠀   \n");
    PrintW(L" ⠀⣴⢿⡜⢡⡞⢀⢼⣿⣿⣿⣿⣿⣿⠟⣂⠀⠀⢀⣀⠱⡀⠀⠀⠀⢰⠁⠀⠀⠀  \n");
    PrintW(L" ⠰⢫⢟⡇⢸⡇⢸⢾⣿⣿⣿⣿⣿⣿⡷⠰⠀⢰⡏⠀⠀⢡⠀⠀⢠⠃⠀⠀⠀⠀\n") ;
    PrintW(L" ⢰⠁⣿⢣⣿⠇⢀⣿⣿⡿⠿⠤⣭⣥⣶⡆⠀⠸⣷⣤⣠⡾⠀⢀⡇⠀⠀⠀⠀⠀\n");
    PrintW(L"  ⡞⣰⣧⠟⡝⢸⢸⣿⣥⠖⣴⡆⣤⣬⠉⠀⠀⠀⠈⠉⠉⠀⠀⢸⣇⠀⠀⠀⠀⠀\n");
    PrintW(L" ⠀⡿⡟⢸⡇⠸⡄⢹⣿⢸⣿⣇⡏⠟⣰⣄⠀⠀⠀⠀⠀⠀⠀⠀⠉⠉⠁⠀⠀⠀\n");
    PrintW(L" ⠀⠇⣧⠘⡇⠦⣹⣸⣿⡇⡿⡿⣡⣼⣿⣿⣷⣦⣄⡀⠀⠀⣸⣿⣿⠄⠻⢷⣦⠀\n");
    PrintW(L" ⠀⢀⠘⣇⢹⡸⣿⣿⣿⢹⢃⣠⣿⣿⣿⣿⣿⣿⣿⣿⣆⠀⠑⠋⠉⠀⠀⠈⣿⣧\n");
    PrintW(L" ⠀⢸⣿⡌⠘⢷⣿⣿⡏⢀⣾⣿⣿⣿⣿⣿⣿⢻⣿⣿⣿⡆⠀⠀⠀⠀⠀⠀⣿⡿\n");
    PrintW(L" ⠀⠈⣿⣿⣦⡌⢿⠏⣰⣿⣿⣿⣿⣿⣿⡿⡏⣼⣿⣿⣿⡇⣄⠀⠀⠀⢀⣼⣿⠇\n");
    PrintW(L" ⠀⠀⠹⣿⣿⢻⡀⣼⣿⣿⢻⣿⣿⣿⣿⡇⡇⢻⣿⣿⣿⡇⣿⣿⣶⣿⣿⠟⠁⠀\n");
    PrintW(L" ⠀⠀⠀⢻⣿⣦⡓⢿⣿⣿⡆⣿⣿⣿⣿⢃⣶⡸⣿⣿⣿⡇⠀⠉⠉⠁⠀⠀⠀⠀\n");
    PrintW(L" ⠀⠀⠀⠈⣿⣿⣿⡆⠀⠀⠀⣿⣿⣿⡟⣼⡿⠁⢹⣿⣿⣷⠀⠀⠀⠀⠀⠀⠀⠀\n");
    PrintW(L" ⠀⠀⠀⠀⢸⣿⣿⡇⠀⠀⢠⣿⣿⣿⢠⣿⣇⠀⠀⢻⣿⣿⡄⠀⠀⠀⠀⠀⠀⠀\n");
    PrintW(L" ⠀⠀⠀⠀⠘⣿⣿⣧⠀⠀⢸⣿⣿⡿⠘⣿⣿⣆⠀⠀⣿⣿⣿⠀⠀⠀⠀⠀⠀⠀\n");
    PrintW(L" ⠀⠀⠀⠀⠀⣿⣿⣿⡄⠀⢸⣿⣿⡇⢀⣿⣿⡟⠀⠀⢸⣿⣿⡀⠀⠀⠀⠀⠀⠀\n");
    PrintW(L" ⠀⠀⠀⠀⡴⢿⣿⣿⡇⢠⡿⣿⣿⢷⢩⣿⡟⠁⠀⢀⢼⡿⣿⡇⠀⠀⠀⠀⠀⠀\n");
    PrintW(L" ⠀⠀⠀⠀⠁⢿⠸⠿⠇⠈⠃⠛⠃⠚⠈⠁⠀⠀⠀⠈⠸⠜⠛⠀⠀⠀⠀⠀⠀⠀\n");
    SetColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    puts("====================================");
    SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("=          Welcome to %s      =\n", APP_N);
    SetColor(FOREGROUND_BLUE | FOREGROUND_INTENSITY);
    puts("====================================");
    SetColor(FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
}

void HelpMain() {
    puts("Usage: LOC.exe [options]");
    puts("ex: ");
    puts("--help               Display this help message");
    puts("--version            Display version information");
    puts("--try-get-cmd        trying get command line whit SYSTEM privlege");
    puts("--brudeforce         bruteforce password of SYSTEM and all users in server from local passwords");
    puts("--kernal-exploit     try to exploit known kernel vulnerabilities to get SYSTEM privlege");
    puts("--log                log all actions and results to file 'loc_log.txt'");
}


void PrintRed(const char* msg) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, FOREGROUND_RED | FOREGROUND_INTENSITY);
    printf("%s\n", msg);
    SetConsoleTextAttribute(h, 7); // reset
}

void PrintGreen(const char* msg) {
    HANDLE h = GetStdHandle(STD_OUTPUT_HANDLE);
    SetConsoleTextAttribute(h, FOREGROUND_GREEN | FOREGROUND_INTENSITY);
    printf("%s\n", msg);
    SetConsoleTextAttribute(h, 7); // reset
}

// ======================= ENUMARATIONS AND STRUCTS =======================

// Function to write formatted data to the info file
void write_to_file(const char* format, ...) {
    FILE *file = fopen(INFO_FILE, "a");
    if (file) {
        va_list args;
        va_start(args, format);
        vfprintf(file, format, args);
        va_end(args);
        fclose(file);
    }
}

void wide_to_utf8(const wchar_t *src, char *dest, int destSize) {
    WideCharToMultiByte(CP_UTF8, 0, src, -1, dest, destSize, NULL, NULL);
}

// ======================= FUNCTIONS TO POPULATE SystemInfo STRUCT =======================

void get_user_hostname() {
    DWORD username_len = sizeof(sysInfo.username);
    DWORD hostname_len = sizeof(sysInfo.hostname);
    
    GetUserNameA(sysInfo.username, &username_len);
    GetComputerNameA(sysInfo.hostname, &hostname_len);
    
    // Also write to file
    write_to_file("=== User and Hostname Information ===\n");
    write_to_file("Username: %s\n", sysInfo.username);
    write_to_file("Hostname: %s\n\n", sysInfo.hostname);
}

void get_group_memberships() {
    write_to_file("=== Current User Group Memberships ===\n");
    
    HANDLE hToken;
    DWORD dwSize = 0;
    
    sysInfo.group_count = 0;
    
    if (OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        GetTokenInformation(hToken, TokenGroups, NULL, 0, &dwSize);
        PTOKEN_GROUPS pGroups = (PTOKEN_GROUPS)LocalAlloc(LPTR, dwSize);
        
        if (GetTokenInformation(hToken, TokenGroups, pGroups, dwSize, &dwSize)) {
            for (DWORD i = 0; i < pGroups->GroupCount && sysInfo.group_count < 128; i++) {
                char name[256];
                char domain[256];
                DWORD name_size = sizeof(name);
                DWORD domain_size = sizeof(domain);
                SID_NAME_USE sidType;
                
                if (LookupAccountSidA(NULL, pGroups->Groups[i].Sid, 
                                    name, &name_size, 
                                    domain, &domain_size, 
                                    &sidType)) {
                    snprintf(sysInfo.groups[sysInfo.group_count], 256, "%s\\%s", domain, name);
                    sysInfo.group_count++;
                    
                    write_to_file("Group: %s\\%s\n", domain, name);
                }
            }
        }
        LocalFree(pGroups);
        CloseHandle(hToken);
    }
    write_to_file("\n");
}

void get_users_and_groups() {
    write_to_file("=== Local Users ===\n");
    
    sysInfo.user_count = 0;
    sysInfo.local_group_count = 0;

    DWORD dwEntriesRead = 0, dwTotalEntries = 0;
    PUSER_INFO_1 pUserInfo = NULL;

    if (NetUserEnum(NULL, 1, FILTER_NORMAL_ACCOUNT,
        (LPBYTE*)&pUserInfo, MAX_PREFERRED_LENGTH,
        &dwEntriesRead, &dwTotalEntries, NULL) == NERR_Success)
    {
        for (DWORD i = 0; i < dwEntriesRead && sysInfo.user_count < 128; i++) {
            char utf8_name[256];
            wide_to_utf8((wchar_t*)pUserInfo[i].usri1_name, utf8_name, sizeof(utf8_name));
            strncpy(sysInfo.users[sysInfo.user_count], utf8_name, 256);
            sysInfo.user_count++;
            
            write_to_file("User: %s\n", utf8_name);
        }
        NetApiBufferFree(pUserInfo);
    }

    write_to_file("\n=== Local Groups ===\n");

    PLOCALGROUP_INFO_1 pGroupInfo = NULL;

    if (NetLocalGroupEnum(NULL, 1, (LPBYTE*)&pGroupInfo,
        MAX_PREFERRED_LENGTH, &dwEntriesRead, &dwTotalEntries, NULL) == NERR_Success)
    {
        for (DWORD i = 0; i < dwEntriesRead && sysInfo.local_group_count < 128; i++) {
            char utf8_group[256];
            wide_to_utf8((wchar_t*)pGroupInfo[i].lgrpi1_name, utf8_group, sizeof(utf8_group));
            strncpy(sysInfo.local_groups[sysInfo.local_group_count], utf8_group, 256);
            sysInfo.local_group_count++;
            
            write_to_file("Group: %s\n", utf8_group);
        }
        NetApiBufferFree(pGroupInfo);
    }

    write_to_file("\n");
}

void get_os_info() {
    write_to_file("=== Operating System Information ===\n");
    
    OSVERSIONINFOEX osvi;
    SYSTEM_INFO si;
    
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOEX));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
    GetVersionEx((OSVERSIONINFO*)&osvi);
    
    GetNativeSystemInfo(&si);
    
    snprintf(sysInfo.os_version, 128, "%d.%d.%d", 
             osvi.dwMajorVersion, 
             osvi.dwMinorVersion, 
             osvi.dwBuildNumber);
    
    switch (si.wProcessorArchitecture) {
        case PROCESSOR_ARCHITECTURE_AMD64:
            strcpy(sysInfo.os_arch, "x64");
            break;
        case PROCESSOR_ARCHITECTURE_INTEL:
            strcpy(sysInfo.os_arch, "x86");
            break;
        case PROCESSOR_ARCHITECTURE_ARM:
            strcpy(sysInfo.os_arch, "ARM");
            break;
        case PROCESSOR_ARCHITECTURE_ARM64:
            strcpy(sysInfo.os_arch, "ARM64");
            break;
        default:
            strcpy(sysInfo.os_arch, "Unknown");
    }
    
    write_to_file("OS Version: %s\n", sysInfo.os_version);
    write_to_file("OS Architecture: %s\n", sysInfo.os_arch);
    write_to_file("\n");
}

void get_network_info() {
    write_to_file("=== Network Information ===\n");
    
    PIP_ADAPTER_INFO pAdapterInfo = NULL;
    ULONG ulOutBufLen = 0;
    
    sysInfo.adapter_count = 0;
    
    GetAdaptersInfo(pAdapterInfo, &ulOutBufLen);
    pAdapterInfo = (IP_ADAPTER_INFO*)malloc(ulOutBufLen);
    
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == NO_ERROR) {
        PIP_ADAPTER_INFO pAdapter = pAdapterInfo;
        while (pAdapter && sysInfo.adapter_count < 32) {
            char adapter_info[512];
            char mac[20] = "";
            
            // بناء عنوان MAC
            for (UINT i = 0; i < pAdapter->AddressLength; i++) {
                char temp[4];
                if (i == pAdapter->AddressLength - 1)
                    sprintf(temp, "%.2X", pAdapter->Address[i]);
                else
                    sprintf(temp, "%.2X-", pAdapter->Address[i]);
                strcat(mac, temp);
            }
            
            snprintf(adapter_info, 512, "Adapter: %s | IP: %s | MAC: %s",
                    pAdapter->Description,
                    pAdapter->IpAddressList.IpAddress.String,
                    mac);
            
            strncpy(sysInfo.adapters[sysInfo.adapter_count], adapter_info, 256);
            sysInfo.adapter_count++;
            
            write_to_file("%s\n", adapter_info);
            pAdapter = pAdapter->Next;
        }
    }
    free(pAdapterInfo);
    write_to_file("\n");
}

void get_installed_apps() {
    write_to_file("=== Installed Applications ===\n");
    
    sysInfo.installed_apps_count = 0;
    
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_LOCAL_MACHINE, 
                     "SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall", 
                     0, KEY_READ, &hKey) == ERROR_SUCCESS) {
        DWORD subkeyIndex = 0;
        char subkeyName[256];
        DWORD subkeyNameSize = sizeof(subkeyName);
        
        while (RegEnumKeyExA(hKey, subkeyIndex, subkeyName, 
                           &subkeyNameSize, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
            HKEY hSubKey;
            if (RegOpenKeyExA(hKey, subkeyName, 0, KEY_READ, &hSubKey) == ERROR_SUCCESS) {
                char displayName[256];
                DWORD dataSize = sizeof(displayName);
                
                if (RegQueryValueExA(hSubKey, "DisplayName", NULL, NULL, 
                                   (LPBYTE)displayName, &dataSize) == ERROR_SUCCESS &&
                    sysInfo.installed_apps_count < 512) {
                    
                    strncpy(sysInfo.installed_apps[sysInfo.installed_apps_count], 
                           displayName, 256);
                    sysInfo.installed_apps_count++;
                    
                    write_to_file("%s\n", displayName);
                }
                RegCloseKey(hSubKey);
            }
            subkeyNameSize = sizeof(subkeyName);
            subkeyIndex++;
        }
        RegCloseKey(hKey);
    }
    write_to_file("\n");
}

void get_running_processes() {
    write_to_file("=== Running Processes ===\n");
    
    sysInfo.process_count = 0;
    
    HANDLE hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap != INVALID_HANDLE_VALUE) {
        PROCESSENTRY32 pe32;
        pe32.dwSize = sizeof(PROCESSENTRY32);
        
        if (Process32First(hProcessSnap, &pe32)) {
            do {
                if (sysInfo.process_count < 256) {
                    char process_info[256];
                    snprintf(process_info, 256, "PID: %d - %s", 
                            pe32.th32ProcessID, 
                            pe32.szExeFile);
                    
                    strncpy(sysInfo.processes[sysInfo.process_count], process_info, 256);
                    sysInfo.process_count++;
                    
                    write_to_file("%s\n", process_info);
                }
            } while (Process32Next(hProcessSnap, &pe32) && sysInfo.process_count < 256);
        }
        CloseHandle(hProcessSnap);
    }
    write_to_file("\n");
}

// ======================= FUNCTION TO PRINT FROM STRUCT =======================

void print_system_info_from_struct() {
    printf("\n=========== SYSTEM INFO FROM STRUCT ===========\n");
    
    printf("\n[+] User and Hostname:\n");
    printf("    Username: %s\n", sysInfo.username);
    printf("    Hostname: %s\n", sysInfo.hostname);
    
    printf("\n[+] Current User Groups (%d):\n", sysInfo.group_count);
    for (int i = 0; i < sysInfo.group_count; i++) {
        printf("    - %s\n", sysInfo.groups[i]);
    }
    
    printf("\n[+] Local Users (%d):\n", sysInfo.user_count);
    for (int i = 0; i < sysInfo.user_count; i++) {
        printf("    - %s\n", sysInfo.users[i]);
    }
    
    printf("\n[+] Local Groups (%d):\n", sysInfo.local_group_count);
    for (int i = 0; i < sysInfo.local_group_count; i++) {
        printf("    - %s\n", sysInfo.local_groups[i]);
    }
    
    printf("\n[+] Operating System:\n");
    printf("    Version: %s\n", sysInfo.os_version);
    printf("    Architecture: %s\n", sysInfo.os_arch);
    
    printf("\n[+] Network Adapters (%d):\n", sysInfo.adapter_count);
    for (int i = 0; i < sysInfo.adapter_count; i++) {
        printf("    %s\n", sysInfo.adapters[i]);
    }
    
    printf("\n[+] Installed Applications (showing first 10 of %d):\n", sysInfo.installed_apps_count);
    int max_apps_to_show = (sysInfo.installed_apps_count > 10) ? 10 : sysInfo.installed_apps_count;
    for (int i = 0; i < max_apps_to_show; i++) {
        printf("    - %s\n", sysInfo.installed_apps[i]);
    }
    
    printf("\n[+] Running Processes (showing first 15 of %d):\n", sysInfo.process_count);
    int max_procs_to_show = (sysInfo.process_count > 15) ? 15 : sysInfo.process_count;
    for (int i = 0; i < max_procs_to_show; i++) {
        printf("    - %s\n", sysInfo.processes[i]);
    }
    
    printf("\n=========== END OF SYSTEM INFO ===========\n");
}

// ======================= FUNCTION TO SAVE STRUCT TO FILE =======================

void save_struct_info_to_file() {
    FILE *file = fopen("struct_info.txt", "w");
    if (!file) return;
    
    fprintf(file, "=========== SYSTEM INFO STRUCT DUMP ===========\n\n");
    
    fprintf(file, "User and Hostname:\n");
    fprintf(file, "  Username: %s\n", sysInfo.username);
    fprintf(file, "  Hostname: %s\n\n", sysInfo.hostname);
    
    fprintf(file, "Current User Groups (%d):\n", sysInfo.group_count);
    for (int i = 0; i < sysInfo.group_count; i++) {
        fprintf(file, "  - %s\n", sysInfo.groups[i]);
    }
    fprintf(file, "\n");
    
    fprintf(file, "Local Users (%d):\n", sysInfo.user_count);
    for (int i = 0; i < sysInfo.user_count; i++) {
        fprintf(file, "  - %s\n", sysInfo.users[i]);
    }
    fprintf(file, "\n");
    
    fprintf(file, "Local Groups (%d):\n", sysInfo.local_group_count);
    for (int i = 0; i < sysInfo.local_group_count; i++) {
        fprintf(file, "  - %s\n", sysInfo.local_groups[i]);
    }
    fprintf(file, "\n");
    
    fprintf(file, "Operating System:\n");
    fprintf(file, "  Version: %s\n", sysInfo.os_version);
    fprintf(file, "  Architecture: %s\n\n", sysInfo.os_arch);
    
    fprintf(file, "Network Adapters (%d):\n", sysInfo.adapter_count);
    for (int i = 0; i < sysInfo.adapter_count; i++) {
        fprintf(file, "  %s\n", sysInfo.adapters[i]);
    }
    fprintf(file, "\n");
    
    fprintf(file, "Installed Applications (%d total):\n", sysInfo.installed_apps_count);
    for (int i = 0; i < sysInfo.installed_apps_count; i++) {
        fprintf(file, "  - %s\n", sysInfo.installed_apps[i]);
    }
    fprintf(file, "\n");
    
    fprintf(file, "Running Processes (%d total):\n", sysInfo.process_count);
    for (int i = 0; i < sysInfo.process_count; i++) {
        fprintf(file, "  - %s\n", sysInfo.processes[i]);
    }
    
    fprintf(file, "\n=========== END OF STRUCT DUMP ===========\n");
    fclose(file);
    
    printf("Struct info saved to struct_info.txt\n");
}

// end enum 
// ======================= SeImpersonatePrivilege Attacker Check =======================


int HasSeImpersonatePrivilege() {
    HANDLE hToken = NULL;
    DWORD size = 0;
    PTOKEN_PRIVILEGES privileges = NULL;
    int result = 0; // 0 = false, 1 = true

    // Open current process token
    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken)) {
        return 0;
    }

    // First call to get required buffer size
    GetTokenInformation(hToken, TokenPrivileges, NULL, 0, &size);
    privileges = (PTOKEN_PRIVILEGES)malloc(size);

    if (!privileges) {
        CloseHandle(hToken);
        return 0;
    }

    // Get token privileges
    if (!GetTokenInformation(hToken, TokenPrivileges, privileges, size, &size)) {
        free(privileges);
        CloseHandle(hToken);
        return 0;
    }

    // Prepare privilege name
    LUID luid;
    if (!LookupPrivilegeValue(NULL, SE_IMPERSONATE_NAME, &luid)) {
        free(privileges);
        CloseHandle(hToken);
        return 0;
    }

    // Loop through privileges
    for (DWORD i = 0; i < privileges->PrivilegeCount; i++) {
        if (privileges->Privileges[i].Luid.LowPart == luid.LowPart &&
            privileges->Privileges[i].Luid.HighPart == luid.HighPart)
        {
            // Check if enabled
            if (privileges->Privileges[i].Attributes & SE_PRIVILEGE_ENABLED) {
                result = 1; // true
            }
            break;
        }
    }

    free(privileges);
    CloseHandle(hToken);
    return result;
}

void CheckSeImpersonatePrivilege() {
    printf("\n=== SeImpersonatePrivilege Check ===\n");
    if (HasSeImpersonatePrivilege()) {
        PrintGreen("[+] SeImpersonatePrivilege is ENABLED on this account.\n");
    } else {
        PrintRed("[-] SeImpersonatePrivilege is NOT enabled on this account.\n");
    }
}

// ======================= Unquoted Service Paths =======================

int GetUnquotedServicePaths(char paths[][MAX_PATH], int maxCount) {
    SC_HANDLE hSCM = OpenSCManager(NULL, NULL, SC_MANAGER_ENUMERATE_SERVICE);
    if (!hSCM) return 0;

    DWORD bytesNeeded = 0, servicesCount = 0;
    EnumServicesStatusEx(
        hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32,
        SERVICE_STATE_ALL, NULL, 0, &bytesNeeded,
        &servicesCount, NULL, NULL
    );

    BYTE* buffer = (BYTE*)malloc(bytesNeeded);
    if (!buffer) {
        CloseServiceHandle(hSCM);
        return 0;
    }

    if (!EnumServicesStatusEx(
        hSCM, SC_ENUM_PROCESS_INFO, SERVICE_WIN32,
        SERVICE_STATE_ALL, buffer, bytesNeeded,
        &bytesNeeded, &servicesCount, NULL, NULL
    )) {
        free(buffer);
        CloseServiceHandle(hSCM);
        return 0;
    }

    ENUM_SERVICE_STATUS_PROCESS* svc = (ENUM_SERVICE_STATUS_PROCESS*)buffer;
    int count = 0;

    for (DWORD i = 0; i < servicesCount && count < maxCount; i++) {
        SC_HANDLE hService = OpenService(hSCM, svc[i].lpServiceName, SERVICE_QUERY_CONFIG);
        if (!hService) continue;

        DWORD needed = 0;
        QueryServiceConfig(hService, NULL, 0, &needed);

        QUERY_SERVICE_CONFIG* cfg = (QUERY_SERVICE_CONFIG*)malloc(needed);
        if (!cfg) {
            CloseServiceHandle(hService);
            continue;
        }

        if (QueryServiceConfig(hService, cfg, needed, &needed)) {
            char* bin = cfg->lpBinaryPathName;
            // Check if UNQUOTED & path contains spaces
            if (bin &&
                bin[0] != '"' &&
                strstr(bin, " ") != NULL)
            {
                strncpy(paths[count], bin, MAX_PATH);
                count++;
            }
        }

        free(cfg);
        CloseServiceHandle(hService);
    }

    free(buffer);
    CloseServiceHandle(hSCM);
    return count; // number of paths found
}

int CanWriteToPath(const char* dirPath) {
    DWORD rights = 0;
    HANDLE hToken = NULL;
    PSID userSid = NULL;
    PACL dacl = NULL;

    if (!OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hToken))
        return 0;

    TOKEN_USER* tokenUser = (TOKEN_USER*)malloc(512);
    DWORD size = 512;

    if (!GetTokenInformation(hToken, TokenUser, tokenUser, size, &size)) {
        free(tokenUser);
        CloseHandle(hToken);
        return 0;
    }

    userSid = tokenUser->User.Sid;

    PSECURITY_DESCRIPTOR sd = NULL;
    if (GetNamedSecurityInfoA((LPSTR)dirPath,
        SE_FILE_OBJECT,
        DACL_SECURITY_INFORMATION,
        NULL, NULL,
        &dacl, NULL,
        &sd) != ERROR_SUCCESS)
    {
        free(tokenUser);
        CloseHandle(hToken);
        return 0;
    }

    BOOL access = FALSE;
    DWORD accessRights = GENERIC_WRITE | FILE_WRITE_DATA | FILE_ADD_FILE;

    if (AccessCheck(sd, hToken, accessRights, NULL, NULL, &rights, &rights, &access)) {
        free(sd);
        free(tokenUser);
        CloseHandle(hToken);
        return access ? 1 : 0;
    }

    free(sd);
    free(tokenUser);
    CloseHandle(hToken);
    return 0;
}

void CheckUnquotedExploits() {
    char paths[MAX_SERVICES][MAX_PATH];
    int found = GetUnquotedServicePaths(paths, MAX_SERVICES);
    printf("=== Unquoted Service Path Analysis ===\n");
    int exploitable = 0;

    if (found == 0) {
        printf("[-] No unquoted service paths found at all.\n");
        return;
    }

    for (int i = 0; i < found; i++) {
        char dir[MAX_PATH] = {0};
        strncpy(dir, paths[i], MAX_PATH);

        // Extract directory only
        for (int j = strlen(dir); j >= 0; j--) {
            if (dir[j] == '\\' || dir[j] == '/') {
                dir[j] = '\0';
                break;
            }
        }

        if (CanWriteToPath(dir)) {
            PrintGreen("[!] Exploitable: ");
            printf("%s\n", paths[i]);  // Print FULL path
            exploitable++;
        } else {
            PrintRed("[-] Not Exploitable: ");
            printf("%s\n", paths[i]);  // Print FULL path (FIXED)
        }
    }

    printf("\n---------------------------------\n");

    if (exploitable == 0)
        PrintRed("[-] No exploitable unquoted paths found.\n");
    else
        PrintGreen("[+] Total exploitable unquoted service paths: "), printf("%d\n", exploitable);

    fflush(stdout);
}


int main() {
    remove(INFO_FILE);
    
    PrintPanel();
    Sleep(3000);
    printf("Collecting system information...\n\n");
    
    get_user_hostname();
    get_group_memberships();
    get_users_and_groups();
    get_os_info();
    get_network_info();
    get_installed_apps();
    get_running_processes();
    
    printf("\nInformation saved to %s\n", INFO_FILE);
    
    print_system_info_from_struct();
    
    save_struct_info_to_file();
    
    // SeImpersonatePrivilege
    CheckSeImpersonatePrivilege();

    // Unquoted Service Paths
    CheckUnquotedExploits();
    
    printf("\nPress any key to exit...\n");
    getchar();
    
    return 0;
}

// run app : x86_64-w64-mingw32-gcc main.c -o system_info.exe -l netapi32 -l iphlpapi -l advapi32 -l psapi -l advapi32 -l user32