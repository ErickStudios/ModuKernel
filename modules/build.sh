cd ..

compile_raw() {
    local src="$1"
    local out="$2"
    gcc -m32 -ffreestanding -fno-pic -fno-stack-protector -nostdlib -c "$src" -o build/temp.o
    ld -m elf_i386 -T ABI/user_link.ld --oformat binary build/temp.o -o "$out"
}

# ----------------------------------------------------------------------------------------------------------
# | Programa                                                                       | Descripcion/Nombre
# ----------------------------------------------------------------------------------------------------------

# programas
compile_raw modules/init/main.c build/modules/init.modubin                          # System Initializer
compile_raw modules/shell/main.c build/modules/modush.modubin                       # ModuShell

# mouse , inicializar y desinicializr
compile_raw modules/mouse/ps2.c build/moddrivers/mps2.modubin                       # Mouse PS/2 Enable Routine
compile_raw modules/mouse/ups2.c build/moddrivers/mups2.modubin                     # Disable Mouse PS/2 Routine

# pci
compile_raw modules/pci/driver.c build/moddrivers/pci.modubin                       # PCi Driver config I/O

# red
compile_raw modules/net/SimpleNetInit.c build/moddrivers/snet.modubin               # Simple NET initializer
compile_raw modules/net/SimpleNetGetMac.c build/moddrivers/snetm.modubin            # Simple NET Mac Get
compile_raw modules/net/SimpleNetPkgSend.c build/moddrivers/snetps.modubin          # Simple NET Package Send
compile_raw modules/net/SimpleNetPkgRecive.c build/moddrivers/snetpr.modubin        # Simple NET Package Recive
compile_raw modules/net/SimpleNetPkgSet.c build/moddrivers/snetpst.modubin          # Simple NET Package Set Buffer
compile_raw modules/net/SimpleNetPkgUnset.c build/moddrivers/snetus.modubin         # Simple NET Package Unset Buffer
compile_raw modules/teleq/driver.c build/moddrivers/stlq.modubin                    # Simple NET simple telecomunication