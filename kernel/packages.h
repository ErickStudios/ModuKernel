#include "../services/packages/package.h"

uint32_t PackagesIndexCount = 0;
KernelPackage* PackagesKernel = 0;

void InternalPackageLaunch(KernelPackageType Type, uint32_t Data)
{
    KernelPackage Package;
    Package.Author = Type;
    Package.Data   = Data;

    // reservar nuevo espacio
    KernelPackage* New = (KernelPackage*) AllocatePool(sizeof(KernelPackage) * (PackagesIndexCount + 1));

    // copiar los paquetes anteriores si existen
    if (PackagesKernel != NULL && PackagesIndexCount > 0) {
        InternalMemoryCopy(New, PackagesKernel, sizeof(KernelPackage) * PackagesIndexCount);
        FreePool(PackagesKernel);
    }

    // asignar el nuevo paquete en el último slot
    New[PackagesIndexCount] = Package;

    // actualizar contador y puntero
    PackagesIndexCount++;
    PackagesKernel = New;
}
KernelPackage InternalPackageCatch() {
    KernelPackage empty = {0}; // paquete vacío si no hay nada
    if (PackagesIndexCount == 0) {
        return empty;
    }

    // Obtener el último paquete
    KernelPackage pkg = PackagesKernel[PackagesIndexCount - 1];

    // Crear nuevo buffer con uno menos
    uint32_t newCount = PackagesIndexCount - 1;
    KernelPackage* New = NULL;

    if (newCount > 0) {
        New = (KernelPackage*) AllocatePool(sizeof(KernelPackage) * newCount);
        InternalMemoryCopy(New, PackagesKernel, sizeof(KernelPackage) * newCount);
    }

    // Liberar el buffer anterior
    FreePool(PackagesKernel);

    // Actualizar puntero y contador
    PackagesKernel = New;
    PackagesIndexCount = newCount;

    // Devolver el paquete extraído
    return pkg;
}