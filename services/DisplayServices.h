#ifndef DisplayServicesDotH
#define DisplayServicesDotH
#include "basic.h"
/* el tipo */
struct _DisplayServices;
/* el tipo de impresion interna */
typedef void (*KernelSimpleDisplayPrintgInternal)(char* msg, unsigned int line);
/* el tipo de impresion */
typedef void (*KernelSimpleDisplayPrintg)(char* msg);
/* ir a */
typedef void (*KernelSimpleDisplayGoto)(int col, int row);
/* el tipo de limpieza */
typedef void (*KernelSimpleDisplayClearScreen)();
/* el tipo de atributos */
typedef void (*KernelSimpleDisplaySetAttrs)(char bg, char fg);
/* el tipo de setteo */
typedef void (*KernelSimpleDisplaySetAsDefault)(struct _DisplayServices* this);
/* el tipo de pantalla iniciando */
typedef void (*KernelSimpleDisplayInitGopScreen)();
/* el tipo para imprimir pixeles */
typedef void (*KernelSimpleDisplayDrawRectangle)(uint8_t color, int x, int y, int size);
/* el tipo de servicios de pantalla */
typedef struct _DisplayServices {
    /* variable que contiene la linea actual donde el cursor de escritura de
    printg se encuentra ubicado */
    int32_t CurrentLine;
    /* variable que contiene la colummna actual donde el cursor de escritura de
    printg se encuentra ubicado */
    int32_t CurrentCharacter;
    /* variable que contiene el atributo actual Fondo y Texto que el printg
    utilizara para imprimir */
    int8_t CurrentAttrs;
    /* funcion que ajusta los punteros que usa printg a las propiedades CurrentLine
    , CurrentCharacter y CurrentAttrs para que printg los obtenga automaticamente ,
    no se preocupen, el kernel ya hace esto por defecto durante el arranque asi que
    a menos de que requieran otro servicio de display paralelo no van a tener
    que usar manualmente esta funcion, esta funcion contiene un parametro que es
    el puntero a esta estructura */
    KernelSimpleDisplaySetAsDefault Set;
    /* funcion que es la funcion basica de impresion que no es automatica y
    requiere como parametro ademas del texto de en ascii el parametro a la linea
    donde se imprimira el texto, esta funcion no usa los parametros y punteros
    de atributos de color y siempre imprimira en 0x07 (color gris en fondo negro) */
    KernelSimpleDisplayPrintgInternal printg_i;
    /* funcion para imprimir, esta es la central y mas facil de usar que todos
    deben usar si es que el kernel ya esta inicializado y los servicios de display 
    ya esten despiertos, requiere como unico parametro el texto que esta en formato
    ascii */
    KernelSimpleDisplayPrintg printg;
    /* funcion para limpiar la pantalla de la consola */
    KernelSimpleDisplayClearScreen clearScreen;
    /* funcion para establezer la posicion del cursor de printg, requiere como
    parametro la columna (la posicion en x) y la linea (la posicion y), esta funcion
    setea automaticamente los punteros que apuntan a la estructura de servicios de 
    display que se usa actualmente, puedes cambiarla con el metodo Set en esta
    misma estructura */
    KernelSimpleDisplayGoto setCursorPosition;
    /* funcion para ajustar los atributos del texto y fondo manualmente de forma
    mas facil sin usar offsets, requiere como parametros un int8 que contiene el
    color del fondo (0x0 a 0xF) y el segundo parametro es otro int8 que contiene
    el color del texto (0x0 a 0xF), esto ajusta los punteros a la estructura
    de display actual que se puede cambiar con la funcion Set */
    KernelSimpleDisplaySetAttrs setAttrs; 
    /* funcion para inicializar la pantalla grafica, lamentablemente al probarlo
    no se puede volver a el modo texto por lo que el sistema necesita vivir en
    modo grafico al hacerlo , aparte disclamer, usa muchos bugs para despertar
    por completo la tarjeta grafica*/
    KernelSimpleDisplayInitGopScreen ActivatePixel;
    /* funcion para dibujar un rectangulo , el es el color del esquema de
    colores (0-255) para dibujarlo, el segundo y tercer parametro son las
    posiciones X,Y del pixel a dibujar, el cuarto es el tamaño del rectangulo
    en pixeles, no hay tamaño x ni tamaño y ya que esto utiliza la cantidad
    de pixeles del cuarto parametro para eso*/
    KernelSimpleDisplayDrawRectangle DrawRectangle;
    /* puntero a la flag que indica si esta en modo grafico o en modo texto
    , 1 si esta en grafico, y 0 si esta en modo texto */
    uint8_t* IsInPixelsMode;
} DisplayServices;
#endif