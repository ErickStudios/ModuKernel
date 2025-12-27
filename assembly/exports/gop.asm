
; funciones de configuracion y desconfiguracion de modos
global config_mode				; funcion que configura el modo grafico
global unconfig_mode			; funcion que sale del modo grafico pero no funciona, solo para buggearlo y terminar de configurarlo

; funciones del display y display avanzado
global InternalDrawBackground	; funcion para dibujar un fondo de un color
global InternalDrawPixel		; funcion para dibujar un pixel o varios
global InternalGopScreenInit	; funcion para inicializar la pantalla en modo grafico
global InternalGrapichalFlag	; bandera de graficos si esta en modo grafico
