	; mensaje auxilear de que ha fallado
	msg db 10,'El kernel ha fallado y se ha regresado al entorno pre-C, reinicie la pc para continuar',10,0

	; etapa de arranque actual
	InternalBStg db 0			; etapa actual
	InternalBStgG db 0 			; grupo de etapas actual
