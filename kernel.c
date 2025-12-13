// incluir archivos
#include "services/KernelServices.h"
#include "kernel.h"
#include "fs/fat12.h"

// incluir funciones y prototipos
#include "functions/misc_main.h"
#include "functions/disk_main.h"
#include "functions/screen_main.h"
#include "functions/c_main.h"
#include "functions/heap_main.h"

// incluir funciones de la libreria c de bajo nivel
#include "libc/String.h"

// servicios globales
KernelServices* GlobalServices;
// mayusculas y minusculas
char LowerUpper = 0;
// version del sistema
int InternalServicesVersion = 10;

static BlockHeader* heap_start = (BlockHeader*)&_heap_start;
static BlockHeader* free_list = NULL;

void InitHeap() 
{
    free_list = heap_start;
    free_list->size = _heap_end - _heap_start - sizeof(BlockHeader);
    free_list->free = 1;
    free_list->next = NULL;
}
KernelStatus InternalDiskReadSector(unsigned int lba, unsigned char* buffer) {
	// lo lee 1
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); 
	// lo lee 2
    outb(0x1F2, 1); outb(0x1F3, (uint8_t) lba); outb(0x1F4, (uint8_t)(lba >> 8)); 
	// lo lee 3
	outb(0x1F5, (uint8_t)(lba >> 16)); outb(0x1F7, 0x20);     

	int timeout = 100000;

    // esperar a que DRQ esté listo
    while (!(inb(0x1F7) & 0x08) && (timeout--)) if (timeout < 1) return KernelStatusInfiniteLoopTimeouted;

    // leer 256 palabras (512 bytes)
    for (int i = 0; i < 256; i++) ((uint16_t*)buffer)[i] = inw(0x1F0);

	return KernelStatusSuccess;
}
FatFile InternalDiskFindFile(char* name, char* ext) {
    uint8_t sector0[512];
    InternalDiskReadSector(0, sector0);

    struct _FAT12_BootSector* bs_local = (struct _FAT12_BootSector*) sector0;

    // Copia el boot sector a heap (memoria persistente)
    struct _FAT12_BootSector* bs = (struct _FAT12_BootSector*) AllocatePool(sizeof(*bs));
    if (!bs) {
        FatFile empty = {0};
        return empty; // o manejar error
    }
    InternalMemoryCopy(bs, bs_local, sizeof(*bs));

    // calcular root_start, root_sectors...
    unsigned int root_start = bs->reserved_sectors + bs->num_fats * bs->fat_size_sectors;
    unsigned int root_sectors = (bs->root_entries * 32) / bs->bytes_per_sector;

    // en lugar de buffer grande en la pila, usa heap o lee sector a sector
    uint8_t* root_buffer = (uint8_t*) AllocatePool(bs->bytes_per_sector * root_sectors);
    if (!root_buffer) {
        // liberar bs si es necesario
        FatFile empty = {0};
        return empty;
    }

    for (int s = 0; s < root_sectors; s++) {
        InternalDiskReadSector(root_start + s, root_buffer + s * bs->bytes_per_sector);
    }

    struct _FAT12_DirEntry* dir = (struct _FAT12_DirEntry*) root_buffer;
    int total_entries = bs->root_entries;

    for (int i = 0; i < total_entries; i++) {
        if (dir[i].name[0] == 0x00 || dir[i].name[0] == 0xE5) continue;

        if (InternalMemoryComp(dir[i].name, name, 8) == 0 &&
            InternalMemoryComp(dir[i].name + 8, ext, 3) == 0) {
            FatFile File;
            File.bs = bs;           // puntero a la copia en heap (válido después del return)
            File.sector = dir[i];   // copia por valor del dir entry
            // opcional: liberar root_buffer si ya no se necesita
            return File;
        }
    }

    // no encontrado: liberar root_buffer y bs si necesario
    FatFile empty = {0};
    return empty;
}
char* InternalReadLine()
{
	// buffers
	char bufcmd[256]; int char_set = 0;
	for (;;)
	{
		char key = GlobalServices->InputOutpud->WaitKey(); 

		if (key == '\b') {
			// si no es 0
			if (GlobalServices->Display->CurrentCharacter > 0) {
				// retroceder posicion
				GlobalServices->Display->setCursorPosition(GlobalServices->Display->CurrentCharacter - 2, GlobalServices->Display->CurrentLine);
				// caracter anterior
				char_set--;
				// borrar el caracter
				char *vidmem = (char*)0xb8000;
				// linea actual
				int pos = GlobalServices->Display->CurrentLine * 80 * 2 + GlobalServices->Display->CurrentCharacter - 2;
				// llenar con caracter vacio
				vidmem[pos] = ' ';
				// el atributo de texto
				vidmem[pos+1] = *text_attr;
			}
		}
		else {
			// el caracter
			char buff[2] = { key , 0};
			// si no es enter agregar caracter
			if (key != '\n') bufcmd[char_set] = key;
			// imprimir caracter
			if (key != '\n') GlobalServices->Display->printg(buff);
		}
		// para el caracter
		if (key != 0 && key != '\n' && key != '\b') char_set++;
		// si es enter
		if (key == '\n')
		{	
			// retornarlo
			char* retval = AllocatePool(sizeof(char) * 256);

			for (short index = 0; index < 256; index++) retval[index] = bufcmd[index];

			return retval;
		}
	}
}
KernelStatus InternalDiskGetFile(FatFile file, void** content, int* size)
{
    if (file.sector.name[0] == 0x00)
        return KernelStatusNotFound;

    struct _FAT12_BootSector* bs = file.bs;
    struct _FAT12_DirEntry dir   = file.sector;

    // --- Leer FAT completa ---
    unsigned int fat_bytes = bs->fat_size_sectors * bs->bytes_per_sector;
    uint8_t* fat = (uint8_t*) AllocatePool(fat_bytes);
    if (!fat) return KernelStatusNoBudget;

    for (unsigned int s = 0; s < bs->fat_size_sectors; s++)
    {
        KernelStatus st = InternalDiskReadSector(bs->reserved_sectors + s,
                                                 fat + s * bs->bytes_per_sector);

        if (_StatusError(st))
            return st;
    }

    // --- Calcular data region ---
    unsigned int root_start   = bs->reserved_sectors + bs->num_fats * bs->fat_size_sectors;
    unsigned int root_sectors = (bs->root_entries * 32) / bs->bytes_per_sector;
    unsigned int data_start   = root_start + root_sectors;

    // --- Reservar buffer para output ---
    uint8_t* out = (uint8_t*) AllocatePool(dir.file_size);
    if (!out) return KernelStatusMemoryRot;

    unsigned int cluster   = dir.first_cluster;
    unsigned int remaining = dir.file_size;
    unsigned int offset    = 0;

    uint8_t sector[512];

    // --- Leer clusters ---
    while (cluster < 0xFF8 && remaining > 0)
    {
        unsigned int lba = data_start + (cluster - 2) * bs->sectors_per_cluster;

        for (unsigned int s = 0; s < bs->sectors_per_cluster && remaining > 0; s++)
        {
            KernelStatus st = InternalDiskReadSector(lba + s, sector);
            if (_StatusError(st)) return st;

            unsigned int copy = (remaining > bs->bytes_per_sector ?
                                 bs->bytes_per_sector : remaining);

            InternalMemoryCopy(out + offset, sector, copy);

            offset    += copy;
            remaining -= copy;
        }

        // siguiente cluster
        cluster = get_fat_entry(cluster, fat);
    }

    *content = out;
    *size    = dir.file_size;
    return KernelStatusSuccess;
}
void InternalSetAttriubtes(char bg, char fg)
{
	// crearlo
	*text_attr = ((bg << 4) | fg);
}
void InternalSetActualDisplayService(DisplayServices* Serv)
{
	// linea seleccionada
	line_selected = &Serv->CurrentLine;
	// colummna
	row_selected = &Serv->CurrentCharacter;
	// atributo de texto
	text_attr = &Serv->CurrentAttrs;
}
KernelStatus InternalKernelReset(int func) 
{

    __asm__ __volatile__("cli");   // Deshabilitar interrupciones

    if (func == 1) {
        outw(0x604, 0x2000);
        outw(0xB004, 0x2000);

        goto triple_fault;
    }

    while (inb(0x64) & 0x02);

	outb(0x64, 0xFE);

triple_fault:
    return KernelStatusDisaster;
}
unsigned char InternalKeyboardReadChar()
{
	int extended = 0;
	while(1) {
		uint8_t status = inb(0x64);

		if(status & 0x01) {
			uint8_t scancode = inb(0x60);

			char character = 0;

			if(scancode == 0x01) character = KernelSimpleIoSpecKey(9);
			else if(scancode == 0x0E) character = '\b';
			else if(scancode == 0x02) character = LowerUpper ? '!' : '1';
			else if(scancode == 0x03) character = LowerUpper ? '"' : '2';
			else if(scancode == 0x04) character = LowerUpper ? '#' : '3';
			else if(scancode == 0x05) character = LowerUpper ? '$' : '4';
			else if(scancode == 0x06) character = LowerUpper ? '%' : '5';
			else if(scancode == 0x07) character = LowerUpper ? '&' : '6';
			else if(scancode == 0x08) character = LowerUpper ? '/' : '7';
			else if(scancode == 0x09) character = LowerUpper ? '(' : '8';
			else if(scancode == 0x0A) character = LowerUpper ? ')' : '9';
			else if(scancode == 0x0B) character = LowerUpper ? '=' : '0';
			else if(scancode == 0x0C) character = LowerUpper ? '?' : '\'';
			else if(scancode == 0x0D) character = LowerUpper ? ' ' : '!';
			else if(scancode == 0x1A) character = LowerUpper ? '{' : '[';
			else if(scancode == 0x1B) character = LowerUpper ? '}' : ']';
			else if(scancode == 0x27) character = LowerUpper ? ':' : ';';
			else if(scancode == 0x0D) character = LowerUpper ? '+' : '=';
			else if(scancode == 0x33) character = LowerUpper ? '?' : ',';
			else if(scancode == 0x34) character = LowerUpper ? '>' : '.';
			else if(scancode == 0x35) character = LowerUpper ? '>' : '<';
			else if(scancode == 0x3A) LowerUpper = !LowerUpper;
			else if(scancode == 0x1E) character = 'a';
			else if(scancode == 0x30) character = 'b';
			else if(scancode == 0x2E) character = 'c';
			else if(scancode == 0x20) character = 'd';
			else if(scancode == 0x12) character = 'e';
			else if(scancode == 0x21) character = 'f';
			else if(scancode == 0x22) character = 'g';
			else if(scancode == 0x23) character = 'h';
			else if(scancode == 0x17) character = 'i';
			else if(scancode == 0x24) character = 'j';
			else if(scancode == 0x25) character = 'k';
			else if(scancode == 0x26) character = 'l';
			else if(scancode == 0x32) character = 'm';
			else if(scancode == 0x31) character = 'n';
			else if(scancode == 0x18) character = 'o';
			else if(scancode == 0x19) character = 'p';
			else if(scancode == 0x10) character = 'q';
			else if(scancode == 0x13) character = 'r';
			else if(scancode == 0x1F) character = 's';
			else if(scancode == 0x14) character = 't';
			else if(scancode == 0x16) character = 'u';
			else if(scancode == 0x2F) character = 'v';
			else if(scancode == 0x11) character = 'w';
			else if(scancode == 0x2D) character = 'x';
			else if(scancode == 0x15) character = 'y';
			else if(scancode == 0x2C) character = 'z';
			else if(scancode == 0x39) character = ' ';
			else if(scancode == 0x1C) character = '\n'; 

			else if(scancode == 0x3B) character = KernelSimpleIoFuncKey(1);
			else if(scancode == 0x3C) character = KernelSimpleIoFuncKey(2);
			else if(scancode == 0x3D) character = KernelSimpleIoFuncKey(3);
			else if(scancode == 0x3E) character = KernelSimpleIoFuncKey(4);
			else if(scancode == 0x3F) character = KernelSimpleIoFuncKey(5);
			else if(scancode == 0x40) character = KernelSimpleIoFuncKey(6);
			else if(scancode == 0x41) character = KernelSimpleIoFuncKey(7);
			else if(scancode == 0x42) character = KernelSimpleIoFuncKey(8);
			else if(scancode == 0x43) character = KernelSimpleIoFuncKey(9);
			else if(scancode == 0x44) character = KernelSimpleIoFuncKey(10);

            // Tecla extendida
            if (scancode == 0xE0) {
                extended = 1;
                continue;
            }

			if (extended) {
                extended = 0;

                if (scancode == 0x48) return KernelSimpleIoSpecKey(1); // arriba
                if (scancode == 0x50) return KernelSimpleIoSpecKey(2); // abajo
                if (scancode == 0x4B) return KernelSimpleIoSpecKey(3); // izquierda
                if (scancode == 0x4D) return KernelSimpleIoSpecKey(4); // derecha
				if (scancode == 0x1D) return KernelSimpleIoSpecKey(4); // control izquierdo
				if (scancode == 0x38) return KernelSimpleIoSpecKey(5); // AltGr
				if (scancode == 0x5B) return KernelSimpleIoSpecKey(6); // Logo derecho
				if (scancode == 0x5C) return KernelSimpleIoSpecKey(7); // Logo izquierdo
				if (scancode == 0x5D) return KernelSimpleIoSpecKey(8); // menu/apps
            }

			return LowerUpper == 1 ? CharToUpCase(character) : character;
		}
	}

}
void InitializeKernel(KernelServices* Services)
{
    // opcional: limpia la estructura principal (si vive en stack)
    InternalMemorySet(Services, 0, sizeof(KernelServices));

    // reservar subestructuras
    DisplayServices* Dsp = AllocatePool(sizeof(DisplayServices));
    MemoryServices* Mem = AllocatePool(sizeof(MemoryServices));
    IoServices* IO     = AllocatePool(sizeof(IoServices));
    DiskServices* Dsk  = AllocatePool(sizeof(DiskServices));
    KernelMiscServices* Msc = AllocatePool(sizeof(KernelMiscServices));

    // comprobar allocs
    if (!Dsp || !Mem || !IO || !Dsk || !Msc) {
        InternalPrintg("ERR: alloc failed in InitializeKernel", 1);
        // manejo de error mínimo: puedes haltear o intentar fallback
        return;
    }

    // limpiar memoria de cada subestructura
    InternalMemorySet(Dsp, 0, sizeof(DisplayServices));
    InternalMemorySet(Mem, 0, sizeof(MemoryServices));
    InternalMemorySet(IO,  0, sizeof(IoServices));
    InternalMemorySet(Dsk, 0, sizeof(DiskServices));
    InternalMemorySet(Msc, 0, sizeof(KernelMiscServices));

    // conectar subestructuras
    Services->Display = Dsp;
    Services->Memory  = Mem;
    Services->InputOutpud = IO;
    Services->File    = Dsk;
    Services->Misc    = Msc;

    // servicios principales
    Services->Misc->Run       = &InternalSysCommandExecute;
	Services->Misc->Reset	  = &InternalKernelReset;
    Services->ServicesVersion = &InternalServicesVersion;

    // pantalla
    Dsp->printg_i          = &InternalPrintg;
    Dsp->printg            = &InternalPrintgNonLine;
    Dsp->setCursorPosition = &InternalCursorPos;
    Dsp->clearScreen       = &InternalClearScreen;
    Dsp->Set               = &InternalSetActualDisplayService;
    Dsp->setAttrs          = &InternalSetAttriubtes;

    Dsp->CurrentLine      = 0;
    Dsp->CurrentCharacter = 0;
    Dsp->CurrentAttrs     = 0;

    // IO
    IO->Input   = &inb;
    IO->Outpud  = &outb;
    IO->WaitKey = &InternalKeyboardReadChar;
    IO->ReadLine= &InternalReadLine;

    // memoria
    Mem->AllocatePool = &AllocatePool;
    Mem->MoveMemory   = &InternalMemMove;
    Mem->CoppyMemory  = &InternalMemoryCopy;
    Mem->CompareMemory= &InternalMemoryComp;
    Mem->FreePool     = &FreePool;

    // disco
    Dsk->RunFile    = &ProcessCrtByFile;
    Dsk->FindFile   = &InternalDiskFindFile;
    Dsk->GetFile    = &InternalDiskGetFile;
    Dsk->ReadSector = &InternalDiskReadSector;

    // hacer global la estructura principal
    GlobalServices = Services;
}
void InternalSysCommandExecute(KernelServices* Services, char* command, int lena)
{
	int len = StrLen(command);

	if (StrCmp(command, "cls") == 0) Services->Display->clearScreen();
	else if (StrCmp(command ,"ls") == 0)
	{
		// leer boot sector
    	uint8_t sector0[512]; InternalDiskReadSector(0, sector0);

		struct _FAT12_BootSector* bs = (struct _FAT12_BootSector*) sector0;
		unsigned int root_start = bs->reserved_sectors + bs->num_fats * bs->fat_size_sectors;
		unsigned int root_sectors = (bs->root_entries * 32) / bs->bytes_per_sector;
		uint8_t root_buffer[512 * root_sectors];

// Leer todos los sectores del directorio raíz
for (int s = 0; s < root_sectors; s++) {
    InternalDiskReadSector(root_start + s, root_buffer + s * bs->bytes_per_sector);
}

// Recorrer todas las entradas del directorio raíz
struct _FAT12_DirEntry* dir = (struct _FAT12_DirEntry*)root_buffer;
int total_entries = bs->root_entries;

for (int i = 0; i < total_entries; i++) {
    if (dir[i].name[0] == 0x00 || dir[i].name[0] == 0xE5) continue;

    // Mostrar nombre y extensión
    Services->Display->printg(dir[i].name);
	Services->Display->printg("     ");
}

Services->Display->printg("\n");
	}
	else if (StrnCmp(command, "echo ", 5) == 0) { if (len == 5); else Services->Display->printg(command + 5);Services->Display->printg("\n"); }
	else if (StrCmp(command, "prp") == 0){Services->Display->setAttrs(0, 10); Services->Display->printg("ModuKernel");Services->Display->setAttrs(0, 9); Services->Display->printg(":~");Services->Display->setAttrs(0, 7); Services->Display->printg("# ");}
	else if (StrCmp(command, "reset") == 0) Services->Misc->Reset(0);
	else if (StrCmp(command, "shutdown") == 0) Services->Misc->Reset(1);
	else if (StrCmp(command, "") == 0);
	else
	{
		if (len < 9)
		{
			char name[9];

			// Rellenar con espacios (mejor práctica)
			InternalMemorySet(name, ' ', 8);
			name[8] = '\0';

			// Copiar el comando a name
			for (int i = 0; i < len && i < 8; i++)
				name[i] = command[i];

			FatFile File = Services->File->FindFile(name, "BIN");

			void* buffer = 0;
			int size = 0;

			KernelStatus Status = Services->File->GetFile(File, &buffer, &size);

			#define USER_LOAD_ADDR ((uint8_t*)0x00400000)

			typedef struct {
				char magic[8];       // "ModuBin\0"
				uint32_t entry;       // dirección de ErickMain
				uint32_t bss_start;   // dirección inicio .bss
				uint32_t bss_end;     // dirección fin .bss
			} UserHeader;

			if ((!_StatusError(Status)) && size >= sizeof(UserHeader)) {
				// Copiar binario a su dirección de enlace
				InternalMemoryCopy(USER_LOAD_ADDR, buffer, size);

				// Leer encabezado
				UserHeader* hdr = (UserHeader*)USER_LOAD_ADDR;
				if (Services->Memory->CompareMemory(hdr->magic, "ModuBin", 7) != 0) {
					Services->Display->printg("header magic invalido\n");
				}

				// Cero .bss
				if (hdr->bss_end > hdr->bss_start) {
					size_t bss_size = (size_t)(hdr->bss_end - hdr->bss_start);
					InternalMemorySet((void*)hdr->bss_start, 0, bss_size);
				}

				// Ejecutar
				typedef KernelStatus (*ProgramEntry)(KernelServices*);
				ProgramEntry entry = (ProgramEntry)(uintptr_t)hdr->entry;
				KernelStatus result = entry(Services);

				char out[13];
				IntToString(result, out);
				Services->Display->printg("\nEl programa retorno = ");
				Services->Display->printg(out);
				Services->Display->printg("\n");
			}
			else
			{
				if (size == 0) Services->Display->printg("(size==0)\n");
				Services->Display->printg("KernelStatus Status = ");
				char error[13]; 
				IntToString(Status, error);
				Services->Display->printg(error);
				Services->Display->printg("; // no se pudo leer\n");
			}
		}
	}
}
KernelStatus InternalMiniKernelProgram(KernelServices* Services)
{
	// mensajes
	Services->Display->printg("Welcome to ModuKernel!\n");

	for (;;) {
		Services->Misc->Run(Services, "prp", 0);
		char* Prompt = Services->InputOutpud->ReadLine();

		Services->Display->printg("\n");
		Services->Misc->Run(Services, Prompt, 0);
		Services->Memory->FreePool(Prompt);
	}
}
void k_main() 
{
	InitHeap();
    KernelServices Services;

	InitializeKernel(&Services);

    Services.Display->Set(Services.Display);
    Services.Display->setAttrs(0, 7);
    Services.Display->clearScreen();

    InternalMiniKernelProgram(&Services);
}
KernelStatus ProcessCrtByFile(char* name, char* ext, KernelServices* Services)
{
	FatFile file = InternalDiskFindFile(name, ext);

	// después de cargar el archivo en memoria
	void* data;
	int size;

	if (InternalDiskGetFile(file, &data, &size) == 0) {
		// convertir el puntero a función
		typedef int (*ProgramEntry)(KernelServices* Services);
		ProgramEntry entry = (ProgramEntry) data;

		// ejecutar el programa pasando los servicios del kernel
		int result = entry(Services);
	}
}
void* AllocatePool(unsigned int size) {
    BlockHeader* current = free_list;

    while (current) {
        if (current->free && current->size >= size) {

            if (current->size > size + sizeof(BlockHeader)) {
                // dividir el bloque
                BlockHeader* new_block = (BlockHeader*)((char*)current + sizeof(BlockHeader) + size);
                new_block->size = current->size - size - sizeof(BlockHeader);
                new_block->free = 1;
                new_block->next = current->next;

                current->next = new_block;
            }

            current->free = 0;
            current->size = size;
            return (char*)current + sizeof(BlockHeader);
        }

        current = current->next;
    }

    return 0; // out of memory
}
void FreePool(void* ptr) {
    if (!ptr) return;

    BlockHeader* block = (BlockHeader*)((char*)ptr - sizeof(BlockHeader));
    block->free = 1;

    // intento de coalescer bloques contiguos
    BlockHeader* current = free_list;

    while (current) {
        if (current->free && current->next && current->next->free) {
            // unir bloques
            current->size += sizeof(BlockHeader) + current->next->size;
            current->next = current->next->next;
        }
        current = current->next;
    }
}
void InternalClearScreen()
{
	*row_selected = 0;
	*line_selected = 0;

	// la memoria de video
	char *vidmem = (char *) 0xb8000; unsigned int i=0;
	// limpia la pantalla
	while(i < (80*25*2)) { vidmem[i]=' '; i++; vidmem[i]=*text_attr; i++; };
};
void InternalPrintg(char *message, unsigned int line)
{
	// memoria de video
	char *vidmem = (char *) 0xb8000;
	// el int
	unsigned int i=0;
	// añadir linea
	i=(line*80*2);
	// esperar a que no sea 0
	while(*message!=0)
	{
		// nueva linea
		if(*message=='\n')
		{
			// salto
			line++; i=(line*80*2); *message++;
		} else {
			// llenar con el caracter
			vidmem[i]=*message; *message++; i++;
			// llenar con el atributo de texto
			vidmem[i]=0x07; i++;
		};
	};
}
void InternalCursorPos(int x, int y) { 
	*(row_selected) = x; *(line_selected) = y; 
}
void InternalPrintgNonLine(char *message)
{
	// memoria de video
	char *vidmem = (char *) 0xb8000;
	// el int
	unsigned int i=0;
	// linea
	int line = *line_selected;
	// añadir linea
	i=(line*80*2) + ((*row_selected)%(1*80*2));
	// esperar a que no sea 0
	while(*message!=0)
	{
		// nueva linea
		if(*message=='\n' || (((*row_selected)%(2*80)) > (78*2))) 
		{
			// salto
			(*line_selected)++; i=((*line_selected)*80*2); *message++;

		if (*line_selected >= 25) {
			// mover todo hacia arriba (24 líneas)
			InternalMemMove(vidmem, vidmem + 80*2, 24*80*2);
			// limpiar última línea
			for (int j = 0; j < 80; j++) {
				vidmem[(24*80 + j)*2] = ' ';
				vidmem[(24*80 + j)*2 + 1] = *text_attr;
			}
			*line_selected = 24;
			i = (*line_selected)*80*2;
		}

		} else {
			// llenar con el caracter
			vidmem[i]=*message; *message++; i++;

			vidmem[i]=*text_attr; i++;
		};
	};
	// llenar con el atributo de texto
	*row_selected = i;
}
