#include "structs.h"

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

unsigned int FatClusterToSector(struct _FAT12_BootSector* bs, unsigned short cluster) {
    unsigned int root_dir_sectors = ((bs->root_entries * 32) + (bs->bytes_per_sector - 1)) / bs->bytes_per_sector;
    unsigned int data_region_start = bs->reserved_sectors + (bs->num_fats * bs->fat_size_sectors) + root_dir_sectors;
    return data_region_start + (cluster - 2) * bs->sectors_per_cluster;
}