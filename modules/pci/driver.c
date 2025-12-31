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
    // informacion
    ActionDriver DriverFunction = Services->Info->DriverParams[0];
    uint32_t* OutParam          = (uint32_t*)((uintptr_t)(Services->Info->DriverParams[1]));

    if (DriverFunction == DriverPciReadConfig)
        *OutParam = pci_read_config(
            (uint8_t)Services->Info->DriverParams[2],
            (uint8_t)Services->Info->DriverParams[3], 
            (uint8_t)Services->Info->DriverParams[4],
            (uint8_t)Services->Info->DriverParams[5]
        );
    else if (DriverFunction == DriverPciWriteConfig)
        pci_write_config(
            (uint8_t)Services->Info->DriverParams[2],
            (uint8_t)Services->Info->DriverParams[3], 
            (uint8_t)Services->Info->DriverParams[4],
            (uint8_t)Services->Info->DriverParams[5],
            *OutParam
        );

    return KernelStatusSuccess;
}