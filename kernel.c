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

// servicios globales
KernelServices* GlobalServices;

// mayusculas y minusculas
char LowerUpper = 0;

void InternalDiskReadSector(uint32_t lba, uint8_t* buffer) {
	// lo lee 1
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); 
	// lo lee 2
    outb(0x1F2, 1); outb(0x1F3, (uint8_t) lba); outb(0x1F4, (uint8_t)(lba >> 8)); 
	// lo lee 3
	outb(0x1F5, (uint8_t)(lba >> 16)); outb(0x1F7, 0x20);      
	                 
    // esperar a que DRQ esté listo
    while (!(inb(0x1F7) & 0x08));

    // leer 256 palabras (512 bytes)
    for (int i = 0; i < 256; i++) ((uint16_t*)buffer)[i] = inw(0x1F0);
}
FatFile InternalDiskFindFile(char* name, char* ext) {
    // leer boot sector
    uint8_t sector0[512];
	// lee el sector 0
    InternalDiskReadSector(0, sector0);

	// el sector
    struct _FAT12_BootSector* bs = (struct _FAT12_BootSector*) sector0;
    // calcular root dir
    unsigned int root_start = bs->reserved_sectors + bs->num_fats * bs->fat_size_sectors;
    // sectores
	unsigned int root_sectors = (bs->root_entries * 32) / bs->bytes_per_sector;
	// el sector
    uint8_t root_buffer[512 * root_sectors];
	// los lee
    for (int s = 0; s < root_sectors; s++) {
		// lo busca
        InternalDiskReadSector(root_start + s, root_buffer + s*512);
    }
    
	// recorrer directorio
    struct _FAT12_DirEntry* dir = (struct _FAT12_DirEntry*) root_buffer;
    // recorre los directorios
	for (int i = 0; i < bs->root_entries; i++) {
        // si no existe o fue eliminado continuar
		if (dir[i].name[0] == 0x00 || dir[i].name[0] == 0xE5) continue;
		
		// si coincide el nombe y la extension
        if (memcmp(dir[i].name, name, 8) == 0 &&
            memcmp(dir[i].ext, ext, 3) == 0) {
			// convertirlo a retorno multiple
			FatFile File;

			// mover retornos
			File.bs = bs; File.sector = dir[i];

			// retornar la estrcutura
			return File;
        }
    }
}
int InternalDiskGetFile(FatFile file, void** content, int* size) {
    // ir al sector
	struct _FAT12_BootSector* bs = file.bs;
    struct _FAT12_DirEntry dir = file.sector;

    // leer FAT completa (usamos la primera copia)
    unsigned int fat_start = bs->reserved_sectors;
    unsigned int fat_size = bs->fat_size_sectors * bs->bytes_per_sector;
    
	// crear la memoria
	uint8_t* fat = (uint8_t*) AllocatePool(fat_size);

   // leer sectores
	for (unsigned int s = 0; s < bs->fat_size_sectors; s++) {
        InternalDiskReadSector(fat_start + s, fat + s*bs->bytes_per_sector);
    }

    // calcular inicio de data region
    unsigned int root_start   = bs->reserved_sectors + bs->num_fats * bs->fat_size_sectors;
	unsigned int root_sectors = (bs->root_entries * 32) / bs->bytes_per_sector;
    unsigned int data_start   = root_start + root_sectors;

    // reservar buffer para el archivo
    uint8_t* out = (uint8_t*) AllocatePool(dir.file_size);
    if (!out) return -1;

	// informacion
    unsigned int cluster   = dir.first_cluster;
    unsigned int remaining = dir.file_size;
    unsigned int offset    = 0;
	
	// el sector
    uint8_t sector[512];

	// recuperarlo
    while (cluster < 0xFF8 && remaining > 0) {
		// sector grande
        unsigned int lba = data_start + (cluster - 2) * bs->sectors_per_cluster;

		// recorrerlo
        for (unsigned int s = 0; s < bs->sectors_per_cluster && remaining > 0; s++) {
            // leer sector
			InternalDiskReadSector(lba + s, sector);

			// tamaño copiado
            unsigned int copy_size = remaining > bs->bytes_per_sector ? bs->bytes_per_sector : remaining;
			// copiar memoria
			memcpy(out + offset, sector, copy_size);

			// aumentar variables
            offset    += copy_size;
            remaining -= copy_size;
        }

		// obtiene entrada de fat
        cluster = get_fat_entry(cluster, fat);
    }

    *content = out;
    *size    = dir.file_size;
    return 0;
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

char InternalKeyboardReadChar() {
	while(1) {
		uint8_t status = inb(0x64); // THAT'S WHY WE NEED I/O DRIVER, TO READ THE STATUS OF THE KEYBOARD AND THE SCANCODES :)

		if(status & 0x01) {
			uint8_t scancode = inb(0x60);

			char character = 0;

			if(scancode == 0x01) {
			}
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
			else if(scancode == 0x0D) character = LowerUpper ? '¿' : '!';
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

			return LowerUpper == 1 ? CharToUpCase(character) : character;
		}
	}

}


KernelServices InitializeKernel()
{
	// servicios
	KernelServices Services;
	DisplayServices* Dsp = AllocatePool(sizeof(DisplayServices));
	MemoryServices* Mem = AllocatePool(sizeof(MemoryServices));
	IoServices* IO = AllocatePool(sizeof(IoServices));
	DiskServices* Dsk = AllocatePool(sizeof(DiskServices));

	// funcionalidades
	Services.Display = Dsp;
	Services.Memory = Mem;
	Services.InputOutpud = IO;
	Services.File = Dsk;

	// pantalla
	Services.Display->printg_i = &InternalPrintg;
	Services.Display->printg = &InternalPrintgNonLine;
	Services.Display->setCursorPosition = &InternalCursorPos;
	Services.Display->clearScreen = &InternalClearScreen;
	Services.Display->Set = &InternalSetActualDisplayService;
	Services.Display->setAttrs = &InternalSetAttriubtes;

	Services.Display->CurrentLine = 0;
	Services.Display->CurrentCharacter = 0;
	Services.Display->CurrentAttrs = 0;

	// input/outpud
	Services.InputOutpud->Input = &inb;
	Services.InputOutpud->Outpud = &outb;

	// memoria
	Services.Memory->AllocatePool = &AllocatePool;

	// disco
	Services.File->RunFile = &ProcessCrtByFile;
	Services.File->FindFile = &InternalDiskFindFile;
	Services.File->GetFile = &InternalDiskGetFile;

	GlobalServices = &Services;

	return Services;
}

void InternalMiniKernelProgram(KernelServices* Services)
{
	char bufcmd[256];
	int char_set = 0;

	Services->Display->printg("ModuKernel:~# ");
	for (;;) { 
		char key = InternalKeyboardReadChar(); 

		if (key == '\b') {
			if (Services->Display->CurrentCharacter > 0) {
				Services->Display->setCursorPosition(
					Services->Display->CurrentCharacter - 2,
					Services->Display->CurrentLine
				);
				char_set--;
				// opcional: borrar el carácter
				char *vidmem = (char*)0xb8000;
				int pos = Services->Display->CurrentLine * 80 * 2 + Services->Display->CurrentCharacter - 2;
				vidmem[pos] = ' ';
				vidmem[pos+1] = *text_attr;
			}
		}
		else {
			char buff[2] = { key , 0};

			if (key != '\n') bufcmd[char_set] = key;

			Services->Display->printg(buff);
		}

		if (key != 0 && key != '\n') char_set++;

		if (key == '\n')
		{	
			char* cmd = bufcmd;

			if (char_set == 3 ? (memcmp(cmd, "cls", char_set) == 0) : 0) InternalClearScreen();

			Services->Display->printg("ModuKernel:~# ");
			char_set = 0;
		}
	}
}

void k_main() 
{
	// inicializar instancia de kernel
	KernelServices Services = InitializeKernel();

	// inicializar pantalla con la propiedad
	Services.Display->Set(Services.Display);
	// ajustar attributos
	Services.Display->setAttrs(0, 7);
	// limpiar pantalla
	Services.Display->clearScreen();

	// inicializar programa
	InternalMiniKernelProgram(&Services);
}
int ProcessCrtByFile(char* name, char* ext, KernelServices* Services)
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
	// el empiezo de cabezera
    char* p = heap_ptr;
	// si no hay mas espacio
    if (p + size > &_heap_end) {
		// sin memoria
        return 0;
    }
	// aumentar heap
    heap_ptr += size;
	// retornar puntero
    return p;
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
			vidmem[i]=*text_attr; i++;
		};
	};
}
void InternalCursorPos(int x, int y) { *(row_selected) = x; *(line_selected) = y; }
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
		if(*message=='\n') 
		{
			// salto
			(*line_selected)++; i=((*line_selected)*80*2); *message++;
		} else {
			// llenar con el caracter
			vidmem[i]=*message; *message++; i++;

			vidmem[i]=*text_attr; i++;
		};
	};
	// llenar con el atributo de texto
	*row_selected = i;
}
