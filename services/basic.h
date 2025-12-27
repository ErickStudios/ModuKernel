#ifndef KernelBasicsDotH
#define KernelBasicsDotH
// encabezado
#include <stdint.h>
// enum de statuses
enum _KernelStatus;
// prototipo de servicios de kernel
struct _KernelServices;
// prototipo de tipo de memoria
enum _ModuAllocType;
// el error de status
#define _StatusError(Status) Status != 0
// el tipo de objeto
typedef void* ObjectAny;
// el tipo de booleano
typedef uint8_t bool;
// funcion
typedef void function;
// el tipo de memoria y byte de array
typedef uint8_t* bytes;
#endif