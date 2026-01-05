#include "../../library/lib.h"
#include "ports.h"
#include "pkg.h"

KernelStatus ErickMain(KernelServices* Services)
{
    KernelPackage Pkg2 = Services->Packages->Catch();
    KernelPackage Pkg1 = Services->Packages->Catch();

    uint8_t* Data = (uint8_t*)(uintptr_t)Pkg1.Data;
    uint32_t Len = (uint32_t)Pkg2.Data;

    rtl8139_send(Data, Len);

    return KernelStatusSuccess;
}