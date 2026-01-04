#!/usr/bin/env bash

# ceder a punto punto
cd ..
# e
set -e

# compilar un c
compile_raw() {
    # el archivo
    local src="$1"
    # el outer
    local out="$2"

    # el gcc
    gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib -c "$src" -o temp.o
    # linkear
    ld -m elf_i386 -T ABI/user_link.ld --oformat binary temp.o -o "$out"
    # eliminar el temporal
    rm -f temp.o
}

# compilar assembly
compile_asm()
{
    # el archivo
    local src="$1"
    # el outer
    local out="$2"

    # compilar
    nasm -f elf32 "$src" -o build/tempa.o
    # linkear
    ld -m elf_i386 -T ABI/user_link.ld --oformat binary build/tempa.o -o "$out"
}

# crear el build
mkdir -p build

# compilar el kernel.asm
nasm -f elf32 assembly/kernel.asm -o build/kasm.o
# compilar el kernel.c
gcc -m32 -ffreestanding -fno-stack-protector -nostdlib -c kernel/kernel.c -o build/kc.o
# linkear el kernel
ld -m elf_i386 -T ABI/kernel_link.ld -o build/kernel build/kasm.o build/kc.o

# cedear a modulos
cd modules
# abrir el otro build de modulos
source build.sh

# FS map
rm -rf disk/*
# crear
: > disk/FSLST.IFS

# contador
afaf=0
# bucle
for file in programs/*.c; do
    # nombre
    nameq=$(basename "$file"); name="${nameq%.*}"
    # compilar
    compile_raw "$file" "disk/BC$afaf.BIN"
    # agregar al fsmap
    echo "/bin/$name;BC$afaf;BIN;" >> disk/FSLST.IFS
    # siguiente archivo
    let afaf=afaf+1
done

# bucle
for file in programs/*.asm; do
    # nombre
    nameq=$(basename "$file"); name="${nameq%.*}"
    # compilar
    compile_asm "$file" "disk/BC$afaf.BIN"
    # agregar al fsmap
    echo "/bin/$name;BC$afaf;BIN;" >> disk/FSLST.IFS
    # siguiente archivo
    let afaf=afaf+1
done

# contador
afaf=0
# bucle
for file in build/modules/*.modubin; do
    # nombre
    nameq=$(basename "$file"); name="${nameq%.*}"
    # copiar
    cp "$file" "disk/AC$afaf.BIN"
    # agregar al fsmap
    echo "/kernel/$name;AC$afaf;BIN;" >> disk/FSLST.IFS
    # siguiente archivo
    let afaf=afaf+1
done

# contador
afaf=0
# bucle
for file in build/moddrivers/*.modubin; do
    # nombre
    nameq=$(basename "$file"); name="${nameq%.*}"
    # copiar
    cp "$file" "disk/DV$afaf.BIN"
    # agregar al fsmap
    echo "/dev/$name;DV$afaf;BIN;" >> disk/FSLST.IFS
    # siguiente archivo
    let afaf=afaf+1
done

# bucle
for file in drivers/*.c; do
    # nombre
    nameq=$(basename "$file"); name="${nameq%.*}"
    # compilar
    compile_raw "$file" "disk/DV$afaf.BIN"
    # agregar al fsmap
    echo "/dev/$name;DV$afaf;BIN;" >> disk/FSLST.IFS
    # siguiente archivo
    let afaf=afaf+1
done

# contador
afaf=0
# bucle
for file in modush/*.c; do
    # nombre
    nameq=$(basename "$file"); name="${nameq%.*}"
    # compilar
    compile_raw "$file" "disk/SY$afaf.BIN"
    # agregar al fsmap
    echo "/sys/$name;SY$afaf;BIN;" >> disk/FSLST.IFS
    # siguiente archivo
    let afaf=afaf+1
done

# contador
afaf=0
# bucle
for file in modush/*.sh; do
    # nombre
    nameq=$(basename "$file"); name="${nameq%.*}"
    # copiar
    cp "$file" "disk/SH$afaf.NSH"
    # agregar al fsmap
    echo "/sys/$name.sh;SH$afaf;NSH;" >> disk/FSLST.IFS
    # siguiente archivo
    let afaf=afaf+1
done

# contador
afaf=0
# bucle
for file in usr/*; do
    # nombre
    name=$(basename "$file")
    # copiar
    cp "$file" "disk/SR$afaf.SRN"
    # agregar al fsmap
    echo "/home/$name;SR$afaf;SRN;" >> disk/FSLST.IFS
    # siguiente archivo
    let afaf=afaf+1
done

python rscripts/moducraw.py modules/init/image_array.h disk/LOGOK.BMP
echo "/kernel/logo.bmp;LOGOK;BMP;" >> disk/FSLST.IFS

echo "/fs.struct;FSLST;IFS;" >> disk/FSLST.IFS

# haciendo el fs
echo "making"

# Disk image (explicit raw to avoid warning)
qemu-img create -f raw build/disk.img 1M
# crear el fs
mkfs.fat -F 12 build/disk.img

# crear disco
sudo mkdir -p /mnt/disk_qemu
# montar disco
sudo mount -o loop build/disk.img /mnt/disk_qemu
# copiar todo al disco
sudo cp disk/* /mnt/disk_qemu/
# desmontar el disco
sudo umount /mnt/disk_qemu

# iso de arranque
mkdir -p config/boot/grub
# copiar el kernel a la iso
cp build/kernel config/boot/kernel
# crear configuracion
cat > config/boot/grub/grub.cfg <<'EOF'
# tiempo de espera
set timeout=12
# por defecto
set default=0

# entrada del menu
menuentry "ModuKernel" {
    # multibootear
    multiboot /boot/kernel
    # arrancar
    boot
}

EOF

# crear disco
grub-mkrescue -o build/os.iso config
# formatear el floppy
dd if=/dev/zero of=build/floppy.img bs=1024 count=1440
# crear el floppy
mkfs.fat -F 12 build/floppy.img
# hacer el floppy
mkdir -p /mnt/qemu_floppy
# montar floppy
sudo mount -o loop build/floppy.img /mnt/qemu_floppy
# como plus puedes usar tambien una unidad de floppy disk
sudo cp floppy/* /mnt/qemu_floppy/
# desmontar el floppy
sudo umount /mnt/qemu_floppy

# arranca qemu
qemu-system-i386                                              \
  -cdrom build/os.iso                                         \
  -boot d                                                     \
  -fda build/floppy.img                                       \
  -hda build/disk.img                                         \
  -m 256M                                                     \
  -vga std                                                    \
    -device isa-debugcon,chardev=mimami                       \
    -chardev stdio,id=mimami                                  \
  -audiodev pa,id=snd0,out.frequency=44100,out.channels=2