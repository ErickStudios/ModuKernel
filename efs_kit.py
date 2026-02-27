# incluir el os
import os
# incluir la libreria del sistema
from sys import argv
# incluir libreria de matematicas
import math
# carpeta
from pathlib import Path
# primer sector que debe de ser un disco real
sector_first_0 = b"\xEB\x60\x90\x45\x66\x73\x4B\x69\x74\x50\x79\x00\x02\x01\x01\x00\x02\xE0\x00\x40\x0B\xF0\x09\x00\x12\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x41\x00\x00\x00\x00\x45\x72\x69\x63\x6B\x46\x53\x4B\x69\x74\x20\x45\x72\x69\x63\x6B\x46\x53\x20\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x10\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\xFA\x31\xC0\x8E\xD8\x8E\xC0\x8E\xD0\xBC\x00\x7C\x88\x16\x24\x7C\xB4\x02\xB0\x01\xB5\x00\xB1\x02\xB6\x00\x8A\x16\x24\x7C\xBB\x00\x06\xCD\x13\x72\x73\xBE\x00\x06\xAD\xA3\x3E\x7C\xAD\xA3\x40\x7C\xAD\xA3\x42\x7C\xAD\xA3\x44\x7C\xAD\xA3\x46\x7C\xAD\xA3\x48\x7C\xAD\xA3\x4A\x7C\xAD\xA3\x4C\x7C\xAD\xA3\x4E\x7C\xAD\xA3\x50\x7C\xA1\x4A\x7C\xA3\x54\x7C\xC7\x06\x56\x7C\x00\x80\xC7\x06\x58\x7C\x00\x00\xA1\x4E\x7C\xA3\x5A\x7C\xA1\x50\x7C\xA3\x5C\x7C\xC7\x06\x5E\x7C\x00\x00\xC7\x06\x60\x7C\x00\x00\x1E\x31\xC0\x8E\xD8\xBE\x52\x7C\x8A\x16\x24\x7C\xB4\x42\xCD\x13\x72\x0C\x1F\x8A\x16\x24\x7C\x9A\x00\x80\x00\x00\xEB\xFE\xB0\x21\xB4\x0E\xCD\x10\xEB\xFE\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x55\xAA"
# Carpeta donde está el script actual
BASE_DIR = Path(__file__).resolve().parent
# carpeta
file_path = BASE_DIR / "efskit" / "efs_kit.bin"
# el archivo
if file_path.exists(): sector_first_0 = file_path.read_bytes()

# para llenar un espacio sin que sean los aburridos 0
# de relleno
#
# indica una longitud y rellena esa longitud con DEADBEEF
# y si queda mas espacio la rellena con 0xAF
#
# esto sirve para algunas cosas
def fill(length: int) -> bytearray:
    buf = bytearray(length)
    seq = (0xDEADBEEF).to_bytes(4, byteorder="big")
    copy_len = min(length, len(seq))
    buf[0:copy_len] = seq[0:copy_len]
    if length > copy_len: buf[copy_len:] = b'\xAF' * (length - copy_len)
    return buf

# utilidades para obtener cosas del disco con un bytes
# 
# esta clase proporciona funciones para dividir sectores
# y mas de una manera facil y entendible
#
# ejemplos:
#
# sectors = DiskBinUtils.split_sectors(diskRaw)
class DiskBinUtils:
    @staticmethod
    def split_sectors(data: bytes) -> list[bytes]:
        return [data[i:i+512] for i in range(0, len(data), 512)]

# formatea un byte
# Convierte un string con sufijo opcional (K, M, G) a bytes.
# Ejemplos:
# "512"  -> 512
# "2K"   -> 2048
# "3M"   -> 3145728
# "1G"   -> 1073741824
class FormatsBytes:
    # parsea
    @staticmethod
    def parse(value: str) -> int:
        value = value.strip().upper()

        if value.endswith("K"):
            return int(value[:-1]) * 1024
        elif value.endswith("M"):
            return int(value[:-1]) * 1024 * 1024
        elif value.endswith("G"):
            return int(value[:-1]) * 1024 * 1024 * 1024
        else:
            return int(value)  # sin sufijo, se interpreta como bytes crudos

# /** una entrada de ErickFS */
# typedef struct _ErickFileEntry {
#     /** puntero al nombre del archivo dentro del disco, para obtener
#      * el sector se usa (Ptr / 512) y para obtener el offset se usa
#      * (Ptr % 512)*/
#     unsigned int FileNamePtr;
#     /** puntero al contenido del archivo dentro del disco, para obtener
#      * el sector se usa (Ptr / 512) y para obtener el offset se usa
#      * (Ptr % 512)*/
#     unsigned int FileContentPtr;
#     /** el tamaño del archivo en disco, indica los bytes para obtener los
#      * sectores se usa (Size / 512) */
#     unsigned int FileContentSize;
#     /** puntero a los plugins del archivo dentro del disco, para obtener
#      * el sector se usa (Ptr / 512) y para obtener el offset se usa
#      * (Ptr % 512) esto sirve para extender el archivo y añadirle funciones
#      * o una interpretacion */
#     unsigned int FilePluginsPtr;
# } ErickFileEntry;
class ErickFileEntry:
    # el constructor
    def __init__(self,FileNamePtr:int,FileContentPtr:int,FileContentSize:int,FilePluginsPtr:int):
        # el puntero al nombre del archivo
        self.FileNamePtr = FileNamePtr
        # el puntero al contenido del archivo
        self.FileContentPtr = FileContentPtr
        # el tamaño del archivo
        self.FileContentSize = FileContentSize
        # el puntero a los plugins del archivo
        self.FilePluginsPtr = FilePluginsPtr
    # convertir a binario
    def to_bytes(self) -> bytes: 
        # lo convierte
        return bytes(self.FileNamePtr.to_bytes(4, "little") +self.FileContentPtr.to_bytes(4, "little") +self.FileContentSize.to_bytes(4, "little") +self.FilePluginsPtr.to_bytes(4, "little"))
    # convertir desde bytes
    @classmethod
    def from_bytes(cls, data: bytes):
        if len(data) < 16:
            raise ValueError("Se requieren al menos 16 bytes para reconstruir ErickFileEntry")

        FileNamePtr = int.from_bytes(data[0:4], "little")
        FileContentPtr = int.from_bytes(data[4:8], "little")
        FileContentSize = int.from_bytes(data[8:12], "little")
        FilePluginsPtr = int.from_bytes(data[12:16], "little")

        return cls(FileNamePtr, FileContentPtr, FileContentSize, FilePluginsPtr)

# /** representa el contenido del sector 1 */
# typedef struct _ErickFsFirstSector {
#     /** total de archivos en el disco */
#     unsigned short TotalFiles;
#     /** total de sectores en el disco */
#     unsigned short TotalSectors;
#     /** el sector donde inicia la tabla de archivos */
#     unsigned short TableStartSector;
#     /** tablas de archivos por sector */
#     unsigned short TablesPerSector;
#     /** primer sector libre */
#     unsigned int FirstFreeSector;
#     /** tamaño del codigo init en sectores */
#     unsigned int InitCodeSizeBySectors;
#     /** donde empieza el codigo de inicializacion */
#     unsigned int InitCodeSector;
# } ErickFsFirstSector;
class ErickFsFirstSector:
    # la entrada
    def __init__(self,TotalFiles:int,TotalSectors:int,TableStartSector:int,TablesPerSector:int,FirstFreeSector:int,InitCodeSizeBySectors:int,InitCodeSector:int):
        # numero de archivos totales
        self.TotalFiles = TotalFiles
        # numero de sectores totales
        self.TotalSectors = TotalSectors
        # el sector donde inicia la tabla de archivos
        self.TableStartSector = TableStartSector
        # el numero de tablas por sector
        self.TablesPerSector = TablesPerSector
        # el primer sector libre
        self.FirstFreeSector = FirstFreeSector
        # el tamaño del codigo de inicializacion en sectores
        self.InitCodeSizeBySectors = InitCodeSizeBySectors
        # el sector donde inicia el codigo de inicializacion
        self.InitCodeSector = InitCodeSector
    # convertir a bytes
    def to_bytes(self):
        # tabla de estructura
        table_struct = bytes(self.TotalFiles.to_bytes(2, "little") + self.TotalSectors.to_bytes(2, "little") + self.TableStartSector.to_bytes(2, "little") + self.TablesPerSector.to_bytes(2, "little") + self.FirstFreeSector.to_bytes(4, "little") + self.InitCodeSizeBySectors.to_bytes(4, "little") + self.InitCodeSector.to_bytes(4, "little"))
        # la tabla + relleno
        return bytes(bytearray(table_struct) + fill(512 - len(table_struct)))
    # convertir desde bytes
    @classmethod
    def from_bytes(cls, data: bytes):
        TotalFiles = int.from_bytes(data[0:2], "little")
        TotalSectors = int.from_bytes(data[2:4], "little")
        TableStartSector = int.from_bytes(data[4:6], "little")
        TablesPerSector = int.from_bytes(data[6:8], "little")
        FirstFreeSector = int.from_bytes(data[8:12], "little")
        InitCodeSizeBySectors = int.from_bytes(data[12:16], "little")
        InitCodeSector = int.from_bytes(data[16:20], "little")

        return cls(TotalFiles, TotalSectors, TableStartSector,
                   TablesPerSector, FirstFreeSector,
                   InitCodeSizeBySectors, InitCodeSector)
    # lo convierte a string
    def __str__(self):
        return (
            f"(\n"
            f"  TotalFiles={self.TotalFiles},\n"
            f"  TotalSectors={self.TotalSectors},\n"
            f"  TableStartSector={self.TableStartSector},\n"
            f"  TablesPerSector={self.TablesPerSector},\n"
            f"  FirstFreeSector={self.FirstFreeSector},\n"
            f"  InitCodeSizeBySectors={self.InitCodeSizeBySectors},\n"
            f"  InitCodeSector={self.InitCodeSector}\n"
            f")"
        )
    
# clase para funciones de hacer sistemas de
# archivos con el formato ErickFS, un fat, mas
# rapido y mas amigable para lectura y escritura
#
# Estructura teorica:
#
# DISCO
# |-> Sector de arranque
# |-> Sector de informacion del filesystem
# |   |-> TotalFiles
# |   |-> TotalSectors
# |   |-> TableStartSector
# |   |-> TablesPerSector
# |   |-> FirstFreeSector
# |   |-> InitCodeSizeBySectors
# |   |-> InitCodeSector
# |-> ...Informacion de usuario...
# |   |->x32 tablas
# |     |-> FileNamePtr
# |     |-> FileContentPtr
# |     |-> FileContentSize
# |     |-> FilePluginsPtr
class ErickFSLib:
    # no hace nada
    def __init__(self): pass
    # cuenta las entradas de un config
    @staticmethod
    def cfg_entrys_parse(cfg: str) -> list[str]:
        # el array
        array = cfg.replace("\r", "").split("\n")
        # la lista
        returned: list[str] = []
        # la recorre
        for entry in array:
            # la entrada
            entry_internal = entry.strip()
            # añada la entrada
            if not (entry_internal == ""):returned.append(entry_internal)
        return returned
    # hace un sistema de archivos
    @staticmethod
    def mkfs(cfg: str, size: int, init: bytes) -> bytes:
        # el disco es pequeño
        if size < 512: raise Exception("tamaño de disco muy pequeño")

        # el fs
        fs = bytearray(size)
        # configuracion
        config = ErickFSLib.cfg_entrys_parse(cfg)
        # los archivos totales
        total_files = len(config)
        # tamaño de un sector
        sector_size = 512

        # el tamaño de init en sectores
        init_size_sectors = math.ceil(len(init) / sector_size)
        # donde inicia el init
        start_sector_init = 2
        # sectores totales
        total_sectors_files = math.ceil(total_files / 32)
        # donde inicia la tabla
        start_sector_table = start_sector_init + init_size_sectors
        # donde inicia los datos del sector
        start_sector_datas = start_sector_table + total_sectors_files + int((size / 512) * 0.064)

        # contenidos de los archivos
        files_contents_dictionary:dict[str, tuple[int, int]] = dict()
        # los nombres de los archivos
        files_names_dictionary:dict[str, int] = dict()
        # el sector actaul a escribir
        actual_sector_write = start_sector_datas

        # alineacion por bug
        bug_aling = 0


        actual_sector_write = start_sector_datas

        # Escribir init
        for offset in range(0, len(init), sector_size):
            block = init[offset:offset+sector_size]
            fs[(start_sector_init*sector_size) + offset:
            (start_sector_init*sector_size) + offset + sector_size] = \
                bytearray(block) + bytearray(sector_size - len(block))

        # procesa las entradas
        for entry in config:
            # si contiene : es por que se crea un alias
            if ":" in entry:
                # la ruta y el archivo
                ruta, archivo = entry.split(":", 1)
                # el archivo
                file_name = ruta.strip()
                # la ruta
                file_path = archivo
            # si no
            else:
                # el archivo
                file_path = entry.strip()
                # la ruta
                file_name = file_path

            # leer contenido real del archivo
            file_content = open(file_path, "rb").read()
            # añadir al los contenidos de archivo
            files_contents_dictionary[file_name] = (actual_sector_write * sector_size, len(file_content))

            # dividir en bloques de 512
            for offset in range(0, len(file_content), sector_size):
                # el bloque
                block = file_content[offset:offset+sector_size]
                # añadir al fs
                fs[(actual_sector_write*sector_size)-bug_aling:((actual_sector_write+1)*sector_size)-bug_aling] = bytearray(block) + bytearray(sector_size - len(block))
                # sector actual a escribir
                actual_sector_write += 1

            # guardar nombre (con ruta incluida)
            name_bytes = file_name.encode()
            # los nombres de archivos
            files_names_dictionary[file_name] = actual_sector_write * sector_size
            # escribir el nombre
            fs[(actual_sector_write*sector_size)-bug_aling:((actual_sector_write+1)*sector_size)-bug_aling] = bytearray(name_bytes) + bytearray(sector_size - len(name_bytes))
            # sector actual a escribir
            actual_sector_write += 1

        # entrdad a poner
        put_entry = 0
        # sector a poner
        put_sector = start_sector_table
        # recorrer nombres
        for direction,name in enumerate(files_contents_dictionary):
            # archivo convertido
            file_conv = ErickFileEntry(
                files_names_dictionary[name], 
                files_contents_dictionary[name][0], 
                files_contents_dictionary[name][1], 0
                )
            # añadir al fs
            fs[(put_sector*sector_size)+(put_entry*16):(put_sector*sector_size)+((put_entry+1)*16)] = file_conv.to_bytes()
            # entrada a poner
            put_entry += 1
            # si es mayor
            if (put_entry == 32):
                # sector a poner
                put_sector += 1
                # entrada a poner
                put_entry = 0

        # primer sector de ErickFS
        first_sector = ErickFsFirstSector(total_files, size // 512, start_sector_table, 32, actual_sector_write, init_size_sectors, start_sector_init)
        # añadir el sector 0
        fs[0:sector_size*1] = bytearray(sector_first_0[0:512])
        # el sector 1
        fs[sector_size*1:sector_size*2] = bytearray(first_sector.to_bytes())

        # el fs
        return bytes(fs)

# DISCO
# |-> Sector de arranque
# |-> Sector de informacion del filesystem
# |   |-> TotalFiles
# |   |-> TotalSectors
# |   |-> TableStartSector
# |   |-> TablesPerSector
# |   |-> FirstFreeSector
# |   |-> InitCodeSizeBySectors
# |   |-> InitCodeSector
# |-> ...Informacion de usuario...
# |   |->x32 tablas
# |     |-> FileNamePtr
# |     |-> FileContentPtr
# |     |-> FileContentSize
# |     |-> FilePluginsPtr
#
# descomprimido
class ErickFsInfo:
    # genera un fs
    @staticmethod
    def from_bytes(fs: bytes) -> tuple[list[tuple[str,int,bytes]], ErickFsFirstSector]:
        sectors = DiskBinUtils.split_sectors(fs)
        info_sector = sectors[1]
        info = ErickFsFirstSector.from_bytes(info_sector)
        file_table_sectors = sectors[info.TableStartSector:]

        entries:list[tuple[str,int]] = []
        count = 0

        for sector in file_table_sectors:
            for i in range(0, len(sector), 16):
                if count >= info.TotalFiles:
                    break  # ya leímos todas las entradas
                chunk = sector[i:i+16]
                if chunk.strip(b"\x00") == b"":  # ignorar bloques vacíos
                    continue
                entryR = ErickFileEntry.from_bytes(chunk)
                base_name = int(entryR.FileNamePtr / 512)
                offset_name = int(entryR.FileNamePtr % 512)
                base_content = int(entryR.FileContentPtr / 512)
                offset_content = int(entryR.FileContentPtr % 512)
                content = sectors[base_content:base_content + (int(entryR.FileContentSize / 512) + 1)]
                raw = (sectors[base_name])[offset_name:]
                raw_content = b"".join(content)
                raw_content = raw_content[0:entryR.FileContentSize]
                name = raw.decode("utf-8", errors="ignore").split("\x00", 1)[0]
                entry = (name, entryR.FileContentSize, raw_content)
                entries.append(entry)
                count += 1
            if count >= info.TotalFiles:
                break
        return (entries, info)

def main():

    # parametros
    params = argv[1:]

    # el index del parametro
    param_index = 0
    # recorrer parametros
    while param_index < len(params):
        # el parametro
        param = params[param_index]
        # si es acciones de perfil
        if param == "-porfile":
            # siguiente
            param_index += 1
            # accion
            param = params[param_index]
            # nuevo perfil
            if param == "new":
                # siguiente
                param_index += 1
                # el perfil
                param = params[param_index]
                # siguiente
                param_index += 1
                # espacio que tendra
                size = params[param_index]
                # siguiente
                param_index += 1
                # archivo del sector
                file_init_path = params[param_index]
                # crear perfil
                open(param + ".fsp", "w").write("")
                # crear tamañom
                open(param + ".fss", "w").write(size)
                # crear tamaño
                open(param + ".fsi", "w").write(file_init_path)
            # añadir archivo
            elif param == "add":
                # siguiente
                param_index += 1
                # el perfil
                param = params[param_index]
                # siguiente
                param_index += 1
                # el archivo a añadir
                add = params[param_index]
                # añadir la linea
                open(param + ".fsp", "a").write(add + "\n")
            # construir
            elif param == "build":
                # siguiente
                param_index += 1
                # el perfil
                param = params[param_index]
                # lo construye
                open(param + ".img", "wb").write(ErickFSLib.mkfs(open(param + ".fsp", "r").read()[0:-1], FormatsBytes.parse(open(param + ".fss", "r").read()), open(open(param + ".fsi", "r").read(), "rb").read()))
        elif param == "-infomnt":
            param_index += 1
            image = params[param_index]
            img = open(image, "rb").read()
            info_fs = ErickFsInfo.from_bytes(img)
            print("ErickFsFirstSector=" + str(info_fs[1]))
            print("Files = " + str(info_fs[0]))
        # siguiente
        param_index += 1

if __name__ == "__main__":
    main()