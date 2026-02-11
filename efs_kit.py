from sys import argv
import math

# primer sector que debe de ser un disco real
sector_first_0 = b'\xeb<\x90mkfs.fat\x00\x02\x04\x01\x00\x02\x00\x02\x00\x08\xf8\x02\x00\x10\x00\x02\x00\x00\x00\x00\x00\x00\x00\x00\x00\x80\x00);\x7f:9NO NAME    ErickFS \x0e\x1f\xbe[|\xac"\xc0t\x0bV\xb4\x0e\xbb\x07\x00\xcd\x10^\xeb\xf02\xe4\xcd\x16\xcd\x19\xeb\xfeThis is not a bootable disk.  Please insert a bootable floppy and\r\npress any key to try again ... \r\n\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00U\xaa'

# typedef struct _ErickFileEntry {
#    unsigned int FileNamePtr;
#    unsigned int FileContentPtr;
#    unsigned int FileContentSize;
#    unsigned int FilePluginsPtr;
# } ErickFileEntry;
class ErickFileEntry:
    # el constructor
    def __init__(self,FileNamePtr:int,FileContentPtr:int,FileContentSize:int,FilePluginsPtr:int):
        self.FileNamePtr = FileNamePtr
        self.FileContentPtr = FileContentPtr
        self.FileContentSize = FileContentSize
        self.FilePluginsPtr = FilePluginsPtr
    # convertir a binario
    def to_bytes(self) -> bytes:
        return bytes(
            self.FileNamePtr.to_bytes(4, "little") +
            self.FileContentPtr.to_bytes(4, "little") +
            self.FileContentSize.to_bytes(4, "little") +
            self.FilePluginsPtr.to_bytes(4, "little")
            )

# typedef struct _ErickFsFirstSector {
#     unsigned short TotalFiles;
#     unsigned short TotalSectors;
#     unsigned short TableStartSector;
# } ErickFsFirstSector;
class ErickFsFirstSector:
    # la entrada
    def __init__(self,TotalFiles:int,TotalSectors:int,TableStartSector:int):
        self.TotalFiles = TotalFiles
        self.TotalSectors = TotalSectors
        self.TableStartSector = TableStartSector
    # convertir a bytes
    def to_bytes(self):
        table_struct = bytes(self.TotalFiles.to_bytes(2, "little") + self.TotalSectors.to_bytes(2, "little") + self.TableStartSector.to_bytes(2, "little"))
        return bytes(bytearray(table_struct) + bytearray(512 - len(table_struct)))

# clase para funciones de hacer sistemas de
# archivos con el formato ErickFS, un fat, mas
# rapido y mas amigable para lectura y escritura
class ErickFSLib:
    # no hace nada
    def __init__(self): pass
    # cuenta las entradas de un config
    @staticmethod
    def cfg_entrys_parse(cfg: str) -> list[str]:
        array = cfg.replace("\r", "").split("\n")
        returned: list[str] = []
        for entry in array:
            entry_internal = entry.strip()
            if not (entry_internal == ""):returned.append(entry_internal)
        return returned
    # hace un sistema de archivos
    @staticmethod
    def mkfs(cfg: str, size: int) -> bytes:
        if size < 512: raise Exception("tamaño de disco muy pequeño")
        fs = bytearray(size)
        config = ErickFSLib.cfg_entrys_parse(cfg)
        total_files = len(config)
        sector_size = 512

        first_sector = ErickFsFirstSector(total_files, 0, 2)
        total_sectors_files = math.ceil(total_files / 32)
        start_sector_datas = 2 + total_sectors_files
        start_sector_table = 2

        files_contents_dictionary:dict[str, tuple[int, int]] = dict()
        files_names_dictionary:dict[str, int] = dict()
        actual_sector_write = start_sector_datas

        bug_aling = 0x10

        for entry in config:
            # si la entrada contiene ':', separar ruta y archivo
            if ":" in entry:
                ruta, archivo = entry.split(":", 1)
                file_name = ruta.strip()
                file_path = archivo
            else:
                file_path = entry.strip()
                file_name = file_path

            # leer contenido real del archivo
            file_content = open(file_path, "rb").read()
            files_contents_dictionary[file_name] = (actual_sector_write * sector_size, len(file_content))

            # dividir en bloques de 512
            for offset in range(0, len(file_content), sector_size):
                block = file_content[offset:offset+sector_size]
                fs[(actual_sector_write*sector_size)-bug_aling:
                ((actual_sector_write+1)*sector_size)-bug_aling] = bytearray(block) + bytearray(sector_size - len(block))
                actual_sector_write += 1

            # guardar nombre (con ruta incluida)
            name_bytes = file_name.encode()
            files_names_dictionary[file_name] = actual_sector_write * sector_size
            fs[(actual_sector_write*sector_size)-bug_aling:
            ((actual_sector_write+1)*sector_size)-bug_aling] = bytearray(name_bytes) + bytearray(sector_size - len(name_bytes))
            actual_sector_write += 1

        put_entry = 0
        put_sector = start_sector_table
        for direction,name in enumerate(files_contents_dictionary):
            file_conv = ErickFileEntry(files_names_dictionary[name], files_contents_dictionary[name][0], files_contents_dictionary[name][1], 0)
            print(file_conv.to_bytes())
            fs[put_sector*sector_size:(put_sector*sector_size)+(put_entry*16)] = file_conv.to_bytes()
            put_entry += 1
            if (put_entry == 32):
                put_sector += 1
                put_entry = 0

        fs[0:sector_size*1] = bytearray(sector_first_0[0:512])
        fs[sector_size*1:sector_size*2] = bytearray(first_sector.to_bytes())

        return bytes(fs)

params = argv[1:]

param_index = 0
while param_index < len(params):
    param = params[param_index]
    if param == "-porfile":
        param_index += 1
        param = params[param_index]
        if param == "new":
            param_index += 1
            param = params[param_index]
            param_index += 1
            size = params[param_index]
            open(param + ".fsp", "w").write("")
            open(param + ".fss", "w").write(size)
        elif param == "add":
            param_index += 1
            param = params[param_index]
            param_index += 1
            add = params[param_index]
            open(param + ".fsp", "a").write(add + "\n")
        elif param == "build":
            param_index += 1
            param = params[param_index]
            open(param + ".img", "wb").write(ErickFSLib.mkfs(open(param + ".fsp", "r").read(), int(open(param + ".fss", "r").read())))
    param_index += 1