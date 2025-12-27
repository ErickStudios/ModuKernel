; kernel.asm
;
; sirve para etapas de arranque pre-prematuras y cosas que se hacen
; en Assembly... oye y en C?... nop no se puede

; es un kernel de 32 bits
bits 32

; multiboot
section .multiboot
	; header multiboot
	align 4						; alineacion para multiboot
	dd 0x1BADB002				; codigo magico para indicar que es multiboot
	dd 0x0						; parametros de cargador de arranque
	dd - (0x1BADB002 + 0x00)	; checksum. m+f+c debe ser 0

; seccion de datos
section .data
	; mensaje auxilear de que ha fallado
	msg db 10,'El kernel ha fallado y se ha regresado al entorno pre-C, reinicie la pc para continuar',10,0
	
	; datos de grafico
	InternalGrapichalFlag db 0	; bandera grafica

; seccion de texto y datos
section .text

; funciones de assembly estandart
global start					; funcion de inicio
global InternalKernelHaltReal	; funcion de detencion

; funciones de configuracion y desconfiguracion de modos
global config_mode				; funcion que configura el modo grafico
global unconfig_mode			; funcion que sale del modo grafico pero no funciona, solo para buggearlo y terminar de configurarlo

; funciones del display y display avanzado
global InternalDrawBackground	; funcion para dibujar un fondo de un color
global InternalDrawPixel		; funcion para dibujar un pixel o varios
global InternalGopScreenInit	; funcion para inicializar la pantalla en modo grafico
global InternalGrapichalFlag	; bandera de graficos si esta en modo grafico

; funciones de la consola serial
global init_serial				; inicializa la consola serial
global InternalSendCharToSerial	; funcion para enviar caracteres a la consola serial

; funciones
extern InternalPrintgNonLine	; funcion de impresion
extern k_main					; funcion principal
extern init_gdt

; variables
extern gdt_ptr

; principal
start:

	; etapa pre-prematura, aqui se configura todo y las tareas de bajo nivel
	; que no se pueden hacer en C se hacen

	; funciones de estandart
    cli							; desactivar interrupciones

	; inicializar gdt
	call init_gdt				; funciones de gdt
	lgdt [gdt_ptr]       		; carga la GDT definida en C
	
	; recargas
	jmp 0x08:flush_cs    		; far jump para recargar CS
	ret							; que diablos tiene que pasar para que pase eso?

; esta funcion hace que cs se flushee
flush_cs:

	; inicializar AX
    mov ax, 0x10				; valor

	; inicializar valores
    mov ds, ax					; ax->ds
    mov es, ax					; ax->es
    mov fs, ax					; ax->fs
    mov gs, ax					; ax->gs
    mov ss, ax					; ax->ss

	; funciones de kernel y inicializar
	call init_serial			; iniciar serial
    call k_main					; preparar al kernel para la etapa post-prematura y el arranque C prematuro

	; aqui si algo salio mal en el arranque aqui se hara un codigo de emergencia
	; que limpiara la pila y congelara el kernel

	; detencion personalizada
    push msg					; el mensaje
	call InternalPrintgNonLine	; mandar a imprimir
    add esp, 4					; no hay nada

	; detencion estandart
	call InternalKernelHaltReal	; si algo sale mal congelar
	ret							; por que pasara esto, bueno esta por si acaso

; halt detiene el procesador para hacer cosas que ya no estan
; y por ejemplo se puede detener
InternalKernelHaltReal:

	; instrucciones de por medio
	nop							; hacer nada

	; llamar
	jmp InternalKernelHaltReal	; llamar a halt

	ret

; configura el modo grafico, esto no funciona por si solo, se necesita
; salir entrar salir entrar y mas secuncias raras que parece que todo
; se hace para entrar y salir repetidamente
config_mode:
	; 28MHz habilita RAM, segmento de pantalla A0000
	mov dx, 0x3C2				; puerto
	mov al, 0x63				; dato
	out dx, al					; mandarlo

	; macro para repeticion de vga secuencer
	%macro CfgVgaSecu 2
		; configurar registro 1
		mov dx, 0x3C4 			; puerto
		mov al, %1 				; dato
		out dx, al 				; enviarlo

		; configurar registro 2
		mov dx, 0x3C5			; puerto
		mov al, %2           	; dato
		out dx, al 				; enviarlo
	%endmacro

	; configurar el vga secuencer y hacer la secuencia
	; para configurar el modo grafico
	CfgVgaSecu 0x00, 0x03		; restablezer para estar listo

	CfgVgaSecu 0x01, 0x01		; clocking mode
	CfgVgaSecu 0x02, 0x0F		; map mask

	CfgVgaSecu 0x03, 0x00		; character map select
	CfgVgaSecu 0x04, 0x06		; seleccionar MemoryMode y activar chain-4

	; configurar CRTC para desbloquear 0x11 y hacer
	; modo grafico

	; configurar registro 1
	mov dx, 0x3D4				; puerto
	mov al, 0x11				; dato
	out dx, al					; mandarlo

	; leer registro 2
	mov dx, 0x3D5				; puerto
	in  al, dx					; leerlo

	; hacer operaciones
	and al, 0x7F				; operar con and
	out dx, al					; mandarlo

	; --- CRTC timings modo 13h ---
	; idx in 0x3D4, data in 0x3D5
	%macro AjustarCrtc 2
		; configurar registro 1
		mov dx, 0x3D4 			; puerto
		mov al, %1				; dato
		out dx, al				; enviarlo

		; configurar registro 2
		mov dx, 0x3D5			; puerto
		mov al, %2				; dato
		out dx, al				; enviarlo
	%endmacro

	; ajustar el Crtc para mas secuencia de comandos
	; al 3D4h y 3D5h

	; display horizontal

	; retrazo horizontal
	AjustarCrtc 0x00, 0x9F		; total de horizontal

	AjustarCrtc 0x04, 0x52		; inicio del retrazo de horizontal
	AjustarCrtc 0x05, 0x56		; fin del retrazo de horizontal
	AjustarCrtc 0x01, 0x8F		; fin del display horizontal

	; blanco horizontal
	AjustarCrtc 0x02, 0x50		; inicio del blanco horizontal
	AjustarCrtc 0x03, 0x24		; fin del blanco horizontal

	; donde se configura la parte vertical de la pantalla

	AjustarCrtc 0x06, 0xCF		; total del vertical

	; overflow
	AjustarCrtc 0x07, 0x1F
	AjustarCrtc 0x08, 0x00
	AjustarCrtc 0x09, 0x41
	AjustarCrtc 0x0A, 0x00
	AjustarCrtc 0x0B, 0x00
	AjustarCrtc 0x0C, 0x00
	AjustarCrtc 0x0D, 0x00
	AjustarCrtc 0x0E, 0x00
	AjustarCrtc 0x0F, 0x00

	; retrazo vertical
	AjustarCrtc 0x10, 0x10C		; inicio del retrazo vertical
	AjustarCrtc 0x11, 0xFE		; fin del retrazo vertical
	AjustarCrtc 0x12, 0xFF		; fin del display vertical

	; offset
	AjustarCrtc 0x13, 0x50

	; ubicacion de underline
	AjustarCrtc 0x14, 0x80

	; blanco vertical

	AjustarCrtc 0x15, 0x96		; inicio del blanco vertical
	AjustarCrtc 0x16, 0xB9		; fin del blanco vertical

	; control del modo del CRTC
	AjustarCrtc 0x17, 0xA3

	; comparador de linea
	AjustarCrtc 0x18, 0xFF

	; --- Graphics Controller ---
	%macro set_gc 2
		mov dx, 0x3CE
		mov al, %1
		out dx, al
		mov dx, 0x3CF
		mov al, %2
		out dx, al
	%endmacro

	; secuencia 2
	set_gc 0x00, 0x00       ; Set/Reset
	set_gc 0x01, 0x00       ; Enable Set/Reset
	set_gc 0x02, 0x00       ; Color Compare
	set_gc 0x03, 0x00       ; Data Rotate
	set_gc 0x04, 0x00       ; Read Map Select
	set_gc 0x05, 0x40       ; Mode: write mode 0
	set_gc 0x06, 0x05       ; Misc: habilitar A0000, gráfico
	set_gc 0x07, 0x0F       ; Color Don't Care
	set_gc 0x08, 0xFF       ; Bit Mask

	; --- Attribute Controller ---
	; cada par necesita leer 0x3DA antes para resetear flip-flop
	%macro set_ac 2
		mov dx, 0x3DA
		in  al, dx
		mov dx, 0x3C0
		mov al, %1
		out dx, al
		mov al, %2
		out dx, al
	%endmacro

	; secuencia 3
	set_ac 0x10, 0x41       ; Mode Control (gráfico, blink off)
	set_ac 0x12, 0x0F       ; Color Plane Enable (todas)
	set_ac 0x13, 0x00       ; Horizontal PEL Panning
	set_ac 0x14, 0x00       ; Color Select

	; Unblank: habilita salida de video

	; obtener unblank
	mov dx, 0x3DA			; puerto
	in  al, dx				; recibir

	; configurar registro
	mov dx, 0x3C0			; puerto
	mov al, 0x20			; dato
	out dx, al				; mandar

	ret

; desconfiguar el modo grafico y vuelve al modo texto, bueno, se
; supone que debe hacer eso pero esta tan buggeado que solo sierve
; para ayudar con mas bugs a que el modo grafico funcione
unconfig_mode:
	mov byte [InternalGrapichalFlag], 0x01

	; --- Misc Output ---
	mov dx, 0x3C2			; puerto
	mov al, 0x67        	; dato: reloj 25 MHz, habilitar RAM, segmento B8000
	out dx, al				; mandar

	%macro SetSeq2 2
		; configurar registro 1
		mov dx, 0x3C4		; puerto
		mov al, %1			; dato
		out dx, al			; mandar

		; configurar registro 2
		mov dx, 0x3C5		; puerto
		mov al, %2			; dato
		out dx, al			; mandar
	%endmacro

	; --- Sequencer ---
	SetSeq2 0x00, 0x03		; mandar 0x00 y 0x03
	SetSeq2 0x01, 0x00		; clocking mode texto
	SetSeq2 0x02, 0x03		; planes 0 y 1
	SetSeq2 0x04, 0x02		; modo texto

	; configurar el Grapichs controller para poder inicializar el
	; modo texto

	; configurar registro 1
	mov dx, 0x3CE			; puerto
	mov al, 0x06			; dato
	out dx, al				; mandar

	; configurar registro 2
	mov dx, 0x3CF			; puerto
	mov al, 0x0A        	; dato: memoria B8000, modo texto
	out dx, al				; mandar

	; --- Attribute Controller ---
	mov dx, 0x3DA
	in  al, dx
	mov dx, 0x3C0
	mov al, 0x10
	out dx, al
	mov al, 0x0C        ; modo texto
	out dx, al

	; Unblank
	mov dx, 0x3DA
	in  al, dx
	mov dx, 0x3C0
	mov al, 0x20
	out dx, al

	ret

; Inicializa la pantalla con bugs con las funciones de inicializacion por
; que no se puede con una y para despertar el display completamente
;
; esta secuencia se probo primero con comandos y se implemento la secuencia interna
; despues
InternalGopScreenInit:
	; configuracion
	call config_mode		; modo de configuracion
	call screen_draw_init	; dibujar negro para inicializar

	; recargarlo
	call unconfig_mode		; desconfigurar modo
	call config_mode		; configurarlo

	; dibujar el fondo negro
	call screen_draw_init	; dibujar fondo negro
	call config_mode		; configurar para dibujar por fin

	; activar bandera de flag
	mov byte [InternalGrapichalFlag], 0x01

	; retornar
	ret

; funcion para dibujar fondo de inicializacion, que este
; hace que la pantalla se inicialize con negro para no dejarlo
; raro
screen_draw_init:
	; dibujar pantalla negra
    mov edi, 0x000A0000		; el display
    mov ecx, 320*200		; tamaño de la pantalla
    mov al, 0				; el color negro
    rep stosb				; repetirlo

	; mover valores 10 al registro
    mov eax, 10
    mov ebx, 10

	; operaciones para hacer x
    mov edx, 320			; mover el tamaño en x de la pantalla
    imul ebx, edx			; multiplicarlo
    add ebx, eax			; sumarle

	; añadir la direccion de la pantalla y sumarle
    mov edi, 0x000A0000		; direccion
    add edi, ebx			; dato

	; mover el byte edi a 15
    mov byte [edi], 15		; mover
    ret

; dibujar un fondo de un color especifico para la pantalla
InternalDrawBackground:

	; guardar registros para no perderlos y luego poder 
	; restaurarlos

	; parametros
    mov eax, [esp+4]		; el color
    mov al, al				; mover al a al

	; dibujar
    mov edi, 0xA0000		; la direccion de la pantalla
    mov ecx, 320*200		; el tamaño del dibujado
    rep stosb				; repetirlo

    ret

; funcion para inicializar el COM1 de la consola serial
; para poder mandar cosas alli
init_serial:
	%macro ComSerialSet 2
		mov dx, 0x3F8+%1
		mov al, %2
		out dx, al
	%endmacro

	; configurar
	ComSerialSet 1, 0x00	; Interrupt Enable Register
	ComSerialSet 3, 0x80	; Line Control Register y habilitar acceso a divisor
	ComSerialSet 0, 0x01	; Divisor Low (115200 baud)
	ComSerialSet 1, 0x00	; Divisor High
	ComSerialSet 3, 0x03	; Line Control Register y 8 bits, sin paridad, 1 stop
	ComSerialSet 2, 0xC7	; FIFO Control y habilitar FIFO
	ComSerialSet 4, 0x0B	; Modem Control y RTS/DSR

	ret

; enviar un caracter al puerto serial
InternalSendCharToSerial:
	; guardarlo para despues recuperarlo
	push eax				; el argumento
	push dx					; el registro

    ; obtener argumento desde la pila
    mov eax, [esp+4]   		; primer argumento

	; guardar al
	movzx eax, al			; extension
	push eax				; guardar

    ; espera a que el transmisor esté listo
    mov dx, 0x3F8+5   	 	; Line Status Register

	; si esta listo?
	.waita:
    	in al, dx			; leerlo
    	test al, 0x20		; probarlo
    	jz .waita			; si no no se puede

    ; enviar carácter
    mov dx, 0x3F8			; registro de serial
	pop eax        			; recupera 32 bits
	mov al, al     			; ya está en la parte baja de eax
    out dx, al				; mandarlo

	pop dx					; el registro
	pop eax					; el argumento

    ret

section .bss
	STACK_SPACE: resb 128000 ; 128 KB OF STACK