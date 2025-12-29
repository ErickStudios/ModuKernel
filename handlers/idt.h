#include "idt/entry.h"
#include "idt/pointer.h"

void set_idt_entry(int vector, uint32_t handler, uint16_t selector, uint8_t type_attr) {
    idt[vector].offset_low  = handler & 0xFFFF;
    idt[vector].selector    = selector;
    idt[vector].zero        = 0;
    idt[vector].type_attr   = type_attr;
    idt[vector].offset_high = (handler >> 16) & 0xFFFF;
}