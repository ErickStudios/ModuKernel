#include "../library/lib.h"

// enviar comando al mouse
void mouse_write(uint8_t val) {
    // esperar a que el controlador esté listo
    while (gIOS->Input(0x64) & 0x02);
    gIOS->Outpud(0x64, 0xD4); // decir que el siguiente byte es para el mouse
    while (gIOS->Input(0x64) & 0x02);
    gIOS->Outpud(0x60, val);
}

// leer respuesta del mouse
uint8_t mouse_read() {
    while (!(gIOS->Input(0x64) & 0x01));
    return gIOS->Input(0x60);
}

// inicializar mouse
void mouse_init() {
    // habilitar dispositivo secundario (mouse)
    gIOS->Outpud(0x64, 0xA8);

    // habilitar interrupciones del mouse
    gIOS->Outpud(0x64, 0x20);
    uint8_t status = gIOS->Input(0x60) | 2;
    gIOS->Outpud(0x64, 0x60);
    gIOS->Outpud(0x60, status);

    // activar reporte de datos
    mouse_write(0xF4);
    mouse_read(); // esperar ACK
}

typedef struct {
    int x;
    int y;
    uint8_t buttons; // bit0=izq, bit1=der, bit2=medio
} MouseState;

MouseState gMouse;
#define SCREEN_WIDTH  80   // columnas
#define SCREEN_HEIGHT 25   // filas

void mouse_handler() {
    static uint8_t cycle = 0;
    static int8_t packet[3];

    uint8_t data = gIOS->Input(0x60);

    // sincronizar: primer byte siempre tiene bit3=1
    if (cycle == 0 && !(data & 0x08)) {
        return; // descartar si no está alineado
    }

    packet[cycle++] = data;

    if (cycle == 3) {
        cycle = 0;
        gMouse.buttons = packet[0] & 0x07;

        gMouse.x += packet[1];       // delta X
        gMouse.y -= packet[2];       // delta Y (invertido)

        // límites
        if (gMouse.x < 0) gMouse.x = 0;
        if (gMouse.x >= SCREEN_WIDTH) gMouse.x = SCREEN_WIDTH - 1;
        if (gMouse.y < 0) gMouse.y = 0;
        if (gMouse.y >= SCREEN_HEIGHT) gMouse.y = SCREEN_HEIGHT - 1;
    }
}

KernelStatus ErickMain(KernelServices* Services)
{
    InitializeLibrary(Services);

    // inicializar mouse
    mouse_init();

    Services->Display->clearScreen();
    while (1)
    {
        Services->Display->clearScreen();
        Services->Display->setCursorPosition(0,0);
        // procesar datos del mouse (normalmente IRQ12 lo llamaría)
        mouse_handler();

        // mostrar posición y botones
        char buf[32];

        Services->Display->printg("X=");
        IntToString(gMouse.x, buf);
        Services->Display->printg(buf);

        Services->Display->printg(" Y=");
        IntToString(gMouse.y, buf);
        Services->Display->printg(buf);

        Services->Display->printg(" Buttons=");
        IntToString(gMouse.buttons, buf);
        Services->Display->printg(buf);

        Services->Display->printg("\n");

        Services->Display->setCursorPosition(gMouse.x, gMouse.y);

        Services->Display->printg("x");
    }

    return KernelStatusSuccess;
}