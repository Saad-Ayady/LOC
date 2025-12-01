# Windows Privilege Escalation Scanner 

## Advanced Red Team & Pentest Enumeration Tool (C) 
- A lightweight, fast, and fully‑native Windows privilege‑escalation scanner written in C, designed to mimic real APT‑style tradecraft (like APT28, FIN7, etc.).
- The tool performs deep enumeration to uncover misconfigurations, weak permissions, token privileges, and exploitable system settings used during real‑world intrusions.

## 🚀 Features :

### 🔍 Privilege Enumeration :
- Detects SeImpersonatePrivilege, SeDebugPrivilege, and other high‑value token privileges.
- Returns clean results (1/0) for automation in chained exploits.

### 📂 File & Directory Misconfigurations
- Finds Unquoted Service Paths and checks if they are actually exploitable (write access).
- Scans for writable directories in system‑level paths.

### 🛠️ Service Misconfigurations :
- Enumerates all Windows services
- Detects weak ACLs & permission abuse
- Checks binary paths, start type, and attack surface

### 🪟 System & Network Enumeration: 
- OS version, build, and architecture
- Logged‑in users
- Network interfaces & ARP table
- Domain/workgroup details

### 👣 Red Team Oriented :
- Behaves similarly to tools used by APT groups and advanced operators 
- No dependencies—static and stealthy
- Fast, minimal, ideal for initial foothold enumeration

## 📦 Build Instructions 
### 🔧 Linux (MinGW) 
```bash 
x86_64-w64-mingw32-gcc main.c -o system_info.exe -lnetapi32 -liphlpapi -ladvapi32 -lpsapi -luser32
```

### 🔧 Windows (MinGW / Visual Studio)
```powershell 
gcc main.c -o system_info.exe -l netapi32 -l iphlpapi -l advapi32 -l psapi -l user32
```

## 📄 Output Example:

```mathematica 
=== Unquoted Service Path Analysis ===
[!] Exploitable: C:\Program Files\Example Service\service.exe
[-] Not Exploitable: C:\Service\safe.exe
[+] Total exploitable unquoted service paths: 1
```

```pgsql
=== Token Privileges ===
[!] SeImpersonatePrivilege → ENABLED (OK to exploit)
[ ] SeDebugPrivilege → DISABLED

```
## ⚠️ Disclaimer :
- This tool is for educational, red teaming, and authorized penetration testing only.
- The author is not responsible for any misuse. 

## Roadmap 🧩: 
- UAC bypass detection
- Registry misconfiguration scanning
- DLL hijacking & search‑order weaknesses

## ❤️ Contribute : 
Pull requests welcome—optimizations, new modules, exploit detection logic, or documentation improvements.

