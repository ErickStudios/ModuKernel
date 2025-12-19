#ifndef TimeServicesDotH
#define TimeServicesDotH
/* el tipo de espera */
typedef void (*KernelTimeServicesWait)(int unities);
/* la estructura */
typedef struct _TimeServices {
    /* funcion para esperar unidades de ErickTime, que
    no son exactamente segundos si no una unidad especifica que
    se puede esperar, esto puede ser diferente dependiendo de la
    fecuencia del procesador */
    KernelTimeServicesWait TaskDelay;
} TimeServices;
#endif