/*
 * link.ld
 */

OUTPUT_FORMAT(elf32-i386)
ENTRY(start)
SECTIONS
{
	. = 0x100000;
	.text : {*(.text)}
	.data : {*(.data)}
	.bss  : {*(.bss)}

    /* Definir el heap después de las secciones */
    _heap_start = .;
    . += 0x100000;       /* tamaño del heap: 1 MiB, ajusta según necesites */
    _heap_end = .;
}
