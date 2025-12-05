#include "src/UserAndHost.h"
#include "../color/colorPrint.h"
#include "src/userGrops.h"
#include "src/osInfo.h"
#include "src/networkInfo.h"
#include "src/installApps.h"
#include "src/prosesRun.h"
#include <windows.h>
#include <stdio.h>
#include <lm.h>

void genetorMain() {
    enableANSI();
    


    printYellow(L"\n=== User and Host Information ===\n");
    getUser();
    getHost();
    getAllUsers();
    printYellow(L"\n==============================================\n");
    printYellow(L"\n=== Group memberships of the current user  ===\n");
    getUserGroups();
    getUsersAndGroups();
    printYellow(L"\n==============================================\n");
    // New OS information functions
    printYellow(L"\n========= Windows System Information =========\n");
    printBasicOSInfo();
    printArchitecture();
    printBuildInfo();
    printDisplayInfo();
    printYellow(L"\n==============================================\n");
    // New Network information functions
    printYellow(L"\n========= Windows Network Information ========\n");
    getAllNetworkInfo();
    printYellow(L"\n==============================================\n");
    // Installed applications
    printYellow(L"\n======== Installed Applications ==============\n");
    getInstalledApps();
    printYellow(L"\n==============================================\n");
    // Running processes
    printYellow(L"\n======== Currently Running Processes =========\n");
    getDetailedRunningProcesses();
    printYellow(L"\n==============================================\n");

}


// x86_64-w64-mingw32-gcc enum.c src/*.c -o cs.exe  -liphlpapi -lws2_32 -lole32 -loleaut32 -lnetapi32 -lversion -ladvapi32 -DINITGUID