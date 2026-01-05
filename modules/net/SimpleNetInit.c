#include "../../library/lib.h"
#include "ports.h"
#include "base.h"

KernelStatus ErickMain(KernelServices* Services)
{
    rtl8139_reset();
    return KernelStatusSuccess;
}