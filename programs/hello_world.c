// hello_world.c alias KERNEL.BIN
#include "../library/lib.h"

KernelStatus ErickMain(KernelServices* Services)
{
    Services->Display->printg("hello world\n");
    return KernelStatusSuccess;
}