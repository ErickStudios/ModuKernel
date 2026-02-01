#ifndef _StatusDotH_
#define _StatusDotH_
#ifdef __cplusplus
enum KernelStatus {
#else
typedef enum _KernelStatus {
#endif
    /* se puede */
    KernelStatusSuccess,
    /* no se encontro */
    KernelStatusNotFound,
    /* que fue tan desastrozo que nisiquiera se ModuKernel molesto en decir que diablos paso */
    KernelStatusDisaster,
    /* que se quedo congelado y lo unico que te salvo fue un timeout */
    KernelStatusInfiniteLoopTimeouted,
    /* que no hay presupuesto para mas memoria */
    KernelStatusNoBudget,
    /* error de memoria que no se puede leer o escribir */
    KernelStatusMemoryRot,
    /* error del disco que no se puede leer o escribir */
    KernelStatusDiskServicesDiskErr,
    /* el parametro que se mando fue uno que no es valido */
    KernelStatusInvalidParam,
    /* si algo es muy pequeño para ser usado o cargado */
    KernelStatusThingVerySmall,
    /* si el dispositivo esta opcupado */
    KernelStatusDeviceBusy
#ifdef __cplusplus
};
#else
} KernelStatus;
#endif
#ifdef __cplusplus
typedef KernelStatus _KernelStatus;
#endif
#endif