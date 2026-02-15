// incluir archivos
#include "../services/KernelServices.h"
#include "../fs/structs.h"
#include "../handlers/regs.h"

// funciones desde C
extern "C" void* AllocatePool(unsigned int size);
extern "C" KernelServices* GlobalServices;
extern "C" void packages_gc(regs_t* r);
extern "C" void * InternalMemoryCopy(void *dest, const void *src, unsigned int n);
extern "C" void* InternalMemorySet(void* dest, int value, unsigned int count);
extern "C" KernelStatus InternalDiskReadSector(unsigned int lba, unsigned char* buffer);
extern "C" void outb(uint16_t port, uint8_t val);

// variables desde C
extern uint32_t PitCounter;

/// @brief el handler del pic
struct PicHandler {
    /// @brief el paso del pic
    /// @param r los registros
    static void Step(regs_t* r) {
        // acciones multitarea 
        packages_gc (r);							// recolector de buses basuar

        // interrupcion terminada
        PitCounter++;								// incrementar los eticks que pasaron
        outb(0x20, 0x20); 							// terminar la interrupcion
    }
};
/// @brief el sistema de archivos de ErickFS
struct ErickFS {
    /// @brief lee el contenido desde un ErickFile Entry
    /// @param File el archivo
    /// @param content el contenido
    /// @param size el tamaño
    /// @return el archivo
    static KernelStatus GetFileErickFSContent(FatFile File, void** content, int* size)
    {
        // donde inicia
        int SectorStart = File.ErickFSentry.FileContentPtr / 512;
        // los sectores a leer
        int SectorsToSize = (File.ErickFSentry.FileContentSize / 512) + 1;
        
        // el buffer
        uint8_t* buffer = (uint8_t*)AllocatePool(SectorsToSize * 512);

        // leer sector
        for (int i = 0; i < SectorsToSize; i++) InternalDiskReadSector(SectorStart + i, buffer + (i * 512));

        // el contenido
        *content = buffer;
        // el tamaño
        *size    = File.ErickFSentry.FileContentSize;

        // retornar
        return KernelStatus::KernelStatusSuccess;
    }
};

/// @brief el manejador del pic que se exportara a C
/// @param r el registro
extern "C" void pic_handler(regs_t* r) {
    PicHandler::Step(r);
}
/// @brief obtiene el contenido de un archivo par C
/// @param File el archivo
/// @param content el contenido
/// @param size el tamaño
/// @return el contenido
extern "C" KernelStatus ErickFS_GetFileContent(FatFile File, void** content, int* size) {
    return ErickFS::GetFileErickFSContent(File, content, size);
}