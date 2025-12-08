#ifndef MemoryServicesDotH
#define MemoryServicesDotH
// el tipo de creacion de memoria dinamica
typedef void* (*KernelMemoryMalloc)(unsigned int size);
// la estructura
typedef struct _MemoryServices {
    // para crear memoria
    KernelMemoryMalloc AllocatePool;
} MemoryServices;
#endif