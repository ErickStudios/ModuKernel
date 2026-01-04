from PIL import Image

def rgb_to_llrrggbb(r, g, b):
    # Reducir cada canal a 2 bits (0–3)
    rr = r // 85
    gg = g // 85
    bb = b // 85

    # Determinar brillo ll según el máximo canal
    max_val = max(r, g, b)
    if max_val < 64:
        ll = 0
    elif max_val < 128:
        ll = 1
    elif max_val < 192:
        ll = 2
    else:
        ll = 3

    # Empaquetar en 8 bits: llrrggbb
    code = (ll << 6) | (rr << 4) | (gg << 2) | bb
    return code

# Abrir imagen BMP en RGB
img = Image.open("output.bmp").convert("RGB")
pixels = list(img.getdata())
width, height = img.size

# Generar array estilo C
with open("image_array.h", "w") as f:
    f.write("unsigned char image[%d] = {\n" % (width*height))
    for i, (r,g,b) in enumerate(pixels):
        code = rgb_to_llrrggbb(r,g,b)
        f.write("0b{0:08b},".format(code))  # escribe en binario
        if (i+1) % width == 0:
            f.write("\n")
    f.write("};\n")