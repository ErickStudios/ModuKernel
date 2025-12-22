
/* si esta presionandose shift */
int shift_pressed = 0;
/* crea memoria internal */
void* InternalAllocatePool(unsigned int size, ModuAllocType Type);
/* crea memoria */
void* AllocatePool(unsigned int size);
/* inicializa el kernel */
void InitializeKernel(KernelServices* Services);
/* el mini programa del kernel */
KernelStatus InternalMiniKernelProgram(KernelServices* Services);
/* función para leer una tecla */
unsigned char InternalKeyboardReadChar();
/* ejecuta un comando */
void InternalSysCommandExecute(KernelServices* Services, char* command, int len);
/* para apagar */
KernelStatus InternalKernelReset(int func);
/* para ejecutar un binario */
KernelStatus InternalRunBinary(void* buffer, int size, KernelServices* Services);
/* para encontrar un archivo extendido */
FatFile InternalExtendedFindFile(char* path);
/* para fecha y hora */
void InternalGetDateTime(KernelDateTime* Time);