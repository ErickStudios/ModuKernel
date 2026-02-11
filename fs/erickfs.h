#ifndef ErickFsDotH
#define ErickFsDotH
typedef struct _ErickFileEntry {
    unsigned int FileNamePtr;
    unsigned int FileContentPtr;
    unsigned int FileContentSize;
    unsigned int FilePluginsPtr;
} ErickFileEntry;

typedef struct _ErickFsFirstSector {
    unsigned short TotalFiles;
    unsigned short TotalSectors;
    unsigned short TableStartSector;
} ErickFsFirstSector;
#endif