#ifndef _KernelDiskUnitiesDotH_
#define _KernelDiskUnitiesDotH_
/* enumeracion para el disco que se quiere leer */
typedef enum _DiskTypePort
{
    /* disco duro */
    DiskTypeHardDisk,
    /* disco floppy */
    DiskTypeFloppy,
    /* disco CD-ROM */
    DiskTypeCdRom,
} DiskTypePort;
#endif