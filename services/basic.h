#ifndef KernelBasicsDotH
#define KernelBasicsDotH
// enum de statuses
enum _KernelStatus;
// prototipo de servicios de kernel
struct _KernelServices;
// el error de status
#define _StatusError(Status) Status != 0
#endif