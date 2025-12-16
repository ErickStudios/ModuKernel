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
} MemoryServices;
#endif