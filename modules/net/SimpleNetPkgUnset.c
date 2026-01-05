#include "../../library/lib.h"
#include "ports.h"
#include "pkg.h"

KernelStatus ErickMain(KernelServices* Services)
{
    KernelPackage Pkg1 = Services->Packages->Catch();

    uint8_t* Buffer = (uint8_t*)(uintptr_t)Pkg1.Data;

    rtl8139_deinit_rx();

    return KernelStatusSuccess;
}