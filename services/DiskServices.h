#ifndef DiskServicesDotH
#define DiskServicesDotH
// declaracion temprana
struct _KernelServices;
// archivo fat
struct _FatFile;
// servicio de buscar archivos archivos
typedef struct _FatFile (*KernelSimpleDiskFindFile)(char* name, char* ext);
// cargar archivo
typedef int (*KernelSimpleDiskGetFile)(struct _FatFile file, void** content, int* size);
// servicio de ejecutar archivos
typedef int (*KernelSimpleDiskExecuteFile)(char* name, char* ext, struct _KernelServices* Services);
// servicios de disco
typedef struct _DiskServices {
    // servicio para ejecutar archivos
    KernelSimpleDiskExecuteFile RunFile;
    // para buscar archivos
    KernelSimpleDiskFindFile FindFile;
    // para cargar archivos
    KernelSimpleDiskGetFile GetFile;
} DiskServices;
#endif