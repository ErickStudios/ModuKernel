#ifndef KernelServicesDotH
#define KernelServicesDotH
// incluir los servicios
#include "DisplayServices.h"
#include "MemoryServices.h"
#include "IOServices.h"
#include "DiskServices.h"
// el tipo para ejecutar un comando
typedef void (*KernelServicesExecuteCommand)(struct _KernelServices* Services, char* command, int len);
// para los servicios principales
typedef struct _KernelServices {
    // para la pantalla
    DisplayServices* Display;
    // para la memoria
    MemoryServices* Memory;
    // el servicio de I/O
    IoServices* InputOutpud;
    // el servicio de disco
    DiskServices* File;
    // el servicio de comandos
    KernelServicesExecuteCommand Run;
} KernelServices;
#endif