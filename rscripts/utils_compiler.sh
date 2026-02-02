# compilar un c
function compile_raw() {
    # el archivo
    local src="$1"
    # el outer
    local out="$2"
    # la descripción opcional
    local desc="$3"

    if [ -n "$desc" ]; then
        # Si hay descripción, imprime tres columnas
        printf "%-40s:%-40s:%-7s\n" "$src" "$out" "$desc"
    else
        # Si no hay descripción, imprime solo dos columnas
        printf "%-40s:%-40s\n" "$src" "$out"
    fi

    # el gcc
    gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib -c "$src" -o temp.o -w 1>/dev/null
    # linkear
    ld -m elf_i386 -T ABI/user_link.ld --oformat binary temp.o -o "$out"
    # eliminar el temporal
    rm -f temp.o
}
# compilar un cpp
function compile_cpp() {
    local src="$1"
    local out="$2"
    # la descripción opcional
    local desc="$3"

    if [ -n "$desc" ]; then
        # Si hay descripción, imprime tres columnas
        printf "%-40s:%-40s:%-7s\n" "$src" "$out" "$desc"
    else
        # Si no hay descripción, imprime solo dos columnas
        printf "%-40s:%-40s\n" "$src" "$out"
    fi

    # compilar con g++
    g++ -m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib \
        -fno-exceptions -fno-rtti -c "$src" -o temp.o -w 1>/dev/null

    # linkear igual que en C
    ld -m elf_i386 -T ABI/user_link.ld --oformat binary temp.o -o "$out"

    # limpiar temporal
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
