
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
    mov ecx, 640*400		; el tamaño del dibujado
    rep stosb				; repetirlo

    ret

