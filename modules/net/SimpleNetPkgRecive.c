#include "../../library/lib.h"
#include "ports.h"
#include "pkg.h"

KernelStatus ErickMain(KernelServices* Services)
{
    KernelPackage Pkg3 = Services->Packages->Catch();
    KernelPackage Pkg2 = Services->Packages->Catch();
    KernelPackage Pkg1 = Services->Packages->Catch();

    uint8_t* Data = (uint8_t*)(uintptr_t)Pkg1.Data;
    int32_t* Size = ((int32_t*)((uintptr_t)Pkg2.Data));
    uint8_t* BufferRam = (uint8_t*)(uintptr_t)Pkg3.Data;

    rx_buffer = BufferRam;
    *Size = rtl8139_receive(Data);

    return KernelStatusSuccess;
}