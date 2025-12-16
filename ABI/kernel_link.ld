/*
 * link.ld
 */

/* formato */
OUTPUT_FORMAT(elf32-i386)

/* inicio */
ENTRY(start)

/* secciones */
SECTIONS
{
    /* actual */
	. = 0x100000;

    /* codigo */
	.text : {*(.text)}
    /* datos */
	.data : {*(.data)}
    /* sin inicializar*/
	.bss  : {
    __bss_start = .;      /* marca inicio de bss */
    *(.bss*) *(COMMON)    /* coloca todas las variables sin inicializar */
    __bss_end = .;        /* marca fin de bss */
    }

    /* para AllocatePool y Servicios.Memory->AllocatePool*/
    _heap_start = .;
    /* fin del heap */
    . += 0xF00000;
    /* fin */
    _heap_end = .;
}
