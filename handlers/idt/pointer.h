struct IDTPointer {
    uint16_t limit; // tamaño de la tabla - 1
    uint32_t base;  // dirección base de la tabla
} __attribute__((packed));

struct IDTEntry idt[256];
struct IDTPointer idt_ptr;