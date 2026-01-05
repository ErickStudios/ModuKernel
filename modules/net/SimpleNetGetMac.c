#include "../../library/lib.h"
#include "ports.h"
#include "base.h"

KernelStatus ErickMain(KernelServices* Services)
{
    KernelPackage Pkg1 = Services->Packages->Catch();
    rtl8139_read_mac((uint8_t*)((uintptr_t)Pkg1.Data));
    return KernelStatusSuccess;
}