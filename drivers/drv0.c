// driver0
#include "../library/lib.h"

// entrada
KernelStatus ErickMain(KernelServices* Services)
{
    if (Services->Misc->ParamsCount > 0) {
        int magic = *(int*)Services->Misc->Paramaters[0];  // casteo y desreferencia
        if (magic == 0x3a5b) {
            Services->Display->printg("Hola driver\n");
            return KernelStatusSuccess;
        }
    }

    Services->Display->printg("no se puede cargar como programa\n");
    return KernelStatusInvalidParam;
}