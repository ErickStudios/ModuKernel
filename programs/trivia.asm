; global
global ErickMain

; datos
section .data
    ; mensaje
    msg        db "Hello World!", 0

; bss
section .bss
    ; el buffer
    buffer     resb 64

; texto
section .text
; funcion principal
ErickMain:
    ; limpiar la pantalla
    mov eax, 1      ; servicio
    mov ecx, 1      ; llamada
    int 80h         ; llamar

    ; dibujar mensaje
    mov eax, 1      ; servicio
    mov ecx, 0      ; llamada
    mov ebx, msg    ; parametro
    int 80h         ; llamar

    ; llenar buffer?
    mov edi, buffer ; direccion
    mov ecx, 64     ; cuanto
    xor eax, eax    ; llenar
    rep stosb       ; repetir

    ; retornar
    mov eax, 0      ; estatus
    ret             ; regresar