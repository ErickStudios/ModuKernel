#ifndef IOServicesDotH
#define IOServicesDotH
// para teclas especiales
#define KernelSimpleIoSpecKey(Id) (0xC0 + Id)
// para teclas de funcion
#define KernelSimpleIoFuncKey(Id) (0xD0 + Id)
// el tipo de input
typedef unsigned char (*KernelSimpleIoInput)(unsigned short port);
// el tipo de outpud
typedef void (*KernelSimpleIoOutpud)(unsigned short port, unsigned char val);
// el tipo de teclado y tecla
typedef unsigned char (*KernelSimpleIoKeyWait)();
// el tipo de leer linea
typedef char* (*KernelSimpleIoReadLine)();
// el tipo de servicios de salida
typedef struct _IoServices {
    // para obtener input
    KernelSimpleIoInput Input;
    // para mandar outpud
    KernelSimpleIoOutpud Outpud;
    // para esperar una tecla
    KernelSimpleIoKeyWait WaitKey;
    // leer linea
    KernelSimpleIoReadLine ReadLine;
} IoServices;
#endif