#ifndef _ServicesMemoryPoolDotH_
#define _ServicesMemoryPoolDotH_
// prototipos
#include "../basic.h"
/* el tipo actual de allocation */
ModuAllocType MemoryCurrentSystem;
/* Bloque de memoria y su entrada de memoria */
typedef struct _KernelPool {
    /* tamaño del bloque */
    uint32_t size;
    /* fue liberado */
    bool free;
    /* siguiente */
    struct _KernelPool* next;
    /* tipo de memoria */
    enum _ModuAllocType Type;
    /* para saber donde esta */
    bytes MemoryPtr;
} KernelPool;
#endif