typedef struct _FAT12_DirEntry {
    char name[8];
    char ext[3];
    unsigned char attr;
    unsigned char reserved[10];
    unsigned short time;
    unsigned short date;
    unsigned short first_cluster;
    unsigned int file_size;
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

typedef struct _FatFile {
    struct _FAT12_BootSector* bs;
    struct _FAT12_DirEntry sector;
} FatFile;

unsigned short get_fat_entry(unsigned short cluster, unsigned char* fat) {
    unsigned int index = cluster + (cluster / 2);
    unsigned short value = *(unsigned short*)&fat[index];
    if (cluster & 1) value >>= 4;
    else value &= 0x0FFF;
    return value;
}
