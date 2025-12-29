global IdtDivideBy0
extern IdtDivideBy0Hnd

; handle para dividir por 0 y alli se ira
IdtDivideBy0:
    ; guardar los registros
    pusha                           ; guardar todos los generales
    push ds                         ; guardar ds
    push es                         ; guardar es
    push fs                         ; guardar fs
    push gs                         ; guardar gs

    ; llamar al stub c
    push esp                        ; ponerlo
    call IdtDivideBy0Hnd            ; handler
    add esp, 4                      ; vaciar la pila

    ; recuperar
    pop gs                          ; recuperar gs
    pop fs                          ; recuperar fs
    pop es                          ; recuperar es
    pop ds                          ; recuperar ds
    popa                            ; recuperar los generales

    ; retornar
    iret                            ; retornar