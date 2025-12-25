#ifndef _ServicesDiskFloppyDotH_
#define _ServicesDiskFloppyDotH_
// Puertos FDC y DMA (simplificados)
#define FDC_DOR   0x3F2
#define FDC_MSR   0x3F4
#define FDC_DSR   0x3F4  // write
#define FDC_DATA  0x3F5

// DMA canal 2 (floppy) — controladora 8237
#define DMA_MASK      0x0A
#define DMA_MODE      0x0B
#define DMA_CLEARFF   0x0C
#define DMA_CH2_ADDR  0x04
#define DMA_CH2_COUNT 0x05
#define DMA_CH2_PAGE  0x81

// Helpers de espera de RQM/DIO en FDC
static inline void fdc_write(uint8_t b) {
    // Espera a RQM=1 y DIO=0 (listo para aceptar comandos)
    while (!(inb(FDC_MSR) & 0x80)) { /* RQM */ }
    outb(FDC_DATA, b);
}
static inline uint8_t fdc_read(void) {
    // Espera a RQM=1 y DIO=1 (datos disponibles)
    while (!((inb(FDC_MSR) & 0x90) == 0x90)) { /* RQM+DIO */ }
    return inb(FDC_DATA);
}

#endif