/* desktop.c, archivo del escritorio de ModuKernel y el entorno de escritorio, esta
es una app para la extension del sistema que proximamente sera una app del sistema
que esta estara incluida cuando se termine */

#include "../library/lib.h"

/* prototipos */

struct _Option;
struct _Menu;
struct _MouseState;
struct _DesktopContext;

/* para salir */
Mini Salir = 0;

/* estructura de opcion */
struct _Option
{
    /* texto de la opcion */
    string Texto;
    /* funcion */
    string Funcion;
};

/* estructura de un menu */
struct _Menu
{
    /* el texto del menu */
    string Texto;
    /* opciones */
    struct _Option* Opciones;
    /* activado */
    Mini Activado;
};

/* estado del mouse */
struct _MouseState
{
    /* posicion x*/
    Entero x;
    /* posicion y */
    Entero y;
    /* botones */
    Mini buttons;
};

/* contexto del escritorio */
struct _DesktopContext
{
    /* el numero de menus */
    Entero MenuNumber;
    /* los menus */
    struct _Menu* Menus;
};

/**
 * ExecuteAction
 * 
 * ejecuta una accion
 * @param action es el script que hace y en lineas con linefeed
*/
void ExecuteAction(char* action)
{
    string Action = gMS->AllocatePool(sizeof(char) * StrLen(action));
    for (Entero i=0; i < (StrLen(action) + 1); i++) Action[i] = action[i];

    string Splited[128];

    Entero Count = StrSplit(Action, Splited, '\n');

    for (Entero i = 0; i < Count; i++)
    {
        string Command = Splited[i];

        if (StrCmp(Command, "exitenv") == 0) Salir = 1;
        gSys->Misc->Run(gSys, Command, 0);
    }

    gMS->FreePool(Action);
}

/**
 * ClickDesktop
 * 
 * clickea en el escritorio
 * @param Desktop es el escritorio
 * @param Mouse es el mouse
*/
void ClickDesktop(struct _DesktopContext* Desktop, struct _MouseState* Mouse)
{
    gDS->setCursorPosition(0,0);

    for (Entero i = 0; i < Desktop->MenuNumber; i++)
    {
        struct _Menu Menu = Desktop->Menus[i];

        char* Text = Menu.Texto;

        if (
            (((Mouse->x) > (gDS->CurrentCharacter * 5)) && 
            ((Mouse->x) < ((gDS->CurrentCharacter * 5) + (StrLen(Text) * 5)))) &&
            ((Mouse->y > (gDS->CurrentLine * 8)) && Mouse->y < ((gDS->CurrentLine+1) * 8))
        )
            Desktop->Menus[i].Activado = !Desktop->Menus[i].Activado;
        
        Entero PosX = gDS->CurrentCharacter;

        gDS->CurrentCharacter += StrLen(Text);
        Entero PosXa = gDS->CurrentCharacter;

        if (Menu.Activado)
        {
            Entero Indice = 0;
            Entero OldLine = gDS->CurrentLine;
            struct _Option Opcion = Menu.Opciones[Indice++];

            gDS->CurrentLine++;

            gDS->CurrentCharacter = PosX;

            while (Opcion.Texto[0] != 0)
            {
                char* TextA = Opcion.Texto;

                if (
                    (((Mouse->x) > (gDS->CurrentCharacter * 5)) && 
                    ((Mouse->x) < ((gDS->CurrentCharacter * 5) + (StrLen(TextA) * 5)))) &&
                    ((Mouse->y > (gDS->CurrentLine * 8)) && Mouse->y < ((gDS->CurrentLine+1) * 8))
                )
                    ExecuteAction(Opcion.Funcion);
                
                gDS->CurrentCharacter += StrLen(TextA);
                gDS->CurrentCharacter = PosX;
                gDS->CurrentLine++;
                Opcion = Menu.Opciones[Indice++];
            }

            gDS->CurrentCharacter = PosXa;
            gDS->CurrentLine = OldLine;
        }
    }
}

/** 
 * DrawDesktop
 * 
 * dibuja el escritorio
 * @param Desktop el escritorio
 * @param Mouse el mouse
*/
void DrawDesktop(struct _DesktopContext* Desktop, struct _MouseState* Mouse)
{
    gDS->setCursorPosition(0,0);

    for (Entero i = 0; i < Desktop->MenuNumber; i++)
    {
        struct _Menu Menu = Desktop->Menus[i];

        char* Text = Menu.Texto;

        gDS->setAttrs(0x7, 0x0);

        if (
            (((Mouse->x) > (gDS->CurrentCharacter * 5)) && 
            ((Mouse->x) < ((gDS->CurrentCharacter * 5) + (StrLen(Text) * 5)))) &&
            ((Mouse->y > (gDS->CurrentLine * 8)) && Mouse->y < ((gDS->CurrentLine+1) * 8))
        )
            gDS->setAttrs(0x0, 0x7);
        
        Entero PosX = gDS->CurrentCharacter;

        gDS->printg(Text);
        Entero PosXa = gDS->CurrentCharacter;

        if (Menu.Activado)
        {
            Entero Indice = 0;
            Entero OldLine = gDS->CurrentLine;
            struct _Option Opcion = Menu.Opciones[Indice++];

            gDS->CurrentLine++;

            gDS->CurrentCharacter = PosX;

            while (Opcion.Texto[0] != 0)
            {
                char* TextA = Opcion.Texto;

                gDS->setAttrs(0x7, 0x0);

                if (
                    (((Mouse->x) > (gDS->CurrentCharacter * 5)) && 
                    ((Mouse->x) < ((gDS->CurrentCharacter * 5) + (StrLen(TextA) * 5)))) &&
                    ((Mouse->y > (gDS->CurrentLine * 8)) && Mouse->y < ((gDS->CurrentLine+1) * 8))
                )
                    gDS->setAttrs(0x0, 0x7);
                
                gDS->printg(TextA);
                gDS->CurrentCharacter = PosX;
                gDS->CurrentLine++;
                Opcion = Menu.Opciones[Indice++];
            }

            gDS->CurrentCharacter = PosXa;
            gDS->CurrentLine = OldLine;
        }
    }
    
}

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

typedef struct _MouseState MouseState;

MouseState gMouse;
#define SCREEN_WIDTH  160   // columnas
#define SCREEN_HEIGHT 80   // filas

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
    // incializar libreria
    InitializeLibrary(Services);
    
    // inicializar mouse
    mouse_init();

    // inicializar display avanzado
    if (*Services->Display->IsInPixelsMode == 0) Services->Display->ActivatePixel();

    // mouse antiguo
    MouseState OldMouse;

    mouse_handler();
    OldMouse.x = gMouse.x;
    OldMouse.y = gMouse.y;
    OldMouse.buttons = gMouse.buttons;

    Services->Display->DrawBackgroundColor(0);
    uint8_t ColorInPos = Services->Display->GetPixel(OldMouse.x, OldMouse.y);

    struct _DesktopContext Context = {
        .MenuNumber = 1,
        .Menus = Services->Memory->AllocatePool(sizeof(struct _Menu) * 1)
    };

    Context.Menus[0].Activado = 0;
    Context.Menus[0].Texto = "Menu";

    Context.Menus[0].Opciones = Services->Memory->AllocatePool(sizeof(struct _Option) * 4);
    Context.Menus[0].Opciones[0].Texto = "Shell";
    Context.Menus[0].Opciones[0].Funcion = "cls\nshell";
    Context.Menus[0].Opciones[1].Texto = "Exit";
    Context.Menus[0].Opciones[1].Funcion = "cls\nexitenv";
    Context.Menus[0].Opciones[2].Texto = "Execute";
    Context.Menus[0].Opciones[3].Texto[0] = 0;

    while (1)
    {
        if (Salir == 1) return KernelStatusSuccess;

        mouse_handler();
        
        Mini ActualizarMouse = 0;

        if (!(OldMouse.x == gMouse.x &&
            OldMouse.y == gMouse.y &&
            OldMouse.buttons == gMouse.buttons))
        ActualizarMouse = 1;

        if (ActualizarMouse == 1)
        {
            Services->Display->DrawRectangle(ColorInPos, OldMouse.x, OldMouse.y, 1);

            OldMouse.x = gMouse.x;
            OldMouse.y = gMouse.y;

            ColorInPos = Services->Display->GetPixel(OldMouse.x, OldMouse.y);
        }

        DrawDesktop(&Context, &OldMouse);

        if (ActualizarMouse == 1 && OldMouse.buttons != gMouse.buttons)
        {
            OldMouse.buttons = gMouse.buttons;
            if (gMouse.buttons != 0)
            {
                ClickDesktop(&Context, &OldMouse);
                Services->Display->DrawBackgroundColor(0);
            }
        }

        Services->Display->DrawRectangle(15 ,OldMouse.x, OldMouse.y, 1);
    }

    return KernelStatusSuccess;
}