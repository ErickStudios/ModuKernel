# función: compile_raw <archivo.c> <archivo.bin>
cd ..

compile_raw() {
    local src="$1"
    local out="$2"

    gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib -c "$src" -o temp.o
    ld -m elf_i386 -T ABI/user_link.ld --oformat binary temp.o -o "$out"
    rm -f temp.o
}

# ensamblar y compilar kernel
nasm -f elf32 assembly/kernel.asm -o build/kasm.o
gcc -m32 -c kernel.c -o build/kc.o

# compilar programa de usuario a binario crudo
mkdir -p disk
read e
# linkear kernel
ld -m elf_i386 -T ABI/kernel_link.ld -o build/kernel build/kasm.o build/kc.o

# crear disco y formatear
qemu-img create -f raw  build/disk.img 10M
mkfs.fat -F 12  build/disk.img

# montar y copiar archivo
sudo mkdir -p /mnt/disk_qemu
sudo mount -o loop  build/disk.img /mnt/disk_qemu
sudo cp disk/KERNEL.BIN /mnt/disk_qemu/

afaf=0
filea=""

# eliminar todo de disk
rm -rf disk/*

# el sistema de archivos
echo "" > disk/FSLST.IFS

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

echo $filea

for file in disk/*; do 
    sudo cp "$file" /mnt/disk_qemu/ && echo "$file copiado"
done

ls -l /mnt/disk_qemu

sudo umount /mnt/disk_qemu

cp build/kernel iso/boot/kernel

grub-mkrescue -o build/ModuKernel.iso iso

# arrancar QEMU
#qemu-system-i386 -cdrom build/ModuKernel.iso -m 512M 
    # no funciona
qemu-system-i386 -kernel ./build/kernel -hda build/disk.img -m 512M
#-kernel ./build/kernel  -hda build/disk.img -m 512M