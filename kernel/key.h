
#define KEYBUF_SIZE 256

bool AltPressedKey = 0;
bool CtrlPressedKey = 0;

static volatile uint8_t  keypackage_incomming = 0;
static volatile uint32_t scancodeskey_head = 0;
static volatile uint32_t scancodeskey_tail = 0;
static uint8_t keybuf[KEYBUF_SIZE];

static inline int keybuf_empty(void) { return scancodeskey_head == scancodeskey_tail; }
static inline int keybuf_full(void)  { return ((scancodeskey_head + 1) % KEYBUF_SIZE) == scancodeskey_tail; }

void enqueue_key(uint8_t sc) {
    if (!keybuf_full()) {
        keybuf[scancodeskey_head] = sc;
        scancodeskey_head = (scancodeskey_head + 1) % KEYBUF_SIZE;
        keypackage_incomming = 1; // marca evento
    }
}

uint8_t readkey(void) {
    if (keybuf_empty()) return 0;
    uint8_t sc = keybuf[scancodeskey_tail];
    scancodeskey_tail = (scancodeskey_tail + 1) % KEYBUF_SIZE;
    if (keybuf_empty()) keypackage_incomming = 0; // sin datos pendientes
    return sc;
}

void waitforkey(void) {
    while (keypackage_incomming == 0) { asm volatile("pause"); } // espera ligera
}

void keyboard_handler(regs_t* r) {
    uint8_t scancode = inb(0x60);

    // Control presionado
    if (scancode == 0x1D) {
        CtrlPressedKey = 1;
    }
    // Control soltado
    else if (scancode == 0x9D) {
        CtrlPressedKey = 0;
    }
    // Combinaciones con Control
    else if (CtrlPressedKey) {
        if (scancode == 0x2E) {
            // Control+C detectado
            GlobalServices->Display->printg("\n^C");
            r->eip = (uint32_t)ExceptionHandlePtr;
        }
    }

    if (CtrlPressedKey == 0) enqueue_key(scancode);
    outb(0x20, 0x20); // EOI
}