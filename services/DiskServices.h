#ifndef DiskServicesDotH
#define DiskServicesDotH
// incluir basicos
#include "basic.h"
/* declaracion temprana */
struct _KernelServices;
/* archivo fat */
struct _FatFile;
/* servicio de buscar archivos archivos */
typedef struct _FatFile (*KernelSimpleDiskFindFile)(char* name, char* ext);
/* cargar archivo */
typedef enum _KernelStatus (*KernelSimpleDiskGetFile)(struct _FatFile file, void** content, int* size);
/* servicio de ejecutar archivos */
typedef enum _KernelStatus (*KernelSimpleDiskExecuteFile)(char* name, char* ext, struct _KernelServices* Services);
/* para leer un sector */
typedef enum _KernelStatus (*KernelSimpleDiskReadSector)(unsigned int lba, unsigned char* buffer);
/* para usar el tipo de directorios extendidos */
typedef struct _FatFile (*KernelSimpleDiskOpen)(char* path);
/* servicios de disco */
typedef struct _DiskServices {
    /* funcion para ejecutar un archivo, esta funcion requiere
    como parametro el nombre (en mayuscula solo 8 caracteres), 
    la extension (en mayuscula solo 3 caracteres) y un puntero 
    a los servicios del kernel, retorna el status con el cual
    el programa salio*/
    KernelSimpleDiskExecuteFile RunFile;
    /* funcion para buscar archivos, esta funcion requiere como
    parametro el nombre (solo 8 caracteres en mayusculas) y
    la extension (solo 3 caracteres en mayuscular), y retorna
    una estructura con informacion del archivo y el bootsector en
    el cual esta */
    KernelSimpleDiskFindFile FindFile;
    /* funcion para cargar archivos, esta funcion requiere como parametro
    la estructura del archivo que se ha encontrado anteriormente con
    FindFile y OpenFile, un puntero al contenido que se devolvera alli, y
    un puntero a un int que contendra el tamaño del archivo, retorna el
    status de como le fue al encontrar el archivo */
    KernelSimpleDiskGetFile GetFile;
    /* funcion para leer sectores, esta funcion requiere como parametro
    un lba y un puntero a el contenido del sector donde lo retornara,
    retorna el status de como le fue*/
    KernelSimpleDiskReadSector ReadSector;
    /* funcion para abrir un archivo con un formato mas simple,
    esta funcion requiere como unico parametro una ruta tipo 
    '/ruta/a/archivo', sobre como funciona pues hay un archivo interno
    llamado FSLST.IFS que contiene un mapa con las direcciones complejas
    de los archivos aunque internamente esta funcion esconde lo mas posible
    las limitaciones de Fat12, si quieres usarlo de manera mas simple pues
    utiliza esta funcion, retorna una estructura con informacion del archivo 
    y el bootsector en el cual esta  */
    KernelSimpleDiskOpen OpenFile;
} DiskServices;
#endif