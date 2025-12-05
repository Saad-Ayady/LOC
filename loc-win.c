#include <stdio.h>
#include "args.h"
#include "src/models/enum/enum.h"
#include "src/models/color/colorPrint.h"
#include "src/models/SeImperSona/seImperSona.h"
#include "src/models/usp/getUspPaht.h"
#include "src/models/dllHajacing/dllCheck.h"
#include "src/models/serviceMisconfiguration/serMis.h"
#include "src/models/privescCveCheck/fullScanCVE.h"

int main(int argc, char *argv[]) {

    AppOptions opt;
    parseArgs(argc, argv, &opt);

    enableANSI();
    printPanel();
    Sleep(2000);

    if (opt.runEnum) {
        printYellow(L"\n======== Enumeration =========\n");
        genetorMain();
        printYellow(L"\n======================================\n");
    }

    if (opt.runSeImpersonate) {
        printYellow(L"\n======== SeImpersonate Check =========\n");
        mainCheckSeImp();
        printYellow(L"\n======================================\n");
    }

    if (opt.runUSP) {
        printYellow(L"\n======== Unquoted Service Path =========\n");
        runUspPathCheck();
        printYellow(L"\n======================================\n");

    }

    if (opt.runDllHijack) {
        printYellow(L"\n======== DLL Hijack Check =========\n");
        runDllHijackCheck();
        printYellow(L"\n======================================\n");
    }

    if (opt.runServiceMisconfig) {
        printYellow(L"\n======== Service Misconfiguration =========\n");
        ScanServiceMisconfigs();
        PrintServiceReport();
        printYellow(L"\n======================================\n");
    }

    printYellow(L"\n======== Privilege Escalation CVE Scan =========\n");
    fullScanCVE();
    printYellow(L"\n======================================\n");

    return 0;
}


/*
RUN COMMAND:
 x86_64-w64-mingw32-gcc loc-win.c src/models/enum/enum.c src/models/enum/src/UserAndHost.c src/models/enum/src/installApps.c src/models/enum/src/networkInfo.c src/models/enum/src/osInfo.c src/models/enum/src/prosesRun.c src/models/enum/src/userGrops.c src/models/color/colorPrint.c src/models/SeImperSona/seImperSona.c src/models/usp/getUspPaht.c src/models/dllHajacing/dllCheck.c src/models/serviceMisconfiguration/serMis.c -o mainLOC.exe -ladvapi32 -lshlwapi -lnetapi32 -liphlpapi -lversion -lws2_32 -lole32
*/