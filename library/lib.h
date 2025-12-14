// libreria
#include "../services/KernelServices.h"

#include "../libc/String.h"

// el tipo de sistema
typedef KernelServices Sys;

// servicios
Sys* gSys;
// variable independiente de display
DisplayServices* gDS;
// variable indepdendiente de IO
IoServices* gIOS;
// variable independiente de Memoria
MemoryServices* gMS;

typedef signed char        Mini;
typedef short              Corto;
typedef int                Entero;
typedef long long          Grande;
typedef unsigned char      MiniPositivo;
typedef unsigned short     CortoPositivo;
typedef unsigned int       EnteroPositivo;
typedef unsigned long long GrandePositivo;

typedef GrandePositivo     DireccionAValor;

// teclas
#define EscaneoFlechaArriba KernelSimpleIoSpecKey(1)
#define EscaneoFlechaAbajo KernelSimpleIoSpecKey(2)
#define EscaneoFlechaIzquierda KernelSimpleIoSpecKey(3)
#define EscaneoFlechaDerecha KernelSimpleIoSpecKey(4)

// inicializa la libreria
void InitializeLibrary(Sys* System)
{
    // inicializar global
    gSys = System;

    // inicializar servicios
    gDS = System->Display;
    gIOS = System->InputOutpud;
    gMS = System->Memory;
}