; Compilar: nasm -f elf32 userprog.asm -o userprog.o
; Linkear:  ld -m elf_i386 -T ABI/user_link.ld -o userprog.bin userprog.o

global ErickMain

section .data
msg        db "Hello World!", 0
msg_len    equ $ - msg

section .bss
buffer     resb 64

; eax: service
; ecx: function
; ebx: param1
; edx: param2

section .text
ErickMain:
    ; limpiar la pantalla
    mov eax, 1         ; llamadas a la pantalla
    mov ecx, 1         ; Display.clearscreen
    int 80h            ; llamar

    ; dibujar mensaje
    mov eax, 1         ; llamadas a la pantalla
    mov ecx, 0         ; Display.printg
    mov ebx, msg       ; mensaje
    int 80h            ; llamar

    mov edi, buffer
    mov ecx, 64
    xor eax, eax
    rep stosb

    mov eax, 0
    ret