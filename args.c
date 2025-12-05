#include <string.h>
#include <stdio.h>
#include "args.h"
#include <stdlib.h>

void parseArgs(int argc, char *argv[], AppOptions *opt) {

    // defaults - run everything
    opt->runEnum = 1;
    opt->runSeImpersonate = 1;
    opt->runUSP = 1;
    opt->runDllHijack = 1;
    opt->runServiceMisconfig = 1;

    if (argc <= 1) return; // no args → run everything

    for (int i = 1; i < argc; i++) {

        if (strcmp(argv[i], "--no-enum") == 0)
            opt->runEnum = 0;

        else if (strcmp(argv[i], "--no-imp") == 0)
            opt->runSeImpersonate = 0;

        else if (strcmp(argv[i], "--no-usp") == 0)
            opt->runUSP = 0;

        else if (strcmp(argv[i], "--no-dll") == 0)
            opt->runDllHijack = 0;

        else if (strcmp(argv[i], "--no-serv") == 0)
            opt->runServiceMisconfig = 0;

        else if (strcmp(argv[i], "--help") == 0) {
            printf("\nAvailable Arguments:\n");
            printf("  --no-enum    : Disable enumeration module\n");
            printf("  --no-imp     : Disable SeImpersonate check\n");
            printf("  --no-usp     : Disable Unquoted Service Path scan\n");
            printf("  --no-dll     : Disable DLL Hijacking scan\n");
            printf("  --no-serv    : Disable Service Misconfiguration scan\n");
            printf("\n");
            exit(0);
        }
    }
}
