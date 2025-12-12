#ifndef MemoryServicesDotH
#define MemoryServicesDotH
// el tipo de creacion de memoria dinamica
typedef void* (*KernelMemoryMalloc)(unsigned int size);
// para liberar memoria
typedef void (*KernelMemoryFreePool)(void* ptr);
// el tipo de mover memoria
typedef void * (*KernelMemoryMove)(void *dest, const void *src, int n);
// el tipo para copiar memoria
typedef void *(*KernelMemoryCoppy)(void *dest, const void *src, unsigned int n);
// el tipo para comparar memoria
typedef int (*KernelMemoryComparate)(const void *s1, const void *s2, unsigned int n);
// la estructura
typedef struct _MemoryServices {
    // para crear memoria
    KernelMemoryMalloc AllocatePool;
    // para mover memoria
    KernelMemoryMove MoveMemory;
    // para copiar memoria
    KernelMemoryCoppy CoppyMemory;
    // para comparar memoria
    KernelMemoryComparate CompareMemory;
    // para liberar memoria
    KernelMemoryFreePool FreePool;
} MemoryServices;
#endif