#!/usr/bin/env bash
# función: compile_raw <archivo.c> <archivo.bin>
cd ..
set -e
set -x

compile_raw() {
    local src="$1"
    local out="$2"
    gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib -c "$src" -o temp.o
    ld -m elf_i386 -T ABI/user_link.ld --oformat binary temp.o -o "$out"
    rm -f temp.o
}

mkdir -p build

# Kernel
nasm -f elf32 assembly/kernel.asm -o build/kasm.o
gcc -m32 -ffreestanding -fno-stack-protector -nostdlib -c kernel/kernel.c -o build/kc.o
ld -m elf_i386 -T ABI/kernel_link.ld -o build/kernel build/kasm.o build/kc.o

cd modules
source build.sh

# FS map
rm -rf disk/*
: > disk/FSLST.IFS

afaf=0
for file in programs/*.c; do
    nameq=$(basename "$file"); name="${nameq%.*}"
    compile_raw "$file" "disk/BC$afaf.BIN"
    echo "/bin/$name;BC$afaf;BIN;" >> disk/FSLST.IFS
    let afaf=afaf+1
done

afaf=0
for file in build/modules/*.modubin; do
    nameq=$(basename "$file"); name="${nameq%.*}"
    cp "$file" "disk/AC$afaf.BIN"
    echo "/kernel/$name;AC$afaf;BIN;" >> disk/FSLST.IFS
    let afaf=afaf+1
done

afaf=0
for file in drivers/*.c; do
    nameq=$(basename "$file"); name="${nameq%.*}"
    compile_raw "$file" "disk/DV$afaf.BIN"
    echo "/dev/$name;DV$afaf;BIN;" >> disk/FSLST.IFS
    let afaf=afaf+1
done

afaf=0
for file in modush/*.c; do
    nameq=$(basename "$file"); name="${nameq%.*}"
    compile_raw "$file" "disk/SY$afaf.BIN"
    echo "/sys/$name;SY$afaf;BIN;" >> disk/FSLST.IFS
    let afaf=afaf+1
done

afaf=0
for file in modush/*.sh; do
    nameq=$(basename "$file"); name="${nameq%.*}"
    cp "$file" "disk/SH$afaf.NSH"
    echo "/sys/$name.sh;SH$afaf;NSH;" >> disk/FSLST.IFS
    let afaf=afaf+1
done

afaf=0
for file in usr/*; do
    name=$(basename "$file")
    cp "$file" "disk/SR$afaf.SRN"
    echo "/home/$name;SR$afaf;SRN;" >> disk/FSLST.IFS
    let afaf=afaf+1
done

echo "/fs.struct;FSLST;IFS;" >> disk/FSLST.IFS

echo "making"

# Disk image (explicit raw to avoid warning)
qemu-img create -f raw build/disk.img 1M
mkfs.fat -F 12 build/disk.img

sudo mkdir -p /mnt/disk_qemu
sudo mount -o loop build/disk.img /mnt/disk_qemu
sudo cp disk/* /mnt/disk_qemu/
sudo umount /mnt/disk_qemu

# GRUB ISO
mkdir -p config/boot/grub
cp build/kernel config/boot/kernel
cat > config/boot/grub/grub.cfg <<'EOF'
set timeout=12
set default=0

menuentry "ModuKernel" {
    multiboot /boot/kernel
    boot
}

EOF
grub-mkrescue -o build/os.iso config

dd if=/dev/zero of=build/floppy.img bs=1024 count=1440
mkfs.fat -F 12 build/floppy.img

mkdir -p /mnt/qemu_floppy
sudo mount -o loop build/floppy.img /mnt/qemu_floppy

# como plus puedes usar tambien una unidad de floppy disk
sudo cp floppy/* /mnt/qemu_floppy/

sudo umount /mnt/qemu_floppy

# Run QEMU via GRUB so framebuffer is honored
qemu-system-i386 \
  -cdrom build/os.iso \
  -boot d \
  -fda build/floppy.img \
  -hda build/disk.img \
  -m 512M \
  -vga std \
  -audiodev pa,id=snd0,out.frequency=44100,out.channels=2
