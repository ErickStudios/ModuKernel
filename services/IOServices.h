#ifndef IOServicesDotH
#define IOServicesDotH
// el tipo de impresion interna
typedef unsigned char (*KernelSimpleIoInput)(unsigned short port);
// el tipo de impresion
typedef void (*KernelSimpleIoOutpud)(unsigned short port, unsigned char val);
// el tipo de servicios de pantalla
typedef struct _IoServices {
    // para obtener input
    KernelSimpleIoInput Input;
    // para mandar outpud
    KernelSimpleIoOutpud Outpud;
} IoServices;
#endif