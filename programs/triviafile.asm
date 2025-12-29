global ErickMain

section .data
path        db "/home/test1.txt", 0
file_handle dd 0
content_ptr dd 0
size_ptr    dd 0         ; puntero a int; puedes reservar memoria antes si lo prefieres

section .bss
buffer     resb 64

section .text
ErickMain:
    mov eax, 4          ; servicio de archivos
    mov ecx, 0          ; OpenFile (ruta extendida)
    mov ebx, path       ; path
    mov edx, file_handle ; donde guardar el _FatFile*
    int 0x80

    mov esi, [file_handle]

    ; preparar size_ptr (si no tienes malloc en userland, apunta a una celda en .data)
    ; Aquí lo dejamos como variable global que el kernel llenará.
    ; SysCallParam3 se setea con call=0, function=0 (según tu handler).
    mov eax, 0          ; set params “temporales”
    mov ecx, 0
    mov ebx, size_ptr   ; kernel guardará size en *size_ptr durante GetFile
    int 0x80

    ; invocar GetFile
    mov eax, 4          ; files
    mov ecx, 1          ; GetFile
    mov ebx, esi        ; _FatFile*
    mov edx, content_ptr ; donde guardar void*
    int 0x80

    ; ahora content_ptr -> buffer, *size_ptr = tamaño
    mov edi, [content_ptr] ; buffer
    mov ecx, [size_ptr]    ; tamaño

    mov eax, 1
    mov ecx, 0
    mov ebx, content_ptr

    int 0x80

    ret