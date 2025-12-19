#ifndef IOServicesDotH
#define IOServicesDotH
/* para teclas especiales */
#define KernelSimpleIoSpecKey(Id) (0xC0 | Id)
/* para teclas de funcion */
#define KernelSimpleIoFuncKey(Id) (0xD0 | Id)
/* el tipo de input */
typedef unsigned char (*KernelSimpleIoInput)(unsigned short port);
/* el tipo de outpud */
typedef void (*KernelSimpleIoOutpud)(unsigned short port, unsigned char val);
/* el tipo de teclado y tecla */
typedef unsigned char (*KernelSimpleIoKeyWait)();
/* el tipo de leer linea */
typedef char* (*KernelSimpleIoReadLine)();
/* el tipo de teclado sin esperar */
typedef unsigned char (*KernelSimpleIoKeyRead)();
/* el tipo de servicios de salida */
typedef struct _IoServices {
    /* funcion para obtener un dato de un puerto, el primer y unico
    parametro contiene el identificador del tipo de puerto en un
    uint16, retorna el dato recivido */
    KernelSimpleIoInput Input;
    /* funcion para mandar un dato a un puerto, el primer parametro
    contiene el identificador del tipo de puerto en un uint16,
    el segundo contiene el valor en un uint8 al cual se le tiene que mandar*/
    KernelSimpleIoOutpud Outpud;
    /* funcion para esperar a que se presione una tecla y leerla
    retorna el caracter de la tecla recibida o en otros casos de tecla
    el codigo de la tecla que se puede obtener con la macro KernelSimpleIoSpecKey 
    en el caso de las teclas especiales y para obtener las de funcion (como f1
    , f2 y f3 ...) se usa KernelSimpleIoFuncKey */ 
    KernelSimpleIoKeyWait WaitKey;
    /* funcion para pedirle al usuario que ingrese un texto que este se mostrara una
    vista previa en la consola conforme el usuario presione las teclas, y enter para
    confirmar, esta funcion a diferencia de las tradicionales no imprime un salto de 
    linea al final, retorna un array de caracteres ascii que estos tienen que ser
    liberados con la funcion para liberar memoria que esta en los servicios de memoria
    del kernel (FreePool) ya que se crea con (AllocatePool) y para evitar problemas
    necesita librerarla */
    KernelSimpleIoReadLine ReadLine;
    /* funcion para leer una tecla, esta funcion hace lo mismo que WaitKey solo que
    la unica diferencia es que no espera, puedes estar por ejemplo ejecutando algo y
    mientras lo ejecutas puedes leer una tecla para interrumpir o hacer algo interesante
    con multitarea basica */
    KernelSimpleIoKeyRead ReadKey;
} IoServices;
#endif