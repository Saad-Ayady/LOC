#include "../color/colorPrint.h"
#include "cve/CVE-2024-21338.h"
#include "cve/CVE-2021-36934.h"
#include "fullScanCVE.h"

void fullScanCVE() {
    CVE_2024_21338();
    CVE_2021_36934();
    
    printInfo(L"\nFull CVE Scan Complete.\n");
}
