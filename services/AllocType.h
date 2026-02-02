#ifndef _ModuAllocTypeDotH_
#define _ModuAllocTypeDotH_

#ifdef __cplusplus
// En C++ basta con declarar el enum directamente
enum ModuAllocType {
#else
// En C se usa typedef para crear el alias
/** el enum del el tipo de memoria dinamica */
typedef enum _ModuAllocType {
#endif
    /* para el tipo de memoria para los servicios
       del kernel por ejemplo el display, io, memoria */
    MemAllocTypeKernelServices,
    /* tipo de rastreo de programas, que hacen los programas
       al crear memoria */
    MemAllocTypePrograms,
    /* el tipo de memoria del sistema, que se puede hacer para
       el tipo de memoria del sistema en la shell */
    MemAllocTypeSystem,
    /* el tipo de memoria liberada o desconocida */
    MemAllocTypeFreeOrNotExist,
    /* el tipo de memoria de pila de programas, esta memoria se
       reserva para apilar los programas uno por uno para poder hacer
       un stack de programas y no sobreescribir ninguno de los datos */
    MemAllocTypeProgramsStackMemory,
    /* el tipo de memoria último */
    MemAllocTypeMaxMemoryType
#ifdef __cplusplus
}; // en C++ ya queda como ModuAllocType
#else
} ModuAllocType; // en C se crea el alias ModuAllocType
#endif

#endif