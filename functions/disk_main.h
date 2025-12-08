// crea un proceso a partir de codigo ejecutable en un archivo en el fat12
int ProcessCrtByFile(char* name, char* ext, KernelServices* Services);
// lee un sector con ata
void InternalDiskReadSector(uint32_t lba, uint8_t* buffer);
// encuentra un archivo
FatFile InternalDiskFindFile(char* name, char* ext);
// obtiene un archivo
int InternalDiskGetFile(FatFile file, void** content, int* size);
