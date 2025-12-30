cd ..

compile_raw() {
    local src="$1"
    local out="$2"
    gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib -c "$src" -o build/temp.o
    ld -m elf_i386 -T ABI/user_link.ld --oformat binary build/temp.o -o "$out"
}

compile_raw modules/init/main.c build/modules/init.modubin
compile_raw modules/shell/main.c build/modules/modush.modubin
compile_raw modules/mouse/ps2.c build/moddrivers/mps2.modubin