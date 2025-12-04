// Fix: winsock2.h must be included BEFORE windows.h
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
#include <windows.h>
#include <iphlpapi.h>
#include <netfw.h>
#include <stdlib.h>
#include "../color/colorPrint.h"
#include "networkInfo.h"

#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

void getNetworkAdapters() {
    wchar_t buffer[1024];
    PIP_ADAPTER_ADDRESSES pAddresses = NULL;
    ULONG outBufLen = 0;
    ULONG flags = GAA_FLAG_INCLUDE_PREFIX;
    DWORD result;

    printInfo(L"NETWORK ADAPTERS");

    result = GetAdaptersAddresses(AF_UNSPEC, flags, NULL, NULL, &outBufLen);
    if (result == ERROR_BUFFER_OVERFLOW) {
        pAddresses = (PIP_ADAPTER_ADDRESSES)malloc(outBufLen);
        if (!pAddresses) {
            printError(L"[-] Memory allocation failed for adapter addresses\n");
            return;
        }

        result = GetAdaptersAddresses(AF_UNSPEC, flags, NULL, pAddresses, &outBufLen);
        if (result == NO_ERROR) {
            PIP_ADAPTER_ADDRESSES pCurr = pAddresses;
            while (pCurr) {
                // Adapter Name
                swprintf(buffer, 1024, L"[+] Adapter: %ls\n", pCurr->FriendlyName);
                printSuccess(buffer);

                // Adapter Type
                wcscpy(buffer, L"    Type: ");
                switch (pCurr->IfType) {
                    case IF_TYPE_ETHERNET_CSMACD: wcscat(buffer, L"Ethernet"); break;
                    case IF_TYPE_IEEE80211: wcscat(buffer, L"WiFi"); break;
                    case IF_TYPE_SOFTWARE_LOOPBACK: wcscat(buffer, L"Loopback"); break;
                    default:
                        swprintf(buffer + wcslen(buffer), 1024 - wcslen(buffer), 
                                 L"Other (%lu)", pCurr->IfType);
                        break;
                }
                wcscat(buffer, L"\n");
                printGreen(buffer);

                // MAC Address
                if (pCurr->PhysicalAddressLength > 0) {
                    wcscpy(buffer, L"    MAC Address: ");
                    for (ULONG i = 0; i < pCurr->PhysicalAddressLength; i++) {
                        if (i == pCurr->PhysicalAddressLength - 1) {
                            swprintf(buffer + wcslen(buffer), 1024 - wcslen(buffer),
                                     L"%.2X", pCurr->PhysicalAddress[i]);
                        } else {
                            swprintf(buffer + wcslen(buffer), 1024 - wcslen(buffer),
                                     L"%.2X-", pCurr->PhysicalAddress[i]);
                        }
                    }
                    wcscat(buffer, L"\n");
                    printGreen(buffer);
                }

                // IP Addresses
                PIP_ADAPTER_UNICAST_ADDRESS pUnicast = pCurr->FirstUnicastAddress;
                while (pUnicast) {
                    wchar_t ipStr[INET6_ADDRSTRLEN] = {0};
                    if (pUnicast->Address.lpSockaddr->sa_family == AF_INET) {
                        struct sockaddr_in* sa_in = (struct sockaddr_in*)pUnicast->Address.lpSockaddr;
                        InetNtopW(AF_INET, &(sa_in->sin_addr), ipStr, INET6_ADDRSTRLEN);
                        swprintf(buffer, 1024, L"    IPv4: %ls\n", ipStr);
                        printGreen(buffer);
                    } else if (pUnicast->Address.lpSockaddr->sa_family == AF_INET6) {
                        struct sockaddr_in6* sa_in6 = (struct sockaddr_in6*)pUnicast->Address.lpSockaddr;
                        InetNtopW(AF_INET6, &(sa_in6->sin6_addr), ipStr, INET6_ADDRSTRLEN);
                        swprintf(buffer, 1024, L"    IPv6: %ls\n", ipStr);
                        printGreen(buffer);
                    }
                    pUnicast = pUnicast->Next;
                }

                // DHCP
                swprintf(buffer, 1024, L"    DHCP Enabled: %s\n", 
                        (pCurr->Flags & IP_ADAPTER_DHCP_ENABLED) ? L"Yes" : L"No");
                printGreen(buffer);

                pCurr = pCurr->Next;
            }
        } else {
            swprintf(buffer, 1024, L"[-] GetAdaptersAddresses failed: %lu\n", result);
            printError(buffer);
        }
        free(pAddresses);
    } else {
        swprintf(buffer, 1024, L"[-] Failed to get adapter addresses buffer size: %lu\n", result);
        printError(buffer);
    }
}

void getIPConfig() {
    wchar_t buffer[512];
    FIXED_INFO *pFixedInfo = NULL;
    ULONG ulOutBufLen = 0;
    DWORD dwRetVal;

    printInfo(L" IP CONFIGURATION");

    dwRetVal = GetNetworkParams(NULL, &ulOutBufLen);
    if (dwRetVal == ERROR_BUFFER_OVERFLOW) {
        pFixedInfo = (FIXED_INFO*)malloc(ulOutBufLen);
        if (!pFixedInfo) {
            printError(L"[-] Memory allocation failed for network params\n");
            return;
        }

        dwRetVal = GetNetworkParams(pFixedInfo, &ulOutBufLen);
        if (dwRetVal == NO_ERROR) {
            swprintf(buffer, 512, L"    Host Name: %hs\n", pFixedInfo->HostName);
            printGreen(buffer);
            swprintf(buffer, 512, L"    Domain Name: %hs\n", pFixedInfo->DomainName);
            printGreen(buffer);

            printGreen(L"    DNS Servers:\n");
            IP_ADDR_STRING* pDNS = &pFixedInfo->DnsServerList;
            int i = 1;
            while (pDNS) {
                swprintf(buffer, 512, L"        %d. %hs\n", i++, pDNS->IpAddress.String);
                printGreen(buffer);
                pDNS = pDNS->Next;
            }

            wcscpy(buffer, L"    Node Type: ");
            switch (pFixedInfo->NodeType) {
                case BROADCAST_NODETYPE: wcscat(buffer, L"Broadcast"); break;
                case PEER_TO_PEER_NODETYPE: wcscat(buffer, L"Peer to Peer"); break;
                case MIXED_NODETYPE: wcscat(buffer, L"Mixed"); break;
                case HYBRID_NODETYPE: wcscat(buffer, L"Hybrid"); break;
                default: wcscat(buffer, L"Unknown"); break;
            }
            wcscat(buffer, L"\n");
            printGreen(buffer);

        } else {
            swprintf(buffer, 512, L"[-] GetNetworkParams failed: %lu\n", dwRetVal);
            printError(buffer);
        }
        free(pFixedInfo);
    } else {
        swprintf(buffer, 512, L"[-] Failed to get network parameters: %lu\n", dwRetVal);
        printError(buffer);
    }
}

void getOpenPorts() {
    wchar_t buffer[512];
    PMIB_TCPTABLE_OWNER_PID pTcpTable = NULL;
    PMIB_UDPTABLE_OWNER_PID pUdpTable = NULL;
    DWORD dwSize = 0;
    DWORD dwRetVal = 0;
    
    printInfo(L" OPEN PORTS");
    
    // Get TCP ports
    dwRetVal = GetExtendedTcpTable(NULL, &dwSize, FALSE, AF_INET, 
                                   TCP_TABLE_OWNER_PID_ALL, 0);
    if (dwRetVal == ERROR_INSUFFICIENT_BUFFER) {
        pTcpTable = (PMIB_TCPTABLE_OWNER_PID)malloc(dwSize);
        if (pTcpTable != NULL) {
            dwRetVal = GetExtendedTcpTable(pTcpTable, &dwSize, FALSE, AF_INET, 
                                           TCP_TABLE_OWNER_PID_ALL, 0);
            if (dwRetVal == NO_ERROR) {
                for (DWORD i = 0; i < pTcpTable->dwNumEntries; i++) {
                    // Only show listening ports (state 2 = LISTEN)
                    if (pTcpTable->table[i].dwState == MIB_TCP_STATE_LISTEN) {
                        u_short port = ntohs((u_short)pTcpTable->table[i].dwLocalPort);
                        
                        // Determine service name based on port
                        wchar_t* service = L"unknown";
                        switch (port) {
                            case 20: service = L"ftp-data"; break;
                            case 21: service = L"ftp"; break;
                            case 22: service = L"ssh"; break;
                            case 23: service = L"telnet"; break;
                            case 25: service = L"smtp"; break;
                            case 53: service = L"dns"; break;
                            case 80: service = L"http"; break;
                            case 88: service = L"kerberos"; break;
                            case 110: service = L"pop3"; break;
                            case 135: service = L"msrpc"; break;
                            case 139: service = L"netbios-ssn"; break;
                            case 143: service = L"imap"; break;
                            case 389: service = L"ldap"; break;
                            case 443: service = L"https"; break;
                            case 445: service = L"microsoft-ds"; break;
                            case 465: service = L"smtps"; break;
                            case 587: service = L"smtp-submission"; break;
                            case 993: service = L"imaps"; break;
                            case 995: service = L"pop3s"; break;
                            case 1433: service = L"ms-sql-s"; break;
                            case 1521: service = L"oracle"; break;
                            case 1723: service = L"pptp"; break;
                            case 3306: service = L"mysql"; break;
                            case 3389: service = L"ms-wbt-server"; break;
                            case 5432: service = L"postgresql"; break;
                            case 5900: service = L"vnc"; break;
                            case 5985: service = L"wsman"; break;
                            case 5986: service = L"wsmans"; break;
                            case 8080: service = L"http-proxy"; break;
                            case 8443: service = L"https-alt"; break;
                            case 9000: service = L"cslistener"; break;
                            case 1337: service = L"httpx"; break;
                            // Add more ports as needed
                        }
                        
                        // Format: -> PORT : SERVICE [PROTOCOL]
                        swprintf(buffer, 512, L"    -> %d : %ls [TCP]\n", port, service);
                        printGreen(buffer);
                    }
                }
            }
            free(pTcpTable);
        }
    }
    
    // Get UDP ports
    dwSize = 0;
    dwRetVal = GetExtendedUdpTable(NULL, &dwSize, FALSE, AF_INET, 
                                   UDP_TABLE_OWNER_PID, 0);
    if (dwRetVal == ERROR_INSUFFICIENT_BUFFER) {
        pUdpTable = (PMIB_UDPTABLE_OWNER_PID)malloc(dwSize);
        if (pUdpTable != NULL) {
            dwRetVal = GetExtendedUdpTable(pUdpTable, &dwSize, FALSE, AF_INET, 
                                           UDP_TABLE_OWNER_PID, 0);
            if (dwRetVal == NO_ERROR) {
                for (DWORD i = 0; i < pUdpTable->dwNumEntries; i++) {
                    u_short port = ntohs((u_short)pUdpTable->table[i].dwLocalPort);
                    
                    // Determine service name for UDP
                    wchar_t* service = L"unknown";
                    switch (port) {
                        case 53: service = L"dns"; break;
                        case 67: service = L"dhcps"; break;
                        case 68: service = L"dhcpc"; break;
                        case 69: service = L"tftp"; break;
                        case 123: service = L"ntp"; break;
                        case 137: service = L"netbios-ns"; break;
                        case 138: service = L"netbios-dgm"; break;
                        case 161: service = L"snmp"; break;
                        case 162: service = L"snmptrap"; break;
                        case 500: service = L"isakmp"; break;
                        case 514: service = L"syslog"; break;
                        case 520: service = L"rip"; break;
                        case 1900: service = L"upnp"; break;
                        case 5353: service = L"mdns"; break;
                        case 8080: service = L"http"; break;
                        // Add more UDP ports as needed
                    }
                    
                    swprintf(buffer, 512, L"    -> %d : %ls [UDP]\n", port, service);
                    printGreen(buffer);
                }
            }
            free(pUdpTable);
        }
    }
    
    // If no open ports found
    if ((pTcpTable == NULL || pTcpTable->dwNumEntries == 0) && 
        (pUdpTable == NULL || pUdpTable->dwNumEntries == 0)) {
        printInfo(L"    No open ports found (only showing listening ports)\n");
    }
}

void getFirewallStatus() {
    wchar_t buffer[256];
    HRESULT hr = CoInitialize(NULL);
    if (FAILED(hr)) {
        printError(L"[-] COM Initialization failed for Firewall check\n");
        return;
    }

    INetFwMgr* fwMgr = NULL;
    // Note: For Mingw, we need to use the GUIDs directly
    static const IID IID_INetFwMgr = {0x174A0DDA, 0xB9FB, 0x11D2, {0x81, 0x61, 0x00, 0xC0, 0x4F, 0xB6, 0x61, 0x8A}};
    static const CLSID CLSID_NetFwMgr = {0x304CE942, 0x6E39, 0x40D8, {0x94, 0x3A, 0xB9, 0x8C, 0x84, 0xD3, 0x71, 0xD8}};
    
    hr = CoCreateInstance(&CLSID_NetFwMgr, NULL, CLSCTX_INPROC_SERVER, &IID_INetFwMgr, (void**)&fwMgr);
    if (SUCCEEDED(hr) && fwMgr) {
        INetFwPolicy* fwPolicy = NULL;
        hr = fwMgr->lpVtbl->get_LocalPolicy(fwMgr, &fwPolicy);
        if (SUCCEEDED(hr) && fwPolicy) {
            INetFwProfile* fwProfile = NULL;
            hr = fwPolicy->lpVtbl->get_CurrentProfile(fwPolicy, &fwProfile);
            if (SUCCEEDED(hr) && fwProfile) {
                VARIANT_BOOL fwEnabled;
                hr = fwProfile->lpVtbl->get_FirewallEnabled(fwProfile, &fwEnabled);
                if (SUCCEEDED(hr)) {
                    swprintf(buffer, 256, L"\n[+] Windows Firewall Status: %s\n", 
                            fwEnabled == VARIANT_TRUE ? L"ENABLED" : L"DISABLED");
                    printSuccess(buffer);
                } else {
                    printError(L"[-] Failed to get firewall status\n");
                }
                fwProfile->lpVtbl->Release(fwProfile);
            } else {
                printError(L"[-] Failed to get current profile\n");
            }
            fwPolicy->lpVtbl->Release(fwPolicy);
        } else {
            printError(L"[-] Failed to get local policy\n");
        }
        fwMgr->lpVtbl->Release(fwMgr);
    } else {
        printError(L"[-] Failed to create Firewall Manager instance\n");
    }
    CoUninitialize();
}

void getAllNetworkInfo() {
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        printError(L"[-] WSAStartup failed\n");
        return;
    }

    getNetworkAdapters();
    getIPConfig();
    getOpenPorts();
    getFirewallStatus();

    WSACleanup();
}