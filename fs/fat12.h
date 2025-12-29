typedef struct _FAT12_DirEntry
{
    uint8_t  name[11];             // 0
    uint8_t  attr;                 // 11
    uint8_t  reserved;             // 12
    uint8_t  creation_time_tenths; // 13
    uint16_t creation_time;        // 14
    uint16_t creation_date;        // 16
    uint16_t last_access_date;     // 18
    uint16_t high_cluster;         // 20 (siempre 0 en FAT12 y FAT16)
    uint16_t write_time;           // 22
    uint16_t write_date;           // 24
    uint16_t first_cluster;        // 26  <---- ¡AQUÍ ESTÁ EL PROBLEMA!
    uint32_t file_size;            // 28
} __attribute__((packed)) FAT12_DirEntry;

typedef struct _FAT12_BootSector {
    unsigned char jump[3];
    char oem[8];
    unsigned short bytes_per_sector;
    unsigned char sectors_per_cluster;
    unsigned short reserved_sectors;
    unsigned char num_fats;
    unsigned short root_entries;
    unsigned short total_sectors;
    unsigned char media;
    unsigned short fat_size_sectors;
    unsigned short sectors_per_track;
    unsigned short num_heads;
    unsigned int hidden_sectors;
    unsigned int total_sectors_large;
} __attribute__((packed)) FAT12_BootSector;

typedef struct {
    unsigned short sector_num;
    unsigned short pos;
} FileAddress;

typedef struct _FatFile {
    struct _FAT12_BootSector* bs;
    struct _FAT12_DirEntry sector;
    struct _FAT12_DirEntry* dir;
} FatFile;

unsigned short get_fat_entry(unsigned short cluster, unsigned char* fat) {
    unsigned int index = cluster + (cluster / 2);
    unsigned short value = *(unsigned short*)&fat[index];
    if (cluster & 1) value >>= 4;
    else value &= 0x0FFF;
    return value;
}

FileAddress InternalFat12GetFileIndex(FatFile File)
{
    FileAddress addr;
    struct _FAT12_BootSector* bs = File.bs;
    struct _FAT12_DirEntry* entry = &File.sector;

    // Calcular tamaño del directorio raíz en sectores
    unsigned short root_dir_sectors = ((bs->root_entries * 32) + (bs->bytes_per_sector - 1)) / bs->bytes_per_sector;

    // Calcular inicio de la región de datos
    unsigned short data_region_start = bs->reserved_sectors + (bs->num_fats * bs->fat_size_sectors) + root_dir_sectors;

    // Primer cluster del archivo
    unsigned short first_cluster = entry->first_cluster;

    // Calcular sector físico
    addr.sector_num = data_region_start + (first_cluster - 2) * bs->sectors_per_cluster;
    addr.pos = 0; // inicio del sector

    return addr;
}
