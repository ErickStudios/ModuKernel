#ifndef KernelServicesDotH
#define KernelServicesDotH
// incluir los servicios
#include "DisplayServices.h"
#include "MemoryServices.h"
#include "IOServices.h"
#include "DiskServices.h"
#include "TimeServices.h"
#include "MusicServices.h"
/* codigos de error entre otros */
typedef enum _KernelStatus {
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
    KerneLStatusThingVerySmall
} KernelStatus;
/* el tipo de hora y fecha que es una estructura para hacerlo mas limpio */
typedef struct _KernelDateTime {
    /* contiene el año actual */
    int year;
    /* contiene el mes actual */
    int month; 
    /* contiene el dia actual */
    int day;
    /* contiene la hora actual */
    int hour;
    /* contiene el minuto actual */
    int minute;
    /* contiene el segundo actual */
    int second;
} KernelDateTime;
/* el tipo para ejecutar un comando */
typedef void (*KernelServicesExecuteCommand)(struct _KernelServices* Services, char* command, int len);
/* el tipo para engendrar servicios */
typedef void (*KernelServicesGiveBirth)(struct _KernelServices*);
/* el tipo para apagar o reiniciar */
typedef KernelStatus (*KernelServicesReset)(int func);
/* el tipo para ejecutar binarios */
typedef KernelStatus (*KernelServicesRunBinary)(void* buffer, int size, struct _KernelServices* Services);
/* el tipo para obtener la fecha y hora */
typedef void (*KernelServicesGetTimeDate)(KernelDateTime* Time);
/* el tipo para obtener la cantidad de ticks que transcurrieron desde que inicio la maquina*/
typedef unsigned long long (*KernelServicesGetTicks)();
/* el tipo para hacer un panic */
typedef void (*KernelServicesThrow)(KernelStatus Status);
/* el tipo de servicios otros */
typedef struct _KernelMiscServices {
    /* funcion para ejecutar un comando desde la shell, requiere como 
    parametro el puntero a los servicios de kernel, el comando en texto,
    y una longitud que de hecho no se usa pero esta por compatibilidad*/
    KernelServicesExecuteCommand Run;
    /* funcion para crear una nueva estructura de servicios de kernel,
    necesita como parametro un puntero a los servicios a inicializar */
    KernelServicesGiveBirth GiveBirth;
    /* funcion para, requiere un unico parametro que es la accion de poder 
    reiniciar o apagar (0=reiniciar, 1=apagar) , retorna como le fue*/
    KernelServicesReset Reset;
    /* funcion para ejecutar un binario, el primer parametro es el buffer al
    contenido del binario, el segundo es el tamaño, y el tercero es el puntero 
    que el binario recibira en su main para conectarlo con el kernel */
    KernelServicesRunBinary RunBinary;
    /* funcion para obtener la fecha y hora, como unico parametro usa
    un puntero a la estructura del tiempo*/
    KernelServicesGetTimeDate GetTime;
    /* variable que apunta a los parametros extras con los cuales se ejecuta
    un programa */
    void*** Paramaters;
    /* variable que apunta a la longitud de los parametros */
    int* ParamsCount;
    /* funcion para obtener el numero de ciclos de relog que transcurrieron desde
    que inicio la maquina ya sea qemu o maquina real */
    KernelServicesGetTicks GetTicks;
    /* lanza un error a nivel de kernel si es necesario no te preocupes en
    usarlo el usuario puede presionar enter para continuar */
    KernelServicesThrow Throw;
} KernelMiscServices;
/* esta estructura contiene la tabla de servicios del kernel 
para que sea mas comprensible , organizado y facil de usar */
typedef struct _KernelServices {
    /* propiedad que contiene un puntero a la version actual de los servicios */
    unsigned int* ServicesVersion;
    /* servicio para controlar la pantalla y la consola*/
    DisplayServices* Display;
    /* servicio para administrar memoria dinamica */
    MemoryServices* Memory;
    /* servicio para entrada y salida de dispositivos */
    IoServices* InputOutput;
    /* servicio para administracion de FAT12 y FAT12 extendido */
    DiskServices* File;
    /* servicios miscelianos que no encajan en ninguna otra categoria */
    KernelMiscServices* Misc;
    /* servicios del tiempo */
    TimeServices* Time;
    /* servicios de musica */
    MusicServices* Music;
} KernelServices;
#endif