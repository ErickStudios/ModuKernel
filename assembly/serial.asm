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
    push dx                ; salvar dx

    mov al, [esp+6]        ; obtener argumento (char ch)
    mov bl, al             ; guardar en BL para no perderlo

    ; esperar transmisor listo
    mov dx, 0x3F8+5
.waita:
    in  al, dx
    test al, 0x20
    jz .waita

    ; enviar carácter
    mov dx, 0x3F8
    mov al, bl             ; recuperar el carácter original
    out dx, al

    pop dx
    ret