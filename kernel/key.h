
#define KEYBUF_SIZE 256

bool AltPressedKey = 0;
bool CtrlPressedKey = 0;

static volatile uint8_t  keypackage_incomming = 0;
static volatile uint32_t scancodeskey_head = 0;
static volatile uint32_t scancodeskey_tail = 0;
static uint8_t keybuf[KEYBUF_SIZE];

int keybuf_empty(void) { 
    // si esta vacio
    return scancodeskey_head == scancodeskey_tail;
 }
 
int keybuf_full(void)  { 
    // si esta lleno
    return ((scancodeskey_head + 1) % KEYBUF_SIZE) == scancodeskey_tail; 
}

void enqueue_key(uint8_t sc) {
    // si el teclado no esta lleno
    if (!keybuf_full()) {
        // la tecla
        keybuf[scancodeskey_head] = sc;
        // el head
        scancodeskey_head = (scancodeskey_head + 1) % KEYBUF_SIZE;
        // tecla en camino
        keypackage_incomming = 1;
    }
}

uint8_t readkey(void) {
    // si esta lleno, retornar null
    if (keybuf_empty()) return 0;

    // scaneo
    uint8_t sc = keybuf[scancodeskey_tail];

    // cola
    scancodeskey_tail = (scancodeskey_tail + 1) % KEYBUF_SIZE;

    // ya se atrapo
    if (keybuf_empty()) keypackage_incomming = 0;
    return sc;
}

void waitforkey(void) {
    while (keypackage_incomming == 0) { asm volatile("pause"); } // espera ligera
}

bool ControlCCloseActive = 0;

void keyboard_handler(regs_t* r) {
    // leer tecla
    uint8_t scancode = inb(0x60);

    // Control presionado
    if (scancode == 0x1D) CtrlPressedKey = 1;
    // Control soltado
    else if (scancode == 0x9D) CtrlPressedKey = 0;
    // Combinaciones con Control
    else if (CtrlPressedKey) {
        // Control^C
        if (scancode == 0x2E) {
            // poner el atajo puesto
            GlobalServices->Display->printg("^C");
            // para cerrar
            ControlCCloseActive = 1;
            // handlear el control
            r->eip = (uint32_t)ExceptionHandlePtr;
        }
    }

    // si no es control, empaquetar tecla
    if (CtrlPressedKey == 0) enqueue_key(scancode);

    // hacer outb
    outb(0x20, 0x20);
}