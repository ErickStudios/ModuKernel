/* para automandarse algo*/

#include "../library/lib.h"

KernelStatus ErickMain(KernelServices* Services)
{
    InitializeLibrary(Services);

    Services->Display->printg("setear el buffer\n");
    // setear el buffer
    uint8_t* BufferNet = Services->Memory->AllocatePool(8192);                  // el buffer
    Services->Packages->Launch(KPackageToDriver, (uint32_t)BufferNet);          // el buffer de la net
    ExecuteFile("/dev/snetst");                                                 // mandar

    Services->Display->printg("ver la mac\n");
    // ver la mac
    uint8_t* MacAddressDest = Services->Memory->AllocatePool(6);                // el dato
    Services->Packages->Launch(KPackageToDriver, (uint32_t)MacAddressDest);     // la mac
    ExecuteFile("/dev/snetm");                                                  // mandar

    Services->Display->printg("enviar datos\n");
    // enviar datos
    char* DataSend = "hello MAC!";                                              // el dato
    Services->Packages->Launch(KPackageToDriver, (uint32_t)MacAddressDest);     // la mac a la que se enviara
    Services->Packages->Launch(KPackageToDriver, (uint32_t)DataSend);           // el dato
    Services->Packages->Launch(KPackageToDriver, (uint32_t)StrLen(DataSend));   // el tamaño del dato
    Services->Packages->Launch(KPackageToDriver, (uint32_t)0);                  // accion: send
    ExecuteFile("/dev/stlq");                                                   // mandar

    uint8_t* Data = Services->Memory->AllocatePool(256);                        // el dato
    int32_t Size;                                                               // el tamaño

    Services->Display->printg("recibir\n");
    // recibir
    Services->Packages->Launch(KPackageToDriver, (uint32_t)Data);               // enviar dato
    Services->Packages->Launch(KPackageToDriver, (uint32_t)&Size);              // enviar parametro
    Services->Packages->Launch(KPackageToDriver, (uint32_t)BufferNet);          // enviar el netrecive
    ExecuteFile("/dev/snetpr");                                                 // mandarlo
    Services->Display->printg(Data);                                            // imprimirlo

    ExecuteFile("/dev/snetus");                                                 // cerrar net

    return KernelStatusSuccess;
}