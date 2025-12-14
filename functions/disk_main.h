/* crea un proceso a partir de codigo ejecutable en un archivo en el fat12 */
KernelStatus ProcessCrtByFile(char* name, char* ext, KernelServices* Services);
/* lee un sector con ata */
KernelStatus InternalDiskReadSector(uint32_t lba, uint8_t* buffer);
/* encuentra un archivo */
FatFile InternalDiskFindFile(char* name, char* ext);
/* obtiene un archivo */
KernelStatus InternalDiskGetFile(FatFile file, void** content, int* size);
