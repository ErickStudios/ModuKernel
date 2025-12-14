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
compile_raw programs/hello_world.c disk/KERNEL.BIN
compile_raw programs/calculator.c disk/CALC.BIN
compile_raw drivers/drv0.c disk/DRV0IN.BIN
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
for file in disk/*; do
    sudo cp "$file" /mnt/disk_qemu/ && echo "$file copiado"
done

ls -l /mnt/disk_qemu

sudo umount /mnt/disk_qemu

# arrancar QEMU
qemu-system-i386 -kernel ./build/kernel -hda ./build/disk.img -m 512M
