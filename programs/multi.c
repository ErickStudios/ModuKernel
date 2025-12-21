// para ver la tarea de pila emulada

#include "../library/lib.h"

KernelStatus ErickMain(KernelServices* Services)
{
    char em[] = "hello world\n";
    char em1[] = "apple banana\n";
    char em2[] = "foo bar\n";

    FatFile OtherFile = Services->File->OpenFile("/bin/calculator");

    void* content; int size;

    KernelStatus OpenFileBin = Services->File->GetFile(OtherFile, &content, &size);

    // ejecutar
    if (!(_StatusError(OpenFileBin)))
        Services->Misc->RunBinary(content, size, Services);
    else 
        return OpenFileBin;
    
    Services->Memory->FreePool(content);

    Services->Display->printg(em);
    Services->Display->printg(em1);
    Services->Display->printg(em2);

    return KernelStatusSuccess;
}