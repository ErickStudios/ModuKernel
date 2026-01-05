#define RTL8139_IOBASE 0xC000  // dirección base de la tarjeta (PCI BAR0)

// Resetear la tarjeta
void rtl8139_reset() {
    poutb(RTL8139_IOBASE + 0x37, 0x10); // escribir en Command Register
    while (pinb(RTL8139_IOBASE + 0x37) & 0x10); // esperar a que se limpie el bit
}

// Leer MAC address
void rtl8139_read_mac(uint8_t* mac) {
    for (int i = 0; i < 6; i++) {
        mac[i] = pinb(RTL8139_IOBASE + i);
    }
}