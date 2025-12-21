; kernel.asm
;
; sirve para etapas de arranque pre-prematuras y cosas que se hacen
; en Assembly... oye y en C?... nop no se puede

; es un kernel de 32 bits
bits 32

; multiboot
section .multiboot
	align 4
	dd 0x1BADB002			;magic
	dd 0x00				;flags
	dd - (0x1BADB002 + 0x00)	;checksum. m+f+c should be zero

; seccion de datos
section .data
	; mensaje auxilear de que ha fallado
	msg db 10,'El kernel ha fallado y se ha regresado al entorno pre-C, reinicie la pc para continuar',10,0

; seccion de texto y datos
section .text

; globalizar start
global start

; funciones
extern InternalPrintgNonLine		; funcion de impresion
extern k_main						; funcion principal

; principal
start:

	; etapa pre-prematura, aqui se configura todo y las tareas de bajo nivel
	; que no se pueden hacer en C se hacen

	; desactivar interrupciones
    cli

	; preparar al kernel para la etapa post-prematura y el arranque C prematuro
    call k_main

	; aqui si algo salio mal en el arranque aqui se hara un codigo de emergencia
	; que limpiara la pila y congelara el kernel

	; el mensaje
    push msg
	; mandar a imprimir
	call InternalPrintgNonLine
	; no hay nada
    add esp, 4

	; si algo sale mal congelar
	hlt