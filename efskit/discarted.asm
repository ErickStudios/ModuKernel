; ========================================================================================
; IMPRIME UN WORD
PrintWord:
    push ax
    mov dx,ax          ; copia AX en DX
    mov cx,4           ; 4 dígitos hex
.next_digit:
    rol dx,4           ; rota la copia, no AX
    mov bl,dl
    and bl,0x0F
    cmp bl,9
    jbe .digit
    add bl,7
.digit:
    add bl,'0'
    mov ah,0x0E
    mov al,bl
    int 10h
    loop .next_digit
    pop ax
    ret
; ========================================================================================
; IMPRIME UN DWORD
PrintDWord:
    push ax
    push dx
    mov cx, 8            ; 8 dígitos hex
.next_digit:
    shl dx, 4
    rcl ax, 4            ; rota 32 bits en DX:AX
    mov bl, al
    and bl, 0x0F
    cmp bl, 9
    jbe .digit
    add bl, 7
.digit:
    add bl, '0'
    mov ah, 0x0E
    mov al, bl
    int 0x10
    loop .next_digit
    pop dx
    pop ax
    ret
