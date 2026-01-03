#ifndef PackageServicesDotH
#define PackageServicesDotH
#include "packages/package.h"
/* el tipo de lanzar */
typedef void (*KernelPackagesLaunch)(KernelPackageType Type, uint32_t Data);
/* el tipo de atrapar */
typedef KernelPackage (*KernelPackagesCatch)();
/* la estructura */
typedef struct _PackageServices {
    /* funcion para lanzar un paquete al vacio para ver quien
    lo atrapa y que hace con el */
    KernelPackagesLaunch Launch;
    /* funcion para atrapar paquetes que han venido de la nada*/
    KernelPackagesCatch Catch;
} PackageServices;
#endif