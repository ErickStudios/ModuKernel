; ========================================================================================
; RUTINA DE BOOTLOADER DE EFS KIT
;
; este archivo contiene el codigo asm de la rutina de arranque esperada de un disco ErickFS
; es usado por Python para insertar codigo de bootsector general ya que no cabe todo en un
; sector asi que este lanza el init code que ese si es configurable pero no es un archivo
org 0x7C00
; ========================================================================================
; SECCION DE MBR
MBR_SECTION:
    ; typedef struct _BootSector {
    ;     unsigned char jmp[3];
    ;     unsigned char OEMLabel[8];
    ;     unsigned short BytesPerSector;
    ;     unsigned char SectorsPerCluster;
    ;     unsigned short ReservedForBoot;
    ;     unsigned char NumberOfFats;
    ;     unsigned short RootDirEntries;
    ;     unsigned short LogicalSectors;
    ;     unsigned char MediumByte;
    ;     unsigned short SectorsPerFat;
    ;     unsigned short SectorsPerTrack;
    ;     unsigned short Sides;
    ;     unsigned int HiddenSectors;
    ;     unsigned int LargeSectors;
    ;     unsigned short DriveNo;
    ;     unsigned char Signature;
    ;     unsigned int VolumeID;
    ;     unsigned char VolumeLabel[11];
    ;     unsigned char FileSystem[8];
    ; } BootSector;
    jmp short start                     ; salto
    nop                                 ; instruccion para relleno
    OEMLabel                db "EfsKitPy" ; label de oem
    BytesPerSector          dw 512      ; bytes por sector
    SectorsPerCluster       db 1        ; sectores por cluster
    ReservedForBoot         dw 1        ; reservado para el arranque
    NumberOfFats            db 2        ; numero de fats
    RootDirEntries          dw 224      ; entradas de root
    LogicalSectors          dw 2880     ; sectores logicos
    MediumByte              db 0F0h     ; medio byte
    SectorsPerFat           dw 9        ; sectores por fat
    SectorsPerTrack         dw 18       ; sectores por pista
    Sides                   dw 2        ; lados
    HiddenSectors           dd 0        ; sectores ocultos
    LargeSectors            dd 0        ; sectores largos
    DriveNo                 dw 0        ; numero de dsipositivo
    Signature               db 41h      ; signatura
    VolumeID                dd 0        ; id de volumen
    VolumeLabel             db "ErickFSKit " ; label de volumen
    FileSystem              db "ErickFS " ; sistema de archivos
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
    TotalFiles              dw 0        ; total de archivos
    TotalSectors            dw 0        ; total de sectors
    TableStartSector        dw 0        ; el sector donde inicia la tabla
    TablesPerSector         dw 0        ; tablas por sector
    FirstFreeSector         dd 0        ; primer sector libre
    InitCodeSizeBySectors   dd 0        ; tamaño del codigo init
    InitCodeSector          dd 0        ; el sector donde inicia el codigo init
DAP:
    ; typedef struct  _DAP {
    ;     unsigned char PackSize;
    ;     unsigned short TotalSectors;
    ;     unsigned char Reserved;
    ;     unsigned short NumberOfSectors;
    ;     unsigned short DstOffset;
    ;     unsigned short DstSegment;
    ;     unsigned long long LBA;
    ; } DAP;
    db                      16          ; tamaño del paquete
    db                      0           ; reservado
    dw                      0           ; número de sectores a leer
    dw                      0           ; offset destino
    dw                      0           ; segmento destino
    dd                      0           ; LBA bajo
    dd                      0           ; LBA alto
; ========================================================================================
; INICIO DE CODIGO
start:
    cli                                 ; desactiva interrupciones
.init_segment:
    ; inicializa segmentos
    xor ax, ax                          ; poner a 0 ax
    mov ds, ax                          ; sector de datos
    mov es, ax                          ; sector de extra segment
    mov ss, ax                          ; segmento de stack
    mov sp, 7C00h                       ; donde inicia el stack
    mov [DriveNo], dl                   ; numero de dispositivo

.read_buffer:
    ; lee el buffer
    mov ah,02h                          ; funcion
    mov al,1                            ; sectores a leer
    mov ch,0                            ; dato 1
    mov cl,2                            ; sector que leera
    mov dh,0                            ; dato 2
    mov dl,[DriveNo]                    ; el numero de dispositivo
    mov bx,0600h                        ; donde lo leera
    int 13h                             ; leerlo
    jc disk_error                       ; si algo fallo ir a error de disco
.get_info:
    ; lee primero todos
    mov si,0600h                        ; donde estan los datos
    lodsw                               ; leer
    mov word [TotalFiles], ax           ; numero de archivos
    lodsw                               ; leer
    mov word [TotalSectors], ax         ; numero de sectores
    lodsw                               ; leer
    mov word [TableStartSector], ax     ; el sector donde inicia la tabla
    lodsw                               ; leer
    mov word [TablesPerSector], ax      ; tablas por sector
    lodsw                               ; leer
    mov [FirstFreeSector], ax           ; primer sector libre
    lodsw                               ; leer
    mov [FirstFreeSector+2], ax         ; primer sector libre parte 2
    lodsw                               ; leer
    mov [InitCodeSizeBySectors], ax     ; donde inicializa el codigo
    lodsw                               ; leer
    mov [InitCodeSizeBySectors+2], ax   ; donde inicializa el codigo 2
    lodsw                               ; leer
    mov [InitCodeSector], ax            ; donde inicia
    lodsw                               ; leer
    mov [InitCodeSector+2], ax          ; donde inicia 2
.load_init:
    ; llenar DAP
    mov ax, [InitCodeSizeBySectors]     ; el dap
    mov [DAP+2], ax                     ; leerlo

    ; mover dap
    mov word [DAP+4], 8000h             ; offset
    mov word [DAP+6], 0000h             ; segmento

    ; inicializar
    mov ax, [InitCodeSector]            ; codigo de inicializacion
    mov [DAP+8], ax                     ; mover
    mov ax, [InitCodeSector+2]          ; codigo de inicializacion 2
    mov [DAP+10], ax                    ; mover

    ; mover palabras
    mov word [DAP+12], 0                ; setear dap12 a 0
    mov word [DAP+14], 0                ; setear dap14 a 0

    ; guardar
    push ds                             ; guardar ds
    xor ax, ax                          ; setear ax a 0
    mov ds, ax                          ; mover el segmento de datos

    ; llamar al int del dap
    mov si, DAP                         ; el dap
    mov dl, [DriveNo]                   ; nombre de dispositivo
    mov ah, 42h                         ; la accion
    int 13h                             ; llamar al disco
    jc disk_error                       ; si hay error entonces llamar al fail
    pop ds                              ; recuperar ds

    ; parametros costumizados no necesarios pero para informacion
    mov dl, [DriveNo]                   ; el numero de dispositivo
    mov ax, [SectorsPerTrack]           ; sectores por track
    mov bx, [Sides]                     ; lados
    call 0000h:8000h                    ; llamar
.hand:
    jmp $                               ; saltar a si mismo
; ========================================================================================
; ERROR DE DISCO
disk_error:
    ; imprimir una exclamacion
    mov al, '!'                         ; el signo
    mov ah, 0xE                         ; teletipo
    int 10h                             ; imprimir
    jmp $                               ; loop infinito
; ========================================================================================
; RELLENO HASTA 510 BYTES
padding:
    times 510-($-$$) db 0               ; relleno
; ========================================================================================
; FIRMA DEL BOOTSECTOR
bootsector_end:
    dw 0xAA55                           ; firma