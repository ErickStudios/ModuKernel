/**
* ABI/user_link.ld
*
* archivo de linkeo para ErickABI+NeffiTo
*/

/* punto de entrada= ErickMain */
ENTRY(ErickMain)

SECTIONS {
  /* Dirección fija donde se ejecutará el programa */
  . = 0x00F00000;

  /* Encabezado (16 bytes) al inicio de la imagen */
  .header : {
    BYTE(0x4D)  /* M */
    BYTE(0x6F)  /* o */
    BYTE(0x64)  /* d */
    BYTE(0x75)  /* u */
    BYTE(0x42)  /* B */
    BYTE(0x69)  /* i */
    BYTE(0x6E)  /* n */
    BYTE(0x00)  /* terminador opcional */
    LONG(ErickMain)       /* entry address */
    LONG(__bss_start)     /* bss start address */
    LONG(__bss_end)       /* bss end address */
  }

  /* Código y constantes */
  .text : { *(.text*) *(.rodata*) }

  /* Datos inicializados */
  .data : { *(.data*) }

  /* Datos sin inicializar */
  .bss : {
    __bss_start = .;
    *(.bss*) *(COMMON)
    __bss_end = .;
  }

  /* Fin opcional de imagen */
  __image_end = .;
}