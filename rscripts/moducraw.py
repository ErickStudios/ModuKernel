import sys
import struct

input_header = sys.argv[1]
output_modubmp = sys.argv[2]

class Uint32String:
    def __init__(self, value: int):
        if not (0 <= value <= 0xFFFFFFFF):
            raise ValueError("Fuera de rango uint32_t")
        self.value = value

    def to_bytes(self) -> bytes:
        return struct.pack(">I", self.value)

class ImageHeader:
    def __init__(self, ImageSize: int, SizeX: int, SizeY: int):
        self.imgs = Uint32String(ImageSize).to_bytes()
        self.sx   = Uint32String(SizeX).to_bytes()
        self.sy   = Uint32String(SizeY).to_bytes()

    def joinimage(self, raw_data: bytes) -> bytes:
        return self.imgs + self.sx + self.sy + raw_data

# Leer archivo de cabecera
with open(input_header, "r") as file:
    imgheaderlines = file.readlines()

# Buscar marcas de inicio y fin
img_abc_start = None
img_abc_end = None

for i, line in enumerate(imgheaderlines):
    if line.strip().startswith("//!start_pymoducraw"):
        img_abc_start = i + 1  # la línea siguiente al marcador
    if line.strip().startswith("//!end_pymoducraw"):
        img_abc_end = i
        break

if img_abc_start is None or img_abc_end is None:
    raise RuntimeError("No se encontraron los marcadores en el archivo")

# Extraer bloque de datos
data_lines = imgheaderlines[img_abc_start:img_abc_end]

# Convertir a bytes (ejemplo: cada línea tiene valores hex/bin separados por comas)
raw_bytes = bytearray()
for line in data_lines:
    line = line.strip()
    if not line or line.startswith("//"):
        continue
    for token in line.split(","):
        token = token.strip()
        if token:
            raw_bytes.append(int(token, 0))  # interpreta 0x.. o 0b.. o decimal

# Crear header + datos
header = ImageHeader(len(raw_bytes), 70, 10)  # ejemplo de SizeX, SizeY
blob = header.joinimage(bytes(raw_bytes))

# Guardar archivo binario
with open(output_modubmp, "wb") as out:
    out.write(blob)

print(f"Archivo {output_modubmp} generado con éxito.")