#include "../library/lib.h"

KernelStatus ErickMain(KernelServices* Services)
{
    /* cargar snetpst */
    FatFile SimpleNetPkgSetDriverFile = Services->File->OpenFile("/dev/snetpst");
    ObjectAny SimpleNetPkgSetDriver;
    Entero SimpleNetPkgSetDriverSize;

    KernelStatus Status = Services->File->GetFile(SimpleNetPkgSetDriverFile, &SimpleNetPkgSetDriver, &SimpleNetPkgSetDriverSize);
    if (Status) return Status;

    /* cargar snetpr */
    FatFile SimpleNetPkgReciveDriverFile = Services->File->OpenFile("/dev/snetpr");
    ObjectAny SimpleNetPkgReciveDriver;
    Entero SimpleNetPkgReciveDriverSize;

    Status = Services->File->GetFile(SimpleNetPkgReciveDriverFile, &SimpleNetPkgReciveDriver, &SimpleNetPkgReciveDriverSize);
    if (Status) return Status;

    /* cargar snetus*/
    FatFile SimpleNetPkgUnSetDriverFile = Services->File->OpenFile("/dev/snetus");
    ObjectAny SimpleNetPkgUnSetDriver;
    Entero SimpleNetPkgUnSetDriverSize;

    Status = Services->File->GetFile(SimpleNetPkgUnSetDriverFile, &SimpleNetPkgUnSetDriver, &SimpleNetPkgUnSetDriverSize);
    if (Status) return Status;

    uint8_t* NetRecive = Services->Memory->AllocatePool(8192);

    // enviar el set
    Services->Packages->Launch(KPackageToDriver, (uint32_t)NetRecive);
    // ejecutar driver
    Services->Misc->RunBinary(SimpleNetPkgSetDriver, SimpleNetPkgSetDriverSize, Services);

    // esperar
    Services->Time->TaskDelay(20);

    uint8_t* Data = Services->Memory->AllocatePool(256);
    int32_t Size;

    Services->Packages->Launch(KPackageToDriver, (uint32_t)Data);
    Services->Packages->Launch(KPackageToDriver, (uint32_t)&Size);
    Services->Packages->Launch(KPackageToDriver, (uint32_t)NetRecive);

    Services->Misc->RunBinary(SimpleNetPkgReciveDriver, SimpleNetPkgReciveDriverSize, Services);
    Services->Display->printg(Data);

    Services->Misc->RunBinary(SimpleNetPkgUnSetDriver, SimpleNetPkgUnSetDriverSize, Services);

    /* liberar */

    Services->Memory->FreePool(NetRecive);
    Services->Memory->FreePool(Data);

    Services->Memory->FreePool(SimpleNetPkgSetDriver);
    Services->Memory->FreePool(SimpleNetPkgReciveDriver);
    Services->Memory->FreePool(SimpleNetPkgUnSetDriver);

    Services->File->CloseFile(SimpleNetPkgSetDriverFile);
    Services->File->CloseFile(SimpleNetPkgReciveDriverFile);
    Services->File->CloseFile(SimpleNetPkgUnSetDriverFile);
}