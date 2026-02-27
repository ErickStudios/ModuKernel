org 0x8000
jmp start
; ========================================================================================
; SECCION DE DATOS
DATA_SECTION:
    ; typedef struct  _ErickFsFirstSector {
    ;     unsigned short TotalFiles;
    ;     unsigned short TotalSectors;
    ;     unsigned short TableStartSector;
    ;     unsigned short TablesPerSector;
    ;     unsigned int FirstFreeSector;
    ;     unsigned int InitCodeSizeBySectors;
    ;     unsigned int InitCodeSector;
    ; } ErickFsFirstSector;
    TotalFiles              dw 0
    TotalSectors            dw 0
    TableStartSector        dw 0
    TablesPerSector         dw 0
    FirstFreeSector         dd 0
    InitCodeSizeBySectors   dd 0
    InitCodeSector          dd 0
    ; numero de driver
    DriveNo                 db 0
; ========================================================================================
; INICIO DEL CODIGO
start:
    mov [DriveNo], dl
.init_segment:
    ; inicializa segmentos
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 7C00h

    mov [DriveNo], dl
.read_buffer:
    mov ah,02h
    mov al,1
    mov ch,0
    mov cl,2
    mov dh,0
    mov dl,[DriveNo]
    mov bx,0600h
    int 13h
    jc disk_error
.get_info:
    ; lee primero todos
    mov si,0600h
    lodsw
    mov word [TotalFiles], ax
    call PrintWord
    lodsw
    mov word [TotalSectors], ax
    lodsw
    mov word [TableStartSector], ax
    lodsw
    mov word [TablesPerSector], ax
    lodsw
    mov [FirstFreeSector], ax
    lodsw
    mov [FirstFreeSector+2], ax
    lodsw
    mov [InitCodeSizeBySectors], ax
    lodsw
    mov [InitCodeSizeBySectors+2], ax
    lodsw
    mov [InitCodeSector], ax
    lodsw
    mov [InitCodeSector+2], ax
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
; ========================================================================================
; ERROR DE DISCO
disk_error:
    mov al, '!'
    mov ah, 0xE
    int 10h
    jmp $