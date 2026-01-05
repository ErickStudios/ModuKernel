#define RTL8139_TX_STATUS0   0x10
#define RTL8139_TX_ADDR0     0x20
#define RTL8139_IOBASE 0xC000  // dirección base de la tarjeta (PCI BAR0)
#define RTL8139_RBSTART      0x30   // Receive Buffer Start Address
#define RTL8139_CAPR         0x38   // Current Address of Packet Read

uint8_t rx_buffer[8192]; // ejemplo pequeño

void rtl8139_init_rx() {
    poutl(RTL8139_IOBASE + RTL8139_RBSTART, (uint32_t)rx_buffer);
    poutb(RTL8139_IOBASE + 0x37, 0x0C); // habilitar RX y TX
}

void rtl8139_send(uint8_t* data, uint32_t len) {
    uint8_t* tx_buffer = (uint8_t*)0xD0000;
    for (uint32_t i = 0; i < len; i++) {
        tx_buffer[i] = data[i];
    }

    // Escribir dirección física del buffer
    poutl(RTL8139_IOBASE + RTL8139_TX_ADDR0, 0xD0000);

    // Escribir longitud → esto dispara el envío
    poutl(RTL8139_IOBASE + RTL8139_TX_STATUS0, len & 0xFFFF);
}

uint32_t current_offset = 0; // puntero dentro del buffer RX

int rtl8139_receive(uint8_t* out_data) {
    // Usa directamente rx_buffer
    uint8_t* rx_buf = rx_buffer + current_offset;

    uint16_t status = *(uint16_t*)(rx_buf);
    uint16_t len    = *(uint16_t*)(rx_buf + 2);

    if (status & 0x01) { // paquete OK
        for (int i = 0; i < len; i++) {
            out_data[i] = rx_buf[4 + i];
        }

        // Avanzar el puntero CAPR (Current Address of Packet Read)
        current_offset = (current_offset + len + 4 + 3) & ~3;
        poutw(RTL8139_IOBASE + 0x38, current_offset - 0x10);

        return len;
    }
    return 0;
}