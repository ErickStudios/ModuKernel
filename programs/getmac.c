#include "../library/lib.h"

KernelStatus ErickMain(KernelServices* Services)
{
    // cargar driver

    FatFile SimpleNetMacDriverFile = Services->File->OpenFile("/dev/snetm");
    ObjectAny SimpleNetMacDriver;
    Entero SimpleNetMacDriverSize;

    // abrir driver
    KernelStatus OpenDriver = Services->File->GetFile(SimpleNetMacDriverFile, &SimpleNetMacDriver, &SimpleNetMacDriverSize);

    // si hubo un error
    if (OpenDriver)
    {
        // notificar
        Services->Display->printg("la dependencia no esta instalada: /dev/snetm");

        // retornar estado
        return OpenDriver;
    }

    // crear pool
    uint8_t* MacAddress = Services->Memory->AllocatePool(6);

    // lanzar el paquete
    Services->Packages->Launch(KPackageToDriver, (uint32_t)MacAddress);

    // ejecutar driver
    Services->Misc->RunBinary(SimpleNetMacDriver, SimpleNetMacDriverSize, Services);

    for (size_t i = 0; i < 6; i++)
    {
        uint8_t AddressPart = MacAddress[i];
        char inttostr[5];
        IntToHexString((int)AddressPart, inttostr);

        Services->Display->printg(inttostr);
        if (i != 5) Services->Display->printg(".");
    }
    

    return KernelStatusSuccess;
}