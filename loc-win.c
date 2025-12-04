#include "src/models/enum/enum.h" // 1
#include "src/models/color/colorPrint.h"
#include "src/models/SeImperSona/seImperSona.h" // 2
#include "src/models/usp/getUspPaht.h" // 3
#include "src/models/dllHajacing/dllCheck.h" // 4
#include "src/models/serviceMisconfiguration/serMis.h" // 5
#include <stdio.h>

int main() {
    // enumaration
    genetorMain();
    // se impersonate privilege check
    printf("\n");
    printYellow(L"\n======== SeImpersonatePrivilege Check =========\n");
    mainCheckSeImp();
    printYellow(L"\n==============================================\n");
    // unquoted service path check
    printf("\n");
    printYellow(L"\n======== Unquoted Service Path Check =========\n");
    runUspPathCheck();
    printYellow(L"\n==============================================\n");
    // dll hijack check
    printf("\n");
    printYellow(L"\n======== DLL Hijack Service Check =========\n");
    runDllHijackCheck();
    printYellow(L"\n==============================================\n");
    // service misconfiguration check
    printf("\n");
    printYellow(L"\n======== Service Misconfiguration Check =========\n");
    ScanServiceMisconfigs();
    PrintServiceReport();
    printYellow(L"\n==============================================\n");
    return 0;

}

/*
RUN COMMAND:
 x86_64-w64-mingw32-gcc loc-win.c src/models/enum/enum.c src/models/enum/src/UserAndHost.c src/models/enum/src/installApps.c src/models/enum/src/networkInfo.c src/models/enum/src/osInfo.c src/models/enum/src/prosesRun.c src/models/enum/src/userGrops.c src/models/color/colorPrint.c src/models/SeImperSona/seImperSona.c src/models/usp/getUspPaht.c src/models/dllHajacing/dllCheck.c src/models/serviceMisconfiguration/serMis.c -o mainLOC.exe -ladvapi32 -lshlwapi -lnetapi32 -liphlpapi -lversion -lws2_32 -lole32
*/