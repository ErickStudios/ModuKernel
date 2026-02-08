#!/usr/bin/env bash
cd ..;set -e

# flags de compilacion
UnableExamplesDrivers=false
UnableExamplesPrograms=false
UnableSystemRuntimePrograms=false
UnableModuShUtilities=false
UnableUserFiles=false
UnableKernelCompilation=false

# importar funciones
source rscripts/utils_compiler.sh

# crear el build
mkdir -p build;rm -rf disk/*;: > disk/FSLST.IFS

echo "------------------------------------------------------------------------------------------------------------------------"
printf "| %-37s | %-38s | %-35s |\n" "Program/Driver/Module" "Target File" "Description"
echo "------------------------------------------------------------------------------------------------------------------------"

# parametros del script
for arg in "$@"; do
    # desactivar drivers de ejemplos
    if [ "$arg" == "--udrvex" ]; then
        UnableExamplesDrivers=true
    # desactivar programas de ejemplos
    elif [ "$arg" == "--uprgex" ]; then
        UnableExamplesPrograms=true
    # desactivar runtime estandar del sistema
    elif [ "$arg" == "--usysrt" ]; then
        UnableSystemRuntimePrograms=true
    # desactivar programas de la shell
    elif [ "$arg" == "--umshu" ]; then
        UnableModuShUtilities=true
    # desactivar archivos de usuario
    elif [ "$arg" == "--uufs" ]; then
        UnableUserFiles=true
    # desactivar compilacion del kernel
    elif [ "$arg" == "--ukc" ]; then
        UnableKernelCompilation=true
    fi
done

# si compilara el kernel
if [ "$UnableKernelCompilation" = false ]; then

    printf "%-40s:%-40s\n" "assembly/kernel.asm" "build/kasm.o"

    # compilar el kernel.asm
    nasm -f elf32 assembly/kernel.asm -o build/kasm.o -w-number-overflow 1>/dev/null
    
    printf "%-40s:%-40s\n" "kernel/kernel.c" "build/kc.o"
    # compilar el kernel.c
    gcc -m32 -ffreestanding -fno-stack-protector -nostdlib -c kernel/kernel.c -o build/kc.o -w 1>/dev/null
    # linkear el kernel
    ld -m elf_i386 -T ABI/kernel_link.ld -o build/kernel build/kasm.o build/kc.o --no-warn-rwx-segments 1>/dev/null

fi
# compilar los programas
if [ "$UnableSystemRuntimePrograms" = false ]; then
    # cedear a modulos
    cd modules
    # abrir el otro build de modulos importantes
    source build.sh
fi
# si se activan los programas de ejemplo
if [ "$UnableExamplesPrograms" = false ]; then
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
    for file in programs/*.cpp; do
        # nombre
        nameq=$(basename "$file"); name="${nameq%.*}"
        # compilar
        compile_cpp "$file" "disk/BC$afaf.BIN"
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
fi
# si se activan el runtime del sistema
if [ "$UnableSystemRuntimePrograms" = false ]; then
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
fi
# los drivers
if [ "$UnableExamplesDrivers" = false ]; then
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
fi
# si se desactivan las utilidades de la shell
if [ "$UnableModuShUtilities" = false ]; then
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
fi
# si se desactivan archivos de usuario
if [ "$UnableUserFiles" = false ]; then
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
fi
# contador
afaf=0
# bucle
for file in candy/lib/*; do
    # nombre
    name=$(basename "$file")
    # copiar
    cp "$file" "disk/CDY$afaf.CDY"
    # agregar al fsmap
    echo "/lib/candy/$name;CDY$afaf;CDY;" >> disk/FSLST.IFS
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
grub-mkrescue -o build/os.iso config;dd if=/dev/zero of=build/floppy.img bs=1024 count=1440

# crear el floppy
mkfs.fat -F 12 build/floppy.img;mkdir -p /mnt/qemu_floppy;sudo mount -o loop build/floppy.img /mnt/qemu_floppy
# como plus puedes usar tambien una unidad de floppy disk
sudo cp floppy/* /mnt/qemu_floppy/;sudo umount /mnt/qemu_floppy

# arranca qemu
qemu-system-i386                                              \
  -cpu pentium3                                               \
  -cdrom build/os.iso                                         \
  -boot d                                                     \
  -fda build/floppy.img                                       \
  -hda build/disk.img                                         \
  -m 256M                                                     \
  -vga std                                                    \
  -device isa-debugcon,chardev=mimami                         \
  -chardev stdio,id=mimami                                    \
  -netdev user,id=net0                                        \
  -device rtl8139,netdev=net0,mac=52:54:00:12:34:56           \
  -audiodev pa,id=snd0,out.frequency=44100,out.channels=2
