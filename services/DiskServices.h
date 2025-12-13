#ifndef DiskServicesDotH
#define DiskServicesDotH
// incluir basicos
#include "basic.h"
// declaracion temprana
struct _KernelServices;
// archivo fat
struct _FatFile;
// servicio de buscar archivos archivos
typedef struct _FatFile (*KernelSimpleDiskFindFile)(char* name, char* ext);
// cargar archivo
typedef enum _KernelStatus (*KernelSimpleDiskGetFile)(struct _FatFile file, void** content, int* size);
// servicio de ejecutar archivos
typedef enum _KernelStatus (*KernelSimpleDiskExecuteFile)(char* name, char* ext, struct _KernelServices* Services);
// para leer un sector
typedef enum _KernelStatus (*KernelSimpleDiskReadSector)(unsigned int lba, unsigned char* buffer);
// para usar el tipo de directorios extendidos
typedef struct _FatFile (*KernelSimpleDiskOpen)(char* path);
// servicios de disco
typedef struct _DiskServices {
    // servicio para ejecutar archivos
    KernelSimpleDiskExecuteFile RunFile;
    // para buscar archivos
    KernelSimpleDiskFindFile FindFile;
    // para cargar archivos
    KernelSimpleDiskGetFile GetFile;
    // para leer sectores
    KernelSimpleDiskReadSector ReadSector;
    // para abrir un archivo mas facilmente
    KernelSimpleDiskOpen OpenFile;
} DiskServices;
#endif