# archivo del script de inicializacion que ejecutara 
# /kernel/init

# entrar al kernel
cd /

# mensaje de bienvenida
echo Welcome to ModuKernel!
# abrir la shell
kernel/modush

# llamar al halt
call halt

# halteo
halt:
    # saltar al halt
    jmp halt