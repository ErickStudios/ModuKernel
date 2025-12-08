// libreria
#include "../services/KernelServices.h"

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

// funcion de print