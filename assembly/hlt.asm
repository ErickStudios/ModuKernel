
; halt detiene el procesador para hacer cosas que ya no estan
; y por ejemplo se puede detener
InternalKernelHaltReal:

	; instrucciones de por medio
	nop							; hacer nada

	; llamar
	jmp InternalKernelHaltReal	; llamar a halt

	hlt