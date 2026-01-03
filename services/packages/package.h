#ifndef _KernelPackagesPackage_
#define _KernelPackagesPackage_

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
    KPackageToDriver
} KernelPackageType;

/* el tipo de estructura para un paquete*/
typedef struct _KernelPackage {
    /* el autor del paquete*/
    KernelPackageType Author;
    /* el dato del paquete, en puntero o algo mas*/
    uint32_t Data;
} KernelPackage;

#endif