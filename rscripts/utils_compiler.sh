
# compilar un c
function compile_raw() {
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
function compile_asm() {
    # el archivo
    local src="$1"
    # el outer
    local out="$2"

    # compilar
    nasm -f elf32 "$src" -o build/tempa.o
    # linkear
    ld -m elf_i386 -T ABI/user_link.ld --oformat binary build/tempa.o -o "$out"
}
