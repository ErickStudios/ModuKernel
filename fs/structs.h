#include "erickfs.h"
#include "fstype.h"
#include <stdint.h>

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

typedef struct _FatFileLocation {
    unsigned int SectorStart;
    unsigned int Offset;
} FatFileLocation;

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
    char* ErickFSFileName;
    FsType type;
    struct _ErickFileEntry ErickFSentry;
} FatFile;