#ifndef KernelServicesDotH
#define KernelServicesDotH
// incluir los servicios
#include "DisplayServices.h"
#include "MemoryServices.h"
#include "IOServices.h"
#include "DiskServices.h"
// status de error
typedef enum _KernelStatus {
    // que se pudo
    KernelStatusSuccess = 0,
    // que no se encontro
    KernelStatusNotFound = 1,
    // que fue tan desastrozo que nisiquiera se molesto en decirlo
    KernelStatusDisaster = 2,
    // que se quedo congelado
    KernelStatusInfiniteLoopTimeouted = 3,
    // que no hay presupuesto
    KernelStatusNoBudget = 4,
    // error de memoria
    KernelStatusMemoryRot = 5,
    // error de disco
    KernelStatusDiskServicesDiskErr = 6
} KernelStatus;
// el tipo para ejecutar un comando
typedef void (*KernelServicesExecuteCommand)(struct _KernelServices* Services, char* command, int len);
// el tipo para engendrar servicios
typedef void (*KernelServicesGiveBirth)(struct _KernelServices*);
// el tipo para apagar o reiniciar
typedef KernelStatus (*KernelServicesReset)(int func);
// el tipo de servicios otros
typedef struct _KernelMiscServices {
    // el servicio de comandos
    KernelServicesExecuteCommand Run;
    // crea uno nuevo
    KernelServicesGiveBirth GiveBirth;
    // para reiniciar o apagar (0=reiniciar, 1=apagar)
    KernelServicesReset Reset;
} KernelMiscServices;
// para los servicios principales
typedef struct _KernelServices {
    // version de servicios
    unsigned int* ServicesVersion;
    // para la pantalla
    DisplayServices* Display;
    // para la memoria
    MemoryServices* Memory;
    // el servicio de I/O
    IoServices* InputOutpud;
    // el servicio de disco
    DiskServices* File;
    // servicios miscelianos
    KernelMiscServices* Misc;
} KernelServices;
#endif