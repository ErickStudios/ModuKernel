# primer sector que debe de ser un disco real
sector_first_0 = open("efskit/efs_kit.bin", "rb").read()

# incluir la libreria del sistema
from sys import argv
# incluir libreria de matematicas
import math

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
    def mkfs(cfg: str, size: int) -> bytes:
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

        # sectores totales
        total_sectors_files = math.ceil(total_files / 32)
        # donde inicia los datos del sector
        start_sector_datas = 2 + total_sectors_files
        # donde inicia la tabla
        start_sector_table = 2

        # contenidos de los archivos
        files_contents_dictionary:dict[str, tuple[int, int]] = dict()
        # los nombres de los archivos
        files_names_dictionary:dict[str, int] = dict()
        # el sector actaul a escribir
        actual_sector_write = start_sector_datas

        # alineacion por bug
        bug_aling = 0

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
        first_sector = ErickFsFirstSector(total_files, 0, 2, 32, actual_sector_write, 0,0)
        # añadir el sector 0
        fs[0:sector_size*1] = bytearray(sector_first_0[0:512])
        # el sector 1
        fs[sector_size*1:sector_size*2] = bytearray(first_sector.to_bytes())

        # el fs
        return bytes(fs)

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
                # crear perfil
                open(param + ".fsp", "w").write("")
                # crear tamaño
                open(param + ".fss", "w").write(size)
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
                open(param + ".img", "wb").write(ErickFSLib.mkfs(open(param + ".fsp", "r").read()[0:-1], int(open(param + ".fss", "r").read())))
        # siguiente
        param_index += 1

if __name__ == "__main__":
    main()