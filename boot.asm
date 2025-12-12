; boot.asm - Bootloader simple FAT12, busca KERNEL  EBX (8.3) en root
; lee root dir, carga primer cluster en 0x10000, entra a protegido y salta
BITS 16
ORG 0x7C00

jmp start
nop

; ===== BPB FAT12 válido =====
OEMLabel        db "MODUKRN "
BytesPerSector  dw 512
SectorsPerCluster db 1
ReservedSectors dw 1
NumberOfFATs    db 2
RootEntries     dw 224
TotalSectors16  dw 2880
MediaDescriptor db 0xF0
SectorsPerFAT   dw 9
SectorsPerTrack dw 18
NumberOfHeads   dw 2
HiddenSectors   dd 0
TotalSectors32  dd 0

start:
    cli
    xor ax, ax
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov sp, 0x7C00
    sti

    mov si, msg_boot
    call print

; ----------- Parse BPB at 0x7C00 ----------
    mov bx, 0x7C00

    mov ax, [bx+11]      ; bytes per sector
    mov [bps], ax

    mov al, [bx+13]      ; sectors per cluster
    mov [spc], al

    mov ax, [bx+14]      ; reserved sectors
    mov [reserved], ax

    mov al, [bx+16]      ; number of FATs
    mov [num_fats], al

    mov ax, [bx+17]      ; root entries
    mov [root_entries], ax

    mov ax, [bx+22]      ; FAT size (sectors)
    mov [fat_size], ax

    mov ax, [bx+24]      ; sectors per track (spt)
    mov [spt], ax

    mov ax, [bx+26]      ; heads
    mov [heads], ax

; ----------- root_sectors = (root_entries * 32) / bps ----------
    mov ax, [root_entries]
    shl ax, 5            ; *32
    mov bx, [bps]
    xor dx, dx
    div bx               ; AX = number of sectors for root dir
    mov [root_secs], ax

; ----------- root_start = reserved + num_fats * fat_size ----------
    mov ax, [fat_size]
    mov bx, [num_fats]
    mul bx               ; DX:AX = fat_size * num_fats
    add ax, [reserved]
    adc dx, 0
    mov [root_start], ax    ; assume fits 16-bit for small images

; ----------- Read root directory sectors consecutively at 0x0200 ----------
    mov ax, [root_start]       ; first root LBA
    mov [sector], ax

    mov cx, [root_secs]        ; number of root sectors
    mov bx, 0x0200             ; target offset within segment 0
    mov di, 0                  ; sector counter

read_root_loop:
    ; compute LBA = root_start + di  (AX holds [sector] so copy)
    mov ax, [sector]
    add ax, di
    push ax
    call lba_to_chs            ; returns CH/CL/DH (int13 params)
    pop ax

    ; set ES:BX to where we want to store this sector:
    mov ax, 0x0000
    mov es, ax
    mov bp, bx                 ; save current target offset in BP (BP will be the low offset)
    ; BX contains offset where to write: initial 0x0200 + di*512
    ; BIOS int13 uses ES:BX as destination (word offset)
    mov ah, 0x02
    mov al, 1
    int 0x13

    add bx, 512               ; next sector target offset (works for small root sizes)
    inc di
    loop read_root_loop

; ----------- Search root entries (each 32 bytes) ----------
    mov si, filename
    mov di, 0x0200            ; start of root in memory
    mov cx, [root_entries]    ; number of entries to check

search_entries:
    push cx
    mov bx, di                ; pointer to entry
    mov si, filename
    mov dx, 11                ; compare 11 bytes (8.3)
cmp_loop2:
    mov al, [bx]
    cmp al, [si]
    jne not_this_entry2
    inc bx
    inc si
    dec dx
    jnz cmp_loop2
    ; match found
    jmp entry_found2

not_this_entry2:
    pop cx
    add di, 32
    dec cx
    jnz search_entries

    mov si, msg_notfound
    call print
    jmp $

entry_found2:
    ; di points to entry start
    mov ax, [di+26]           ; first cluster (low word)
    mov [first_cluster], ax

; ----------- Compute kernel LBA and read first sector into 0x10000 ----------
    ; data_start = root_start + root_secs
    mov ax, [root_start]
    add ax, [root_secs]
    mov [data_start], ax

    ; (cluster-2) * sectors_per_cluster
    mov ax, [first_cluster]
    sub ax, 2
    mov bx, ax
    mov ax, bx
    mov cx, [spc]
    mul cx                    ; AX = (cluster-2) * spc
    add ax, [data_start]
    mov [kernel_lba], ax

    ; read kernel first sector to ES:BX = 0x1000:0x0000
    mov ax, [kernel_lba]
    push ax
    call lba_to_chs
    pop ax
    mov ax, 0x1000
    mov es, ax
    xor bx, bx
    mov ah, 0x02
    mov al, 1
    int 0x13

    jmp enter_pm


; ---------------- LBA -> CHS routine ----------------
; Input: AX = LBA
; Output: CH = cylinder low, CL = sector (bits0-5) + high bits of cylinder in bits6-7, DH = head
lba_to_chs:
    push ax 
    push bx 
    push cx 
    push dx 
    push si 
    push di

    mov bx, [spt]      ; sectors per track
    xor dx, dx
    div bx             ; AX = LBA / spt ; DX = LBA % spt (remainder)
    ; save remainder (sector index)
    push dx

    mov bx, [heads]
    xor dx, dx
    mov si, ax         ; si = quotient (LBA / spt)
    mov ax, si
    div bx             ; AX = cylinder, DX = head

    mov ch, al         ; cylinder low
    mov dh, dl         ; head

    pop dx             ; DX = remainder (sector index)
    ; sector number = remainder + 1
    mov cl, dl
    inc cl

    ; high bits of cylinder (bits 8-9) go into CL bits 6-7
    ; get cylinder high bits (AX originally has cylinder in AL, AH may contain zero since cylinder < 1024)
    ; We assume cylinder < 1024 => high two bits are (CHIGH = (ch >> 8) & 3). For simplicity, zero them.
    ; If needed to support >255 cylinders add code to compute them.
    ; For now set bits 6-7 to zero (most images small)
    and cl, 0x3F

    pop di
    pop si
    pop dx
    pop cx
    pop bx
    pop ax
    ret

; ---------------- Protected mode setup ----------------
gdt_start:
    dq 0
    dq 0x00CF9A000000FFFF
    dq 0x00CF92000000FFFF
gdt_end:

gdt_desc:
    dw gdt_end - gdt_start - 1
    dd gdt_start

enter_pm:
    cli
    lgdt [gdt_desc]
    mov eax, cr0
    or eax, 1
    mov cr0, eax
    jmp 0x08:pm_start

BITS 32
pm_start:
    mov ax, 0x10
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov esp, 0x90000
    ; Jump to kernel entry at 0x10000
    jmp 0x10000

; ---------------- Print routine ----------------
print:
    pusha
.print_loop:
    lodsb
    or al, al
    jz .print_done
    mov ah, 0x0E
    int 0x10
    jmp .print_loop
.print_done:
    popa
    ret

; --------------- Data and variables ----------------
bps            dw 0
spc            dw 0
reserved       dw 0
num_fats       db 0
root_entries   dw 0
fat_size       dw 0
spt            dw 0
heads          dw 0
root_secs      dw 0
root_start     dw 0
sector         dw 0
first_cluster  dw 0
data_start     dw 0
kernel_lba     dw 0

filename db "KERNEL  BIN"
msg_boot db "ModuKernel Boot...",0
msg_notfound db "KERNEL.BIN missing",0

TIMES 510-($-$$) db 0
DW 0xAA55
