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

; seccion de texto y datos
section .text

; globalizar start
global start
; poner el principio del kernel
extern k_main	

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

	; si algo sale mal congelar
	hlt