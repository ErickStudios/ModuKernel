// hello_world.c
#include "../library/lib.h"

KernelStatus Main(KernelServices* Services)
{
    Services->Display->printg("hello world\n");
    return KernelStatusSuccess;
}

/* punto de entrada que el linker pondrá en offset 0 */
KernelStatus _start(KernelServices* Services)
{
    return Main(Services);
}
