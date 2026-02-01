#ifdef __cplusplus
extern "C" {
#endif
#ifndef _ModuKernelBaseLibrary_
#define _ModuKernelBaseLibrary_
// libreria
#include <stdint.h>
#include "../services/KernelServices.h"
#include "../fs/fat12.h"

#include "../libc/String.h"

static uint16_t inw(uint16_t port) {
    uint16_t ret;
    __asm__ __volatile__("inw %1, %0" : "=a"(ret) : "Nd"(port));
    return ret;
}
static inline void outw(unsigned short port, unsigned short value)
{
    __asm__ __volatile__("outw %0, %1" : : "a"(value), "Nd"(port));
}

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

typedef unsigned long      size_t;

typedef signed char        Mini;
typedef short              Corto;
typedef int                Entero;
typedef long long          Grande;
typedef unsigned char      MiniPositivo;
typedef unsigned short     CortoPositivo;
typedef unsigned int       EnteroPositivo;
typedef unsigned long long GrandePositivo;

typedef unsigned int       DriverDataIo;

typedef char*               string;

typedef GrandePositivo     DireccionAValor;

#define BanderaActiva       1
#define BanderaNoActiva     0

// teclas
#define EscaneoFlechaArriba KernelSimpleIoSpecKey(1)
#define EscaneoFlechaAbajo KernelSimpleIoSpecKey(2)
#define EscaneoFlechaIzquierda KernelSimpleIoSpecKey(3)
#define EscaneoFlechaDerecha KernelSimpleIoSpecKey(4)

#define DriverFooterFunctions asm volatile(".long 0xffaa12bb");

// para funciones rapidas
#define GetArgs(Services,Count)                                             \
    int count    = (int)(DireccionAValor)Services->Misc->Paramaters[1];     
#define GetMagic(Services, Magic)                                           \
    int Magic = *(int*)Services->Misc->Paramaters[0];

// inicializa la libreria
void InitializeLibrary(Sys* System)
{
    // inicializar global
    gSys = System;

    // inicializar servicios
    gDS = System->Display;
    gIOS = System->InputOutput;
    gMS = System->Memory;
}

char* AllocateStringArray(char* text)
{
    unsigned int len = (unsigned int)StrLen(text) + 1; // incluye el '\0'

    char* retval = (char*)gMS->AllocatePool(len);
    if (!retval) {
        return 0; // manejo de error si no hay memoria
    }

    gMS->CoppyMemory(retval, text, len);

    return retval;
}

KernelStatus ExecuteFile(char* path)
{
    FatFile Fl = gSys->File->OpenFile(path);
    ObjectAny FlContent;
    Entero FlSize;
    KernelStatus FlOpen = (KernelStatus)gSys->File->GetFile(Fl, &FlContent, &FlSize);

    if (FlOpen) return FlOpen;

    KernelStatus St = gSys->Misc->RunBinary(FlContent, FlSize, gSys);

    gMS->FreePool(FlContent);
    gSys->File->CloseFile(Fl);

    return St;
}

#include "status.h"

#endif
#ifdef __cplusplus
}
#endif