org 0x7C00

jmp short start   ; EB 3C
nop               ; 90

; datos de cosas
OEMLabel        db "EfsKitPy"     ; 8 bytes
BytesPerSector  dw 512
SectorsPerCluster db 1
ReservedForBoot dw 1
NumberOfFats    db 2
RootDirEntries  dw 224
LogicalSectors  dw 2880
MediumByte      db 0F0h
SectorsPerFat   dw 9
SectorsPerTrack dw 18
Sides           dw 2
HiddenSectors   dd 0
LargeSectors    dd 0
DriveNo         dw 0
Signature       db 41h
VolumeID        dd 0
VolumeLabel     db "ErickFSKit "  ; 11 chars
FileSystem      db "ErickFS "     ; 8 chars

; inicio del codigo
start:
    ; inicializa segmentos
    mov ax,07C0h
    mov es,ax
    mov ax,es

    ; leer sector 2 (después del boot)
    mov ah,02h        ; función leer sector
    mov al,1          ; 1 sector
    mov ch,0          ; cilindro 0
    mov cl,2          ; sector 2
    mov dh,0          ; cabeza 0
    mov dl,[DriveNo]  ; unidad
    mov bx,0x0600     ; destino en memoria
    int 13h           ; BIOS copia en ES:BX

    ; parametros de la funcion
    mov si, 0x0600

    ; guarda  los parametros
    mov si, 0x0600
    lodsw          ; TotalFiles → AX
    push ax
    lodsw          ; TotalSectors → AX
    push ax
    lodsw          ; TableStartSector → AX
    push ax
    lodsw          ; TablesPerSector → AX
    push ax
    lodsd          ; FirstFreeSector → EAX
    push dx        ; parte alta
    push ax        ; parte baja
    lodsd          ; InitCodeSizeBySectors → EAX
    push dx
    push ax
    lodsd          ; InitCodeSector → EAX
    push dx
    push ax
    call info_interprete

; ========================================================================================
; para imprimir un string
PrintString:
    ; cargar
    push bp                                             ; guarda bp
    mov bp, sp                                          ; mueve bp a sp
    mov si, [bp+4]                                      ; parámetro: dirección del string
.loop:
    ; el loop
    lodsb                                               ; carga el byte
    cmp al, 0                                           ; lo compara con 0
    je .done                                            ; si es 0 termino
    int 0x10                                            ; imprimirlo
    jmp .loop                                           ; loopear
.done:
    ; el para regresar
    pop bp                                              ; recupera bp
    ret                                                 ; regresa

; ========================================================================================
; para interpretar informacion
info_interprete:
    push bp
    mov bp, sp

    mov bx,info1
    mov cx,[bp+4]
    call PrOptimization1

    mov bx,info2
    mov cx,[bp+6]
    call PrOptimization1

    mov bx,info3
    mov cx,[bp+8]
    call PrOptimization1

    mov bx,info4
    mov cx,[bp+10]
    call PrOptimization1

    mov ah, 0x0E
    push info5
    call PrintString
    mov dx, [bp+12]  ; parte baja de FirstFreeSector
    mov cx, [bp+14]  ; parte alta
    ; aquí puedes imprimirlo como doble palabra (ejemplo simplificado)
    call PrintDWord
    mov ah, 0x0E
    push lineNew
    call PrintString

    pop bp
    ret

; ========================================================================================
; imprime un word
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
; imprime un dword
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

PrOptimization1:
    mov ah, 0x0E
    push bx
    call PrintString
    pop bx
    mov bx, bp        ; base del marco de pila
    add bx, cx        ; sumas el desplazamiento dinámico
    mov ax, [bx]      ; ahora sí, acceso válido
    call PrintWord
    mov ah, 0x0E
    push lineNew
    call PrintString
    pop bx
    ret

; info chan
msg db "Disk func is comming soon. stats:",13,10,13,10,0
info1 db "#Files: ",0
info2 db "#Sectors: ",0
info3 db "TableStartSector: ",0
info4 db "#FilesPerSector: ",0
info5 db "FirstFreeSector: ",0
lineNew db 13,10,0

; --- Relleno hasta 510 bytes ---
times 510-($-$$) db 0
dw 0xAA55