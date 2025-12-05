#ifndef ARGS_H
#define ARGS_H

typedef struct {
    int runEnum;
    int runSeImpersonate;
    int runUSP;
    int runDllHijack;
    int runServiceMisconfig;
} AppOptions;

void parseArgs(int argc, char *argv[], AppOptions *opt);

#endif
