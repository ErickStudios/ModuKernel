#ifndef ErickFsDotH
#define ErickFsDotH
/** una entrada de ErickFS */
typedef struct _ErickFileEntry {
    /** puntero al nombre del archivo dentro del disco, para obtener
     * el sector se usa (Ptr / 512) y para obtener el offset se usa
     * (Ptr % 512)*/
    unsigned int FileNamePtr;
    /** puntero al contenido del archivo dentro del disco, para obtener
     * el sector se usa (Ptr / 512) y para obtener el offset se usa
     * (Ptr % 512)*/
    unsigned int FileContentPtr;
    /** el tamaño del archivo en disco, indica los bytes para obtener los
     * sectores se usa (Size / 512) */
    unsigned int FileContentSize;
    /** puntero a los plugins del archivo dentro del disco, para obtener
     * el sector se usa (Ptr / 512) y para obtener el offset se usa
     * (Ptr % 512) esto sirve para extender el archivo y añadirle funciones
     * o una interpretacion */
    unsigned int FilePluginsPtr;
} ErickFileEntry;
/** representa el contenido del sector 1 */
typedef struct  _ErickFsFirstSector {
    /** total de archivos en el disco */
    unsigned short TotalFiles;
    /** total de sectores en el disco */
    unsigned short TotalSectors;
    /** el sector donde inicia la tabla de archivos */
    unsigned short TableStartSector;
    /** tablas de archivos por sector */
    unsigned short TablesPerSector;
    /** primer sector libre */
    unsigned int FirstFreeSector;
    /** tamaño del codigo init en sectores */
    unsigned int InitCodeSizeBySectors;
    /** donde empieza el codigo de inicializacion */
    unsigned int InitCodeSector;
} ErickFsFirstSector;
#endif