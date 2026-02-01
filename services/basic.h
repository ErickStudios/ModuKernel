#ifndef KernelBasicsDotH
#define KernelBasicsDotH
// encabezado
#include <stdint.h>
#include "Status.h"
#include "AllocType.h"
// prototipo de servicios de kernel
struct _KernelServices;
// el error de status
#define _StatusError(Status) Status != 0
// el tipo de objeto
typedef void* ObjectAny;
// el tipo de booleano
#ifndef __cplusplus
typedef uint8_t bool;
#endif
// funcion
typedef void function;
// el tipo de memoria y byte de array
typedef uint8_t* bytes;
#endif