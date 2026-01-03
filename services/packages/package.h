#ifndef _KernelPackagesPackage_
#define _KernelPackagesPackage_

typedef enum _KernelPackageType {
    KPackageFromKernel,
    KPackageFromProgram,
    KPackageFromDriver,
    KPackageToKernel,
    KPackageToProgram,
    KPackageToDriver
} KernelPackageType;

typedef struct _KernelPackage {
    KernelPackageType Author;
    uint32_t Data;
} KernelPackage;

#endif