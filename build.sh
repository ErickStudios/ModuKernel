# función: compile_raw <archivo.c> <archivo.bin>
compile_raw() {
    local src="$1"
    local out="$2"

    # compilar a objeto de 32 bits, sin libc, sin PIC
    gcc -m32 -ffreestanding -fno-pic -nostdlib -c "$src" -o temp.o

    # linkear a binario plano con símbolo Main
    ld -m elf_i386 -Ttext 0x1000 --oformat binary temp.o -o "$out"

    rm -f temp.o
}


# ensamblar y compilar kernel
nasm -f elf32 kernel.asm -o kasm.o
gcc -m32 -c kernel.c -o kc.o

# compilar programa de usuario a binario crudo
mkdir -p disk
compile_raw programs/hello_world.c disk/KERNEL.BIN
read e
# linkear kernel
ld -m elf_i386 -T link.ld -o kernel kasm.o kc.o

# crear disco y formatear
qemu-img create -f raw disk.img 10M
mkfs.fat -F 12 disk.img

# montar y copiar archivo
sudo mkdir -p /mnt/disk_qemu
sudo mount -o loop disk.img /mnt/disk_qemu
sudo cp disk/KERNEL.BIN /mnt/disk_qemu/
ls -l /mnt/disk_qemu

sudo umount /mnt/disk_qemu

# arrancar QEMU
qemu-system-i386 -kernel ./kernel -hda disk.img
