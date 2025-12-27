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

