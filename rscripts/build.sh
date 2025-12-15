# función: compile_raw <archivo.c> <archivo.bin>
cd ..

compile_raw() {
    local src="$1"
    local out="$2"

    gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib -c "$src" -o temp.o
    ld -m elf_i386 -T ABI/user_link.ld --oformat binary temp.o -o "$out"
    rm -f temp.o
}

# ensamblar kernel (etapa pre-premature boot init)
nasm -f elf32 assembly/kernel.asm -o build/kasm.o
# compilar kernel (etapas post-premature boot init)
gcc -m32 -c kernel.c -o build/kc.o

# crear disco
mkdir -p disk
# linkear kernel
ld -m elf_i386 -T ABI/kernel_link.ld -o build/kernel build/kasm.o build/kc.o

# eliminar todo de disk
rm -rf disk/*
# crear el mapa del sistema de archivos extendido
echo "" > disk/FSLST.IFS

# el contador
afaf=0
# para los programas
for file in programs/*.c; do
    # extraer el nombre
    nameq=$(basename "$file")
    name="${nameq%.*}"
    # compilar
    compile_raw $file disk/BC$afaf.BIN
    # añadir al mapa
    echo "/bin/$name;BC$afaf;BIN;" >> disk/FSLST.IFS
    # siguiente
    let afaf=afaf+1
done

# setear a 0
afaf=0
# para los drivers
for file in drivers/*.c; do
    # extraer el nombre
    nameq=$(basename "$file")
    name="${nameq%.*}"
    # compilar
    compile_raw $file disk/DV$afaf.BIN
    # añadir al mapa
    echo "/dev/$name;DV$afaf;BIN;" >> disk/FSLST.IFS
    # siguiente
    let afaf=afaf+1
done

# setear a 0
afaf=0
# para el usuario
for file in usr/*; do
    # extraer el nombre
    name=$(basename "$file")
    # compilar
    cp $file disk/SR$afaf.SRN
    # añadir al mapa
    echo "/home/$name;SR$afaf;SRN;" >> disk/FSLST.IFS
    # siguiente
    let afaf=afaf+1
done

# hacer la carpeta de monteo
sudo mkdir -p /mnt/disk_qemu
# crear imagen
qemu-img create -f raw  build/disk.img 10M
# formatearla
mkfs.fat -F 12  build/disk.img
# montarla
sudo mount -o loop  build/disk.img /mnt/disk_qemu

# todos los archivos que se pasaran
for file in disk/*; do 
    # copiarlo
    sudo cp "$file" /mnt/disk_qemu/ && echo "$file copiado"
done

# desmontar disco
sudo umount /mnt/disk_qemu

# arrancar QEMU
qemu-system-i386            \
    -kernel ./build/kernel  \
    -hda build/disk.img     \
    -m 512M
