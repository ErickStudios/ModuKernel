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
	dd 0x0				;flags
	dd - (0x1BADB002 + 0x00)	;checksum. m+f+c should be zero

; seccion de datos
section .data
	; mensaje auxilear de que ha fallado
	msg db 10,'El kernel ha fallado y se ha regresado al entorno pre-C, reinicie la pc para continuar',10,0
	InternalGrapichalFlag db 0

; seccion de texto y datos
section .text

; globalizar start
global start
global config_mode
global unconfig_mode
global draw_bg
global InternalDrawPixel
global init_serial
global InternalSendCharToSerial
global InternalGrapichalFlag

; funciones
extern InternalPrintgNonLine		; funcion de impresion
extern k_main						; funcion principal

; principal
start:

	; etapa pre-prematura, aqui se configura todo y las tareas de bajo nivel
	; que no se pueden hacer en C se hacen

	; desactivar interrupciones
    cli

	; iniciar serial
	call init_serial

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

global InternalDrawPixel
global InternalGopScreenInit

config_mode:
	; --- Misc Output (elige reloj y segmento A0000) ---
	mov dx, 0x3C2
	mov al, 0x63            ; 28MHz, habilitar RAM, segmento A0000
	out dx, al

	; --- Sequencer ---
	mov dx, 0x3C4
	mov al, 0x00            ; Reset
	out dx, al
	mov dx, 0x3C5
	mov al, 0x03            ; terminar reset
	out dx, al

	mov dx, 0x3C4
	mov al, 0x01            ; Clocking Mode
	out dx, al
	mov dx, 0x3C5
	mov al, 0x01
	out dx, al

	mov dx, 0x3C4
	mov al, 0x02            ; Map Mask
	out dx, al
	mov dx, 0x3C5
	mov al, 0x0F
	out dx, al

	mov dx, 0x3C4
	mov al, 0x03            ; Character Map Select
	out dx, al
	mov dx, 0x3C5
	mov al, 0x00
	out dx, al

	mov dx, 0x3C4
	mov al, 0x04            ; Memory Mode
	out dx, al
	mov dx, 0x3C5
	mov al, 0x06            ; chain-4
	out dx, al

	; --- CRTC (desbloquear 0x11) ---
	mov dx, 0x3D4
	mov al, 0x11
	out dx, al
	mov dx, 0x3D5
	in  al, dx
	and al, 0x7F
	out dx, al

	; --- CRTC timings modo 13h ---
	; idx in 0x3D4, data in 0x3D5
	%macro set_crtc 2
		mov dx, 0x3D4
		mov al, %1
		out dx, al
		mov dx, 0x3D5
		mov al, %2
		out dx, al
	%endmacro

	set_crtc 0x00, 0x5F
	set_crtc 0x01, 0x4F
	set_crtc 0x02, 0x50
	set_crtc 0x03, 0x82
	set_crtc 0x04, 0x54
	set_crtc 0x05, 0x80
	set_crtc 0x06, 0xBF
	set_crtc 0x07, 0x1F
	set_crtc 0x08, 0x00
	set_crtc 0x09, 0x41
	set_crtc 0x0A, 0x00
	set_crtc 0x0B, 0x00
	set_crtc 0x0C, 0x00
	set_crtc 0x0D, 0x00
	set_crtc 0x0E, 0x00
	set_crtc 0x0F, 0x00
	set_crtc 0x10, 0x9C
	set_crtc 0x11, 0x8E
	set_crtc 0x12, 0x8F
	set_crtc 0x13, 0x28     ; bytes por línea / 2 → 0x28 → 320 bytes por línea
	set_crtc 0x14, 0x40
	set_crtc 0x15, 0x96
	set_crtc 0x16, 0xB9
	set_crtc 0x17, 0xA3
	set_crtc 0x18, 0xFF

	; --- Graphics Controller ---
	%macro set_gc 2
		mov dx, 0x3CE
		mov al, %1
		out dx, al
		mov dx, 0x3CF
		mov al, %2
		out dx, al
	%endmacro

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

	set_ac 0x10, 0x41       ; Mode Control (gráfico, blink off)
	set_ac 0x12, 0x0F       ; Color Plane Enable (todas)
	set_ac 0x13, 0x00       ; Horizontal PEL Panning
	set_ac 0x14, 0x00       ; Color Select

	; Unblank: habilita salida de video
	mov dx, 0x3DA
	in  al, dx
	mov dx, 0x3C0
	mov al, 0x20
	out dx, al

	ret

unconfig_mode:
	; --- Misc Output ---
	mov dx, 0x3C2
	mov al, 0x67        ; reloj 25 MHz, habilitar RAM, segmento B8000
	out dx, al

	; --- Sequencer ---
	mov dx, 0x3C4
	mov al, 0x00
	out dx, al
	mov dx, 0x3C5
	mov al, 0x03
	out dx, al

	mov dx, 0x3C4
	mov al, 0x01
	out dx, al
	mov dx, 0x3C5
	mov al, 0x00        ; clocking mode texto
	out dx, al

	mov dx, 0x3C4
	mov al, 0x02
	out dx, al
	mov dx, 0x3C5
	mov al, 0x03        ; planes 0 y 1
	out dx, al

	mov dx, 0x3C4
	mov al, 0x04
	out dx, al
	mov dx, 0x3C5
	mov al, 0x02        ; modo texto
	out dx, al

	; --- Graphics Controller ---
	mov dx, 0x3CE
	mov al, 0x06
	out dx, al
	mov dx, 0x3CF
	mov al, 0x0A        ; memoria B8000, modo texto
	out dx, al

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

InternalGopScreenInit:
	mov byte [InternalGrapichalFlag], 0x01

	call config_mode
	call screen_draw_init
	call unconfig_mode
	call config_mode
	call screen_draw_init
	call config_mode

	; retornar
	ret

screen_draw_init:
    ; llenar pantalla con rojo (color 4)
    mov edi, 0x000A0000
    mov ecx, 320*200
    mov eax, [esp+4]   ; primer argumento (uint8_t promovido a int)
    mov al, 0         ; AL contiene el color
    rep stosb

    ; dibujar un píxel blanco en (10,10)
    mov eax, 10             ; x
    mov ebx, 10             ; y
    mov edx, 320
    imul ebx, edx           ; y*320
    add ebx, eax
    mov edi, 0x000A0000
    add edi, ebx
    mov byte [edi], 15
    ret

draw_bg:
    ; obtener argumento Color desde la pila
    mov eax, [esp+4]   ; primer argumento
    mov al, al         ; AL = color

    ; llenar pantalla con ese color
    mov edi, 0xA0000
    mov ecx, 320*200
    rep stosb

    ret

InternalDrawPixel:

    ; salvar registros callee-saved
    push ebx
    push esi
    push edi
    push ebp

    ; argumentos: color, x, y, size
    mov eax, [esp+4 + 16]     ; color (ajustado por 4 pushes = 16 bytes)
    mov bl, al
    mov eax, [esp+8 + 16]     ; x
    mov esi, eax
    mov eax, [esp+12 + 16]    ; y
    mov edi, eax
    mov eax, [esp+16 + 16]    ; size
    mov ecx, eax

    ; calcular offset = y*320 + x
    mov eax, edi        ; y
    imul eax, 80       ; cada línea = 320 bytes
    add eax, esi        ; + x
    add eax, 0xA0000    ; base de video
    mov edi, eax
    mov al, bl
    rep stosb

    ; restaurar registros
    pop ebp
    pop edi
    pop esi
    pop ebx
    ret

init_serial:
	; Inicializar COM1
	mov dx, 0x3F8+1      ; Interrupt Enable Register
	mov al, 0x00
	out dx, al

	mov dx, 0x3F8+3      ; Line Control Register
	mov al, 0x80         ; habilitar acceso a divisor
	out dx, al

	mov dx, 0x3F8+0      ; Divisor Low (115200 baud)
	mov al, 0x01
	out dx, al

	mov dx, 0x3F8+1      ; Divisor High
	mov al, 0x00
	out dx, al

	mov dx, 0x3F8+3      ; Line Control Register
	mov al, 0x03         ; 8 bits, sin paridad, 1 stop
	out dx, al

	mov dx, 0x3F8+2      ; FIFO Control
	mov al, 0xC7         ; habilitar FIFO
	out dx, al

	mov dx, 0x3F8+4      ; Modem Control
	mov al, 0x0B         ; RTS/DSR
	out dx, al
	ret

InternalSendCharToSerial:
    ; obtener argumento desde la pila
    mov eax, [esp+4]   ; primer argumento
    ; AL = carácter a enviar

    ; espera a que el transmisor esté listo
    mov dx, 0x3F8+5    ; Line Status Register
.waita:
    in al, dx
    test al, 0x20
    jz .waita

    ; enviar carácter
    mov dx, 0x3F8
    out dx, al

    ret
