; kernel.asm
;
; sirve para etapas de arranque pre-prematuras y cosas que se hacen
; en Assembly... oye y en C?... nop no se puede

; es un kernel de 32 bits
bits 32

section .note.GNU-stack

; multiboot
section .multiboot
	; header multiboot
	align 4						; alineacion para multiboot
	dd 0x1BADB002				; codigo magico para indicar que es multiboot
	dd 0x0						; parametros de cargador de arranque
	dd - (0x1BADB002 + 0x00)	; checksum. m+f+c debe ser 0

; seccion de datos
section .data
	; variables
	%include "assembly/dat/boot.asm"; arranque
	%include "assembly/dat/gop.asm"; graficos
	%include "assembly/dat/level.asm"; niveles

; seccion de texto y datos
section .text

; variables
extern gdt_ptr					; puntero al gdt

; exportaciones
%include "assembly/exports/boot.asm"; arranque
%include "assembly/exports/gop.asm";graficos
%include "assembly/exports/misc.asm";varios
%include "assembly/exports/serial.asm"; serial

; incluir archivos de las funciones de assembly
%include "assembly/boot.asm" 	; arranque
%include "assembly/hlt.asm"		; hlt
%include "assembly/gop.asm"		; graficos
%include "assembly/serial.asm" 	; consola serial
%include "assembly/exceptions.asm"; excepciones

; seccion de datos sin inicializar
section .bss
	STACK_SPACE: resb 128000 ; 128 KB OF STACK