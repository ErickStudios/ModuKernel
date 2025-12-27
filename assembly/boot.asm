
; principal
start:
	; nivel de privilegios
	mov dword [InternalRingLevel],0 ; nivel

	; setear variables
	mov byte [InternalBStg], 0	;  etapa = start
	mov byte [InternalBStgG], 0	; grupo de etapas = pre-C

	; stub
	jmp pre_c_boot_stage1		; stub para el no se que
	hlt							; por si acaso

; etapa 2 donde se hace todo lo de mas bajo nivel
pre_c_boot_stage1:
	; etapa pre-prematura, aqui se configura todo y las tareas de bajo nivel
	; que no se pueden hacer en C se hacen

	; etapa actual
	mov byte [InternalBStg], 1	;  etapa = first

	; funciones de estandart
    cli							; desactivar interrupciones
	jmp pre_c_boot_stage2		; cambiar de etapa
	hlt							; por si acaso

; etapa 3 donde se inicializa el gdt
pre_c_boot_stage2:

	; etapa actual
	mov byte [InternalBStg], 2	;  etapa = init gdt

	; inicializar gdt
	call init_gdt				; funciones de gdt, esto es en C por que no tenia idea de como hacerlo en ASM asi que es una excepcion a pre-C, luego cambia a asm, no se preocupen
	lgdt [gdt_ptr]       		; carga la GDT definida en C
	
	; recargas
	jmp 0x08:flush_cs    		; far jump para recargar CS
	hlt							; que diablos tiene que pasar para que pase eso?

; esta funcion hace que cs se flushee
flush_cs:

	; inicializar AX
    mov ax, 0x10				; valor

	; inicializar valores
    mov ds, ax					; ax->ds
    mov es, ax					; ax->es
    mov fs, ax					; ax->fs
    mov gs, ax					; ax->gs
    mov ss, ax					; ax->ss

	; ir a la etapa 3 del arranque
	jmp pre_c_boot_stage3		; cambiar de etapa
	hlt							; por si acaso...

; etapa 3 del arranque
pre_c_boot_stage3:
	; etapa actual
	mov byte [InternalBStg], 3	;  etapa = last asm stage

	; funciones de kernel y inicializar
	call init_serial			; iniciar serial

	; saltar a c
	mov byte [InternalBStg], 0	;  etapa = k_main
	mov byte [InternalBStgG], 1	; grupo de etapas = in-C
	inc	dword [InternalRingLevel]; aumentar nivel
    call k_main					; preparar al kernel para la etapa post-prematura y el arranque C prematuro

	; cuando termina el kernel
	mov byte [InternalBStgG], 1	; grupo de etapas = in-C
	jmp kernel_ended			; si algo sale mal
	hlt							; por si acaso

; fin del kernel
kernel_ended:
	; aqui si algo salio mal en el arranque aqui se hara un codigo de emergencia
	; que limpiara la pila y congelara el kernel

	; detencion personalizada
    push msg					; el mensaje
	call InternalPrintgNonLine	; mandar a imprimir
    add esp, 4					; no hay nada

	; detencion estandart
	call InternalKernelHaltReal	; si algo sale mal congelar
	hlt							; por que pasara esto, bueno esta por si acaso
