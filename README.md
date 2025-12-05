<p align="center">
        <img src="./logo.png" width=200 height=200 style="border-radius: 10px;">

</p>

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
bash install.sh

make
```

### 🔧 Windows (MinGW / Visual Studio)
```powershell 
make
```

## 📄 Output Example:

```mathematica 
=== Unquoted Service Path Analysis ===
======== Unquoted Service Path =========
[INFO] rsDNSClientSvc
              service name : rsDNSClientSvc
              writable path : Yes
              service path : C:\Program Files\ReasonLabs\DNS\rsDNSClientSvc.exe

======== Privilege Escalation CVE Scan =========

[VULNERABLE] System is affected by CVE-2024-21338
Exploit : https://www.exploit-db.com/exploits/52275
------------------------------------------
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

