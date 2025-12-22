#ifndef MemoryServicesDotH
#define MemoryServicesDotH
/* el tipo de creacion de memoria dinamica */
typedef void* (*KernelMemoryMalloc)(unsigned int size);
/* para liberar memoria */
typedef void (*KernelMemoryFreePool)(void* ptr);
/* el tipo de mover memoria */
typedef void *(*KernelMemoryMove)(void *dest, const void *src, int n);
/* el tipo para copiar memoria */
typedef void *(*KernelMemoryCoppy)(void *dest, const void *src, unsigned int n);
/* el tipo para comparar memoria */
typedef int (*KernelMemoryComparate)(const void *s1, const void *s2, unsigned int n);
/* el tipo para ver cuanto queda de heap */
typedef unsigned int (*KernelMemoryGetFreeHeapSize)();
/* enum para los tipos de memoria dinamica del Kernel */
typedef enum _ModuAllocType
{
    /* para el tipo de memoria para los servicios
    del kernel por ejemplo el display, io, memoria */
    MemAllocTypeKernelServices,
    /* tipo de rastreo de programas, que hacen los programas
    al crear memoria */
    MemAllocTypePrograms,
    /* el tipo de memoria del sistema, que se puede hacer para
    el tipo de memoria del sistema en la shell */
    MemAllocTypeSystem,
    /* el tipo de memoria liberada o desconozida */
    MemAllocTypeFreeOrNotExist,
    /* el tipo de memoria de pila de programas, esta memoria se
    reserva para apilar los programas uno por uno para poder hacer
    un stack de programas y no sobreescribir ninguno de los datos*/
    MemAllocTypeProgramsStackMemory,
    /* el tipo de memoria ultimo */
    MemAllocTypeMaxMemoryType
} ModuAllocType;
/* la estructura */
typedef struct _MemoryServices {
    /* funcion para asignar memoria dinamica usa un unico parametro
    ese es el tamaño de la memoria a reservar, retorna un puntero
    generico a la memoria asignada en el heap */
    KernelMemoryMalloc AllocatePool;
    /* funcion para mover memoria, el primer parametro que usa sirve
    para indicar la direccion al destino a copiar, el segundo parametro
    sirve para indicar de donde se sacara esos datos, el tercero es el
    numero de bytes que se moveran, todo se seteara a 0 */
    KernelMemoryMove MoveMemory;
    /* funcion para copiar memoria, el primer parametro que usa sirve
    para indicar la direccion al destino a copiar, el segundo parametro
    sirve para indicar de donde se sacara esos datos, el tercero es el
    numero de bytes que se copiaran */
    KernelMemoryCoppy CoppyMemory;
    /* funcion para comparar memoria, el primer parametro contiene el primer
    array a los bytes que se compararan, el segundo contiene el segundo array
    a los bytes que se compararan, el ultimo tiene el numero de bytes que
    se compararan de los dos, esta funcion compara byte a byte, retorna
    el numero de diferencias */
    KernelMemoryComparate CompareMemory;
    /* funcion para liberar memoria del heap que anteriormente fue creada
    con AllocatePool, el primer y unico parametro contiene el puntero
    que este no necesita direccion solamente necesitas poner el nombre
    de la variable a la que le hiciste AllocatePool anteriormente */
    KernelMemoryFreePool FreePool;
    /* funcion para obtener cuanto espacio del heap queda en el sistema
    operativo */
    KernelMemoryGetFreeHeapSize GetFreeHeap;
    /* variable que contiene un puntero a el tipo de allocate del sistema
    para poder cambiar entre tipos */
    ModuAllocType* MallocType;
} MemoryServices;
#endif