#include "../../library/lib.h"
#include "pkg.h"
#include "driver.h"

KernelStatus ErickMain(KernelServices* Services)
{
    // inicializar libreria
    InitializeLibrary(Services);

    // driver de paquetes
    KernelPackage PkgAction = Services->Packages->Catch();

    // accion
    enum _TeleqDriverAction Action = PkgAction.Data;

    // mandar
    if (Action == TeleqDriverSendLan)
    {
        // direccion
        uint8_t* MacAddress = Services->Memory->AllocatePool(6);
        Services->Packages->Launch(KPackageToDriver, (uint32_t)MacAddress);

        // obtener ip
        ExecuteFile("/dev/snetm");

        // paquetes
        KernelPackage Pkg3 = Services->Packages->Catch();
        KernelPackage Pkg2 = Services->Packages->Catch();
        KernelPackage Pkg1 = Services->Packages->Catch();

        // parametros
        uint8_t* dest_mac = (uint8_t*)(uintptr_t)Pkg1.Data;
        uint8_t* Data = (uint8_t*)(uintptr_t)Pkg2.Data;
        uint32_t SizeData = Pkg3.Data;

        // mandar
        struct _Telecomunication hdr;
        Services->Memory->CoppyMemory(hdr.Hdr, "MODU", 4);
        Services->Memory->CoppyMemory(hdr.From, MacAddress, 6);
        Services->Memory->CoppyMemory(hdr.To, dest_mac, 6);
        hdr.DataSize = SizeData;

        // memoria y datos
        uint8_t* DataTlq = Services->Memory->AllocatePool(sizeof(struct _Telecomunication) + SizeData);

        // hacer el buffer
        Services->Memory->CoppyMemory(DataTlq, &hdr, sizeof(struct _Telecomunication));
        Services->Memory->CoppyMemory(DataTlq + sizeof(struct _Telecomunication), Data, SizeData);
        
        // lanzar paquetes
        Services->Packages->Launch(KPackageToDriver, (uint32_t)DataTlq);
        Services->Packages->Launch(KPackageToDriver, (uint32_t)(sizeof(struct _Telecomunication) + SizeData));

        // recivir
        ExecuteFile("/dev/snetps");
    
        Services->Memory->FreePool(MacAddress);
        Services->Memory->FreePool(DataTlq);
    }
    // recivir
    else if (Action == TeleqDriverReciveLan)
    {
        // direccion
        uint8_t* MacAddress = Services->Memory->AllocatePool(6);
        Services->Packages->Launch(KPackageToDriver, (uint32_t)MacAddress);

        // obtener ip
        ExecuteFile("/dev/snetm");

        // mandar
        KernelPackage Pkg4 = Services->Packages->Catch();
        KernelPackage Pkg3 = Services->Packages->Catch();
        KernelPackage Pkg2 = Services->Packages->Catch();
        KernelPackage Pkg1 = Services->Packages->Catch();

        // parametros
        uint8_t* Data = (uint8_t*)(uintptr_t)Pkg1.Data;
        int32_t* Size = ((int32_t*)((uintptr_t)Pkg2.Data));
        uint8_t* BufferRam = (uint8_t*)(uintptr_t)Pkg3.Data;
        uint8_t* dest_mac = (uint8_t*)(uintptr_t)Pkg4.Data;

        Services->Packages->Launch(Pkg1.Author, Pkg1.Data);
        Services->Packages->Launch(Pkg2.Author, Pkg2.Data);
        Services->Packages->Launch(Pkg1.Author, Pkg1.Data);

        // recivir
        ExecuteFile("/dev/snetpr");

        struct _Telecomunication* hdr = (struct _Telecomunication*)Data;

        // Recorrer payload
        uint8_t* payload = Data + sizeof(struct _Telecomunication);

        if (
            (hdr->To[0] == MacAddress[0]) &&
            (hdr->To[1] == MacAddress[1]) &&
            (hdr->To[2] == MacAddress[2]) &&
            (hdr->To[3] == MacAddress[3]) &&
            (hdr->To[4] == MacAddress[4]) &&
            (hdr->To[5] == MacAddress[5])
        )
        {
            Services->Packages->Launch(KPackageFromDriver, payload);
            Services->Packages->Launch(KPackageFromDriver, hdr->DataSize);
        }

        Services->Memory->FreePool(MacAddress);
    }

    return KernelStatusSuccess;
}