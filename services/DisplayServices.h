#ifndef DisplayServicesDotH
#define DisplayServicesDotH
// el tipo
struct _DisplayServices;
// el tipo de impresion interna
typedef void (*KernelSimpleDisplayPrintgInternal)(char* msg, unsigned int line);
// el tipo de impresion
typedef void (*KernelSimpleDisplayPrintg)(char* msg);
// ir a
typedef void (*KernelSimpleDisplayGoto)(int col, int row);
// ir a
typedef void (*KernelSimpleDisplayGoto)(int col, int row);
// el tipo de limpieza
typedef void (*KernelSimpleDisplayClearScreen)();
// el tipo de atributos
typedef void (*KernelSimpleDisplaySetAttrs)(char bg, char fg);
// el tipo de setteo
typedef void (*KernelSimpleDisplaySetAsDefault)(struct _DisplayServices* this);
// el tipo de servicios de pantalla
typedef struct _DisplayServices {
    // la linea actual
    int CurrentLine;
    // el caracter actual
    int CurrentCharacter;
    // el atributo actual
    char CurrentAttrs;
    // para seleccionar
    KernelSimpleDisplaySetAsDefault Set;
    // para imprimir
    KernelSimpleDisplayPrintgInternal printg_i;
    // para imprimir sin necesitar la linea
    KernelSimpleDisplayPrintg printg;
    // para limpiar la pantalla
    KernelSimpleDisplayClearScreen clearScreen;
    // para ir a algun lado
    KernelSimpleDisplayGoto setCursorPosition;
    // ajustar attributos
    KernelSimpleDisplaySetAttrs setAttrs;
} DisplayServices;
#endif