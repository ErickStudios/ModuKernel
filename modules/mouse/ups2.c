/* mouse ps2 para inicializarlo */

#include "../../library/lib.h"

// enviar comando al mouse
void MouseWrite(uint8_t val) {
    // esperar a que el controlador esté listo
    while (gIOS->Input(0x64) & 0x02);
    gIOS->Outpud(0x64, 0xD4); // decir que el siguiente byte es para el mouse
    while (gIOS->Input(0x64) & 0x02);
    gIOS->Outpud(0x60, val);
}

// leer respuesta del mouse
uint8_t MouseRead() {
    while (!(gIOS->Input(0x64) & 0x01));
    return gIOS->Input(0x60);
}

// desinicializar mouse
void MouseDeinit() {
    // deshabilitar dispositivo secundario (mouse)
    gIOS->Outpud(0x64, 0xA7);

    // deshabilitar interrupciones del mouse
    gIOS->Outpud(0x64, 0x20);
    uint8_t status = gIOS->Input(0x60) & ~2;
    gIOS->Outpud(0x64, 0x60);
    gIOS->Outpud(0x60, status);

    // desactivar reporte de datos
    MouseWrite(0xF5);
    MouseRead(); // esperar ACK
}

/* funcion para el inicio*/
KernelStatus ErickMain(KernelServices* Services)
{
    // inicializar libreria
    InitializeLibrary(Services);
    
    // inicializar mouse
    MouseDeinit();

    return KernelStatusSuccess;
}