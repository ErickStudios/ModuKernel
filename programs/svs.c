#include "../library/lib.h"

KernelStatus ErickMain(KernelServices* Services) {
    volatile uint32_t *ptr = (uint32_t*)0x0010C19A; 

    // Intento de escritura: tu SVS debería interceptar este MOV
    *ptr = 0xDEADBEEF;

    // Si el SVS funciona, nunca deberías llegar aquí
    return 0;
}