/* driver de pci este driver puede obtener la informacion de un pci
y devolversela al usuario, por medio del parametro de retorno del
kernel que es un extra para los programas, como parametros requerira
el puerto pc en parametros de driver */

// libreria
#include "../../library/lib.h"

// funciones
#include "iol.h"
#include "pci.h"

/* acciones */
typedef enum _ActionDriver {
    /* para recibir un driver */
    DriverPciReadConfig,
    /* para leer una configuracion */
    DriverPciWriteConfig
} ActionDriver;

/* entrada */
KernelStatus ErickMain(KernelServices* Services)
{
    KernelPackage Pkg5 = Services->Packages->Catch();
    KernelPackage Pkg4 = Services->Packages->Catch();
    KernelPackage Pkg3 = Services->Packages->Catch();
    KernelPackage Pkg2 = Services->Packages->Catch();
    KernelPackage Pkg1 = Services->Packages->Catch();
    KernelPackage Pkg0 = Services->Packages->Catch();

    // informacion 
    ActionDriver DriverFunction = (ActionDriver)Pkg0.Data;
    uint32_t* OutParam          = (uint32_t*)((uintptr_t)(Pkg1.Data));

    // funcion para leer un driver de PCi
    if (DriverFunction == DriverPciReadConfig)
        *OutParam = pci_read_config(
            (uint8_t)Pkg2.Data,
            (uint8_t)Pkg3.Data, 
            (uint8_t)Pkg4.Data,
            (uint8_t)Pkg5.Data
        );
    // funcion para escribir un driver PCi
    else if (DriverFunction == DriverPciWriteConfig)
        pci_write_config(
            (uint8_t)Pkg2.Data,
            (uint8_t)Pkg3.Data, 
            (uint8_t)Pkg4.Data,
            (uint8_t)Pkg5.Data,
            *OutParam
        );

    return KernelStatusSuccess;
}