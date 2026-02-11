#ifndef _KernelPackagesPackage_
#define _KernelPackagesPackage_
#include "../basic.h"
/* tipo para el autor del paquete */
typedef enum _KernelPackageType {
    /* desde el kernel (retorno)*/
    KPackageFromKernel,
    /* desde el programa (retorno)*/
    KPackageFromProgram,
    /* desde el dispositivo (retorno)*/
    KPackageFromDriver,
    /* para el kernel (parametro)*/
    KPackageToKernel,
    /* para el programa (parametro)*/
    KPackageToProgram,
    /* para el driver (parametro)*/
    KPackageToDriver,
    /** mascara para hacerlo bus */
    KPackageMaskToMakeItBus = 0x80
} KernelPackageType;
/* el tipo de estructura para un paquete*/
typedef struct _KernelPackage {
    /* el autor del paquete*/
    KernelPackageType Author;
    /* el dato del paquete, en puntero o algo mas*/
    uint32_t Data;
    #ifndef __cplusplus
    /** si es un bus que se puede ir */
    bool IsBus;
    #else
    /** si es un bus que se puede ir */
    char IsBus;
    #endif
    /** cuantos ErickTicks tiene para recibir el paquete
     * de formato bus si no lo recibe en ese tiempo el gc
     * lo eliminara*/
    uint32_t BusTimeOut;
} KernelPackage;
#endif