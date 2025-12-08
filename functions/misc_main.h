// tabla muy básica de scancodes → ASCII (solo letras y números)
static const char scancode_table[128] = {
    0,  27, '1','2','3','4','5','6','7','8','9','0','-','=', '\b',
    '\t','q','w','e','r','t','y','u','i','o','p','[',']','\n', 0,
    'a','s','d','f','g','h','j','k','l',';','\'','`', 0,'\\',
    'z','x','c','v','b','n','m',',','.','/', 0, '*', 0, ' ', // etc...
};
// si esta presionandose shift
int shift_pressed = 0;
// crea memoria
void* AllocatePool(unsigned int size);
// inicializa el kernel
KernelServices InitializeKernel();
// el mini programa del kernel
void InternalMiniKernelProgram(KernelServices* Services);
// el kernel principal
void k_main();
// función para leer una tecla
char InternalKeyboardReadChar();
