// helloworld.c
#include "../library/lib.h"

KernelStatus ErickMain(KernelServices* Services)
{
    GetArgs(Services, count);
    GetMagic(Services, execType);

    for (int i = 1; i < count; i++) {
        char* arg = (char*)Services->Misc->Paramaters[i + 2];
        Services->Display->printg(arg);
        Services->Display->printg("\n");
    }
    Services->Display->printg("hello world\n");
    return KernelStatusSuccess;
}