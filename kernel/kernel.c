// incluir archivos
#include "../services/KernelServices.h"
#include "kernel.h"
#include "../fs/fat12.h"

// incluir funciones y prototipos
#include "../functions/misc_main.h"
#include "../functions/disk_main.h"
#include "../functions/screen_main.h"
#include "../functions/c_main.h"
#include "../functions/heap_main.h"

// incluir funciones de la libreria c de bajo nivel
#include "../libc/String.h"

// funciones del kernel
#include "../services/headers/memory.h"
#include "../services/headers/disk.h"
#include "../services/headers/gdt.h"

// servicios globales
KernelServices* GlobalServices;

// klib
#include "../klib/Klib.h"

// mayusculas y minusculas
bool LowerUpper = 0;
// version del sistema
uint32_t InternalServicesVersion = 26;

// parametros
function** InternalParams;
int32_t InternalParamsCount;

// archivo de shell y directorio
char CurrentWorkDirectory[128] = "/";
uint32_t CwdCurrentCharacter = 1;
int32_t CwdLevelDir = 1;

// el tamaño del programa
int32_t ProgramMainSize = 0;

// pools del kernel
KernelPool* heap_start = (KernelPool*)&_heap_start;
KernelPool* free_list = NULL;

// gdts
struct GDTEntry gdt[23];
struct GDTPtr gdt_ptr;
extern struct GDTPtr gdt_ptr;
uint32_t GdtSetProtectRegion = 3;

// assembly variables
extern uint8_t InternalGrapichalFlag;
extern uint8_t InternalBStg;
extern uint8_t InternalBStgG;
extern uint32_t InternalRingLevel;

// assembly funciones
extern function config_mode();
extern function unconfig_mode();
extern function InternalDrawBackground(uint8_t Color);
extern function InternalGopScreenInit();
extern function InternalSendCharToSerial(char ch);
extern function InternalKernelHaltReal();

void InternalSendCharToSerialFy(char ch) {
    // esperar transmisor listo
    while ((inb(0x3F8 + 5) & 0x20) == 0);

    // enviar carácter
    outb(0x3F8, (uint8_t)ch);
}

void set_gdt_entry(int num, uint32_t base, uint32_t limit, uint8_t access, uint8_t gran)
{
    gdt[num].base_low    = (base & 0xFFFF);
    gdt[num].base_middle = (base >> 16) & 0xFF;
    gdt[num].base_high   = (base >> 24) & 0xFF;

    gdt[num].limit_low   = (limit & 0xFFFF);
    gdt[num].granularity = (limit >> 16) & 0x0F;

    gdt[num].granularity |= (gran & 0xF0);
    gdt[num].access      = access;
}

void InternalProtectRegion(void* ptr, uint32_t size, enum GdtLevelIn level)
{
    uint32_t base  = (uint32_t)ptr;
    uint32_t limit = size - 1;

    uint8_t access;
    if (level == GdtLevelInRing0) {
        // Datos RW, presente, ring 0
        access = 0x92;
    } else {
        // Datos RW, presente, ring 3
        access = 0xF2;
    }

    uint8_t gran = 0xCF; // 4KB granularity, 32-bit

    set_gdt_entry(GdtSetProtectRegion++ , base, limit, access, gran);
}

void InternalDebug(char* str)
{
	char* String = str;

	while (*String) {
		if (*String == '\n')
			InternalSendCharToSerialFy('\r');
		InternalSendCharToSerialFy(*(String++));
	}
}

// pit
#define PIT_FREQUENCY     1193182
#define PIT_PORT_COMMAND  0x43
#define PIT_PORT_CHANNEL0 0x40

void SystemInternalMessage(char* msg)
{
	InternalDebug("\x1B[94m[  \x1B[92mSysInternal  \x1B[94m]     \x1B[0m");
	InternalDebug(msg);
	InternalDebug("\n");
}
void init_gdt() {
    gdt_ptr.limit = (sizeof(gdt) - 1);
    gdt_ptr.base  = (uint32_t)&gdt;

    // Null descriptor
    gdt[0] = (struct GDTEntry){0,0,0,0,0,0};

    // Code segment
    gdt[1].limit_low    = 0xFFFF;
    gdt[1].base_low     = 0x0000;
    gdt[1].base_middle  = 0x00;
    gdt[1].access       = 0x9A;   // code, ring 0
    gdt[1].granularity  = 0xCF;   // 4K granularity, 32-bit
    gdt[1].base_high    = 0x00;

    // Data segment
    gdt[2].limit_low    = 0xFFFF;
    gdt[2].base_low     = 0x0000;
    gdt[2].base_middle  = 0x00;
    gdt[2].access       = 0x92;   // data, ring 0
    gdt[2].granularity  = 0xCF;
    gdt[2].base_high    = 0x00;
}

void pit_init(uint32_t freq) {
    uint16_t divisor = (uint16_t)(PIT_FREQUENCY / freq);

    // Comando: canal 0, acceso low/high, modo 3, binario
    outb(PIT_PORT_COMMAND, 0x36);

    // Enviar divisor (low byte, luego high byte)
    outb(PIT_PORT_CHANNEL0, divisor & 0xFF);
    outb(PIT_PORT_CHANNEL0, (divisor >> 8) & 0xFF);
}
uint16_t pit_read_counter() {
    outb(0x43, 0x00); // latch canal 0
    uint8_t lo = inb(0x40);
    uint8_t hi = inb(0x40);
    return (hi << 8) | lo;
}
void InternalDrawPixel(uint8_t color, int x, int y) 
{
    uint8_t* vram = (uint8_t*)0xA0000;
    vram[y*160 + x] = color;
}
void InternalBlitingRectangle(uint8_t color, int x, int y, int SizeX, int SizeY)
{
	for (size_t yP = 0; yP < SizeY; yP++)
	{
		for (size_t xP = 0; xP < SizeX; xP++) InternalDrawPixel(color, (x + xP), (y + yP));
	}
}
void InternalKernelHalt()
{
	// no se puede en modo usuario
	if (MemoryCurrentSystem == MemAllocTypePrograms || MemoryCurrentSystem == MemAllocTypeProgramsStackMemory) return;

	// llamar a halt
	InternalKernelHaltReal();
}
void DrawBitmap( uint8_t* BitMap, int x, int y, uint8_t color) {
    int height = 7;
    int width  = 6;
    for (int row = 0; row < height; row++) {
        uint8_t line = BitMap[row];
        for (int col = 0; col < width; col++) {
            if (line & (1 << (7 - col))) {   // ← invertimos el orden
				InternalDrawPixel(color, (x + col), y + row);
            }
        }
    }
}
void DrawLetterOffset(int x, int y, char letter, uint8_t color, int ofX, int OfY)
{

	int realx = (x * 5) + ofX;
	int realy = (y * 8) + OfY;

	#include "bitmap.h"

	if (letter == 'a') DrawBitmap(al_bitmap, realx, realy, color);
	else if (letter == 'A') DrawBitmap(au_bitmap, realx, realy, color);
	else if (letter == 'b') DrawBitmap(bl_bitmap, realx, realy, color);
	else if (letter == 'B') DrawBitmap(bu_bitmap, realx, realy, color);
	else if (letter == 'c') DrawBitmap(cl_bitmap, realx, realy, color);
	else if (letter == 'C') DrawBitmap(cu_bitmap, realx, realy, color);
	else if (letter == 'd') DrawBitmap(Dl_bitmap, realx, realy, color);
	else if (letter == 'D') DrawBitmap(Du_bitmap, realx, realy, color);
	else if (letter == 'e') DrawBitmap(El_bitmap, realx, realy, color);
	else if (letter == 'E') DrawBitmap(Eu_bitmap, realx, realy, color);
	else if (letter == 'f') DrawBitmap(Fl_bitmap, realx, realy, color);
	else if (letter == 'F') DrawBitmap(Fu_bitmap, realx, realy, color);
	else if (letter == 'g') DrawBitmap(Gl_bitmap, realx, realy, color);
	else if (letter == 'G') DrawBitmap(Gu_bitmap, realx, realy, color);
	else if (letter == 'h') DrawBitmap(Hl_bitmap, realx, realy, color);
	else if (letter == 'H') DrawBitmap(Hu_bitmap, realx, realy, color);
	else if (letter == 'i') DrawBitmap(Il_bitmap, realx, realy, color);
	else if (letter == 'I') DrawBitmap(Iu_bitmap, realx, realy, color);
	else if (letter == 'j') DrawBitmap(Jl_bitmap, realx, realy, color);
	else if (letter == 'J') DrawBitmap(Ju_bitmap, realx, realy, color);
	else if (letter == 'k') DrawBitmap(Kl_bitmap, realx, realy, color);
	else if (letter == 'K') DrawBitmap(Ku_bitmap, realx, realy, color);
	else if (letter == 'l') DrawBitmap(Ll_bitmap, realx, realy, color);
	else if (letter == 'L') DrawBitmap(Lu_bitmap, realx, realy, color);
	else if (letter == 'm') DrawBitmap(Ml_bitmap, realx, realy, color);
	else if (letter == 'M') DrawBitmap(Mu_bitmap, realx, realy, color);
	else if (letter == 'n') DrawBitmap(Nl_bitmap, realx, realy, color);
	else if (letter == 'N') DrawBitmap(Nu_bitmap, realx, realy, color);
	else if (letter == 'o') DrawBitmap(Ol_bitmap, realx, realy, color);
	else if (letter == 'O') DrawBitmap(Ou_bitmap, realx, realy, color);
	else if (letter == 'p') DrawBitmap(Pl_bitmap, realx, realy, color);
	else if (letter == 'P') DrawBitmap(Pu_bitmap, realx, realy, color);
	else if (letter == 'q') DrawBitmap(Ql_bitmap, realx, realy, color);
	else if (letter == 'Q') DrawBitmap(Qu_bitmap, realx, realy, color);
	else if (letter == 'r') DrawBitmap(Rl_bitmap, realx, realy, color);
	else if (letter == 'R') DrawBitmap(Ru_bitmap, realx, realy, color);
	else if (letter == 's') DrawBitmap(Sl_bitmap, realx, realy, color);
	else if (letter == 'S') DrawBitmap(Su_bitmap, realx, realy, color);
	else if (letter == 't') DrawBitmap(Tl_bitmap, realx, realy, color);
	else if (letter == 'T') DrawBitmap(Tu_bitmap, realx, realy, color);
	else if (letter == 'u') DrawBitmap(Ul_bitmap, realx, realy, color);
	else if (letter == 'U') DrawBitmap(Uu_bitmap, realx, realy, color);
	else if (letter == 'v') DrawBitmap(Vl_bitmap, realx, realy, color);
	else if (letter == 'V') DrawBitmap(Vu_bitmap, realx, realy, color);
	else if (letter == 'w') DrawBitmap(Wl_bitmap, realx, realy, color);
	else if (letter == 'W') DrawBitmap(Wu_bitmap, realx, realy, color);
	else if (letter == 'x') DrawBitmap(Xl_bitmap, realx, realy, color);
	else if (letter == 'X') DrawBitmap(Xu_bitmap, realx, realy, color);
	else if (letter == 'y') DrawBitmap(Yl_bitmap, realx, realy, color);
	else if (letter == 'Y') DrawBitmap(Yu_bitmap, realx, realy, color);
	else if (letter == 'z') DrawBitmap(Zl_bitmap, realx, realy, color);
	else if (letter == 'Z') DrawBitmap(Zu_bitmap, realx, realy, color);
	else if (letter == '[') DrawBitmap(openBracket_bitmap, realx, realy, color);
	else if (letter == ']') DrawBitmap(closeBracket_bitmap, realx, realy, color);
	else if (letter == '(') DrawBitmap(openParen_bitmap, realx, realy, color);
	else if (letter == ')') DrawBitmap(closeParen_bitmap, realx, realy, color);
	else if (letter == '!') DrawBitmap(exclam_bitmap, realx, realy, color);
	else if (letter == '"') DrawBitmap(quote_bitmap, realx, realy, color);
	else if (letter == '#') DrawBitmap(hash_bitmap, realx, realy, color);
	else if (letter == '$') DrawBitmap(dollar_bitmap, realx, realy, color);
	else if (letter == '%') DrawBitmap(percent_bitmap, realx, realy, color);
	else if (letter == '&') DrawBitmap(amp_bitmap, realx, realy, color);
	else if (letter == '/') DrawBitmap(slash_bitmap, realx, realy, color);
	else if (letter == '=') DrawBitmap(equal_bitmap, realx, realy, color);
	else if (letter == ':') DrawBitmap(colon_bitmap, realx, realy, color);
	else if (letter == '0') DrawBitmap(num0_bitmap, realx, realy, color);
	else if (letter == '1') DrawBitmap(num1_bitmap, realx, realy, color);
	else if (letter == '2') DrawBitmap(num2_bitmap, realx, realy, color);
	else if (letter == '3') DrawBitmap(num3_bitmap, realx, realy, color);
	else if (letter == '4') DrawBitmap(num4_bitmap, realx, realy, color);
	else if (letter == '5') DrawBitmap(num5_bitmap, realx, realy, color);
	else if (letter == '6') DrawBitmap(num6_bitmap, realx, realy, color);
	else if (letter == '7') DrawBitmap(num7_bitmap, realx, realy, color);
	else if (letter == '8') DrawBitmap(num8_bitmap, realx, realy, color);
	else if (letter == '9') DrawBitmap(num9_bitmap, realx, realy, color);
	else if (letter == '\a') {
		DrawBitmap(lleno_bitmap, realx, realy - 1, color);
		DrawBitmap(lleno_bitmap, realx + 1, realy - 1, color);
		DrawBitmap(lleno_bitmap, realx, realy, color);
		DrawBitmap(lleno_bitmap, realx + 1, realy, color);
	}
}
void DrawLetter(int x, int y, char letter, uint8_t color)
{
	DrawLetterOffset(x, y, letter, color, 0, 0);
}
uint8_t InternalPixelGetColorOf(int x, int y)
{
	if (!InternalGrapichalFlag) return 0;

	uint8_t* VidMem = (uint8_t *)0xA0000;

	return VidMem[(y * 160) + x];
}
uint8_t ReadRTC(uint8_t reg) 
{ 
	// mandar 0x70 en el rgistro
	outb(0x70, reg); 
	// retornar lo leido
	return inb(0x71);
}
uint8_t BCDtoBin(uint8_t bcd) {
	// retornarlo
	return ((bcd >> 4) * 10) + (bcd & 0x0F);
}
unsigned long long InternalGetNumberOfTicksFromMachineStart()
{
	// parte alta y baja
    unsigned long lo, hi;
	// leer el rdtsc
    __asm__ __volatile__("rdtsc" : "=a"(lo), "=d"(hi));
	// juntarlo y retornarlo
    return (( unsigned long long)hi << 32) | lo;
}
unsigned long long calibrate_tsc() {
    // programa PIT canal 0 para ~50ms
    outb(0x43, 0x34);          // canal 0, modo 2, acceso low/high
    outb(0x40, 0x9B);          // divisor low byte
    outb(0x40, 0x2E);          // divisor high byte (1193180/23863 ≈ 50ms)

    unsigned long long start = InternalGetNumberOfTicksFromMachineStart();

    // espera a que PIT llegue a cero
    while (!(inb(0x61) & 0x20));

    unsigned long long end = InternalGetNumberOfTicksFromMachineStart();

    unsigned long long delta = end - start;
    // 50ms = 0.05s
    return (delta * 20); // escala a Hz
}
void udelay(unsigned long us, unsigned long cpu_freq_hz) {
    unsigned long long start = InternalGetNumberOfTicksFromMachineStart();
    unsigned long long ticks_per_us = cpu_freq_hz / 1000000ULL;
    unsigned long long target = start + (ticks_per_us * us);

    while (InternalGetNumberOfTicksFromMachineStart() < target) {
		__asm__ __volatile__("nop");
    }
}
void InternalModuPanic(KernelStatus Status)
{
	// esto puede ocurrir muy temprano asi que si lo
	// hace muy temprano entonces no hay display entonces
	// solo queda congelarlo
	if (!(GlobalServices && GlobalServices->Display)) while (1);

	// declarar variables para usar durante el ModuPanic

	// el error
	char HexError[9];
	// alineado
	char AlignedErr[9] = "00000000";

	// parsearlo
    IntToHexString(Status, HexError);
	// para evitar errores
	char* PtrHexError = HexError;
	// la longitud
	int LengthHex = StrLen(HexError);
	// el parser
    int StartPparse = LengthHex - 1;

	// parsearlo
	for (int i = 7; i >= 0 && StartPparse >= 0; i--) AlignedErr[i] = HexError[StartPparse--];

	GlobalServices->Display->setAttrs(0, 9);
	GlobalServices->Display->printg("*** stop *** \n\n(Tu sistema ha entrado en un ModuPanic) \n\n ===0x");
	GlobalServices->Display->printg(AlignedErr);
	GlobalServices->Display->printg("===\n\n");

	GlobalServices->Display->printg("*Respuesta personal del sistema*: ");
	switch (Status)
	{
	case KernelStatusSuccess:
		GlobalServices->Display->printg("'mapache que porqueria'");
		break;
	case KernelStatusNotFound:
		GlobalServices->Display->printg("'no se encontro algo que era importante'");
		break;
	case KernelStatusDisaster:
		GlobalServices->Display->printg("'algo hiciste mal y se encabrono el sistema'");
		break;
	case KernelStatusInfiniteLoopTimeouted:
		GlobalServices->Display->printg("'se quedo congelado, te salvaste por poco pero era algo importante y por eso el sistema se disparo al pie'");
		break;
	case KernelStatusNoBudget:
		GlobalServices->Display->printg("'no habia mas memoria'");
		break;
	case KernelStatusMemoryRot:
		GlobalServices->Display->printg("'la memoria se rompio'");
		break;
	case KernelStatusDiskServicesDiskErr:
		GlobalServices->Display->printg("'el disco esta roto'");
		break;
	case KernelStatusInvalidParam:
		GlobalServices->Display->printg("'en alguna cosa importante pusiste datos invalidos o basura'");
		break;
	default:
		GlobalServices->Display->printg("'ni como ayudarte'");
		break;
	}

	while (1)
	{
		char key = GlobalServices->InputOutput->WaitKey();

		if (key == '\n') return;
		else if (key == 'r' || key == 'R') GlobalServices->Misc->Reset(0);
	}
}
void PrintStatus(KernelServices* Services, char* status, char* text)
{
	// empezar
	Services->Display->printg("[ ");
	// atributo
	Services->Display->setAttrs(0, 0x3);
	// estado
	Services->Display->printg(status);
	// desatributo
	Services->Display->setAttrs(0, 0x7);
	// terminar
	Services->Display->printg(" ]    ");
	// imprimir texto
	Services->Display->printg(text);
}
void InternalGetDateTime(KernelDateTime* Time) {
	// segundo
    Time->second = BCDtoBin(ReadRTC(0x00));
	// minuto
    Time->minute = BCDtoBin(ReadRTC(0x02));
	// hora
    Time->hour   = BCDtoBin(ReadRTC(0x04));
	// dia
    Time->day    = BCDtoBin(ReadRTC(0x07));
	// mes
    Time->month  = BCDtoBin(ReadRTC(0x08));
	// año
    Time->year   = 2000 + BCDtoBin(ReadRTC(0x09));
}
void InternalWaitEticks(int Unities)
{
	// esperar
	for (size_t i = 0; i < (5999999 * Unities); i++) { 
		// para delay
		int ala = ((10/2)*3); 
	}
}
void InitHeap() 
{
    // Dirección base del heap
    char* heap_start = &_heap_start;
    char* heap_end   = &_heap_end;

    // Alinea si lo deseas (opcional, por ejemplo a 8 o 16 bytes)
    //heap_start = (char*)(((unsigned int)heap_start + 7) & ~7);

    // Crear bloque único
    KernelPool* first = (KernelPool*)heap_start;

    unsigned int total_bytes = (unsigned int)(heap_end - heap_start);
    unsigned int header_size = sizeof(KernelPool);

    // Verificación mínima
    if (total_bytes <= header_size) {
        // No hay heap suficiente
        free_list = NULL;
        return;
    }

    first->size = total_bytes - header_size; // parte útil
    first->free = 1;
    first->next = NULL;

    free_list = first;
}
KernelStatus InternalFloppyDiskReadSector(unsigned int lba, unsigned char* buffer_phys /* debe ser física y <1MiB */)
{
    // 1. LBA -> CHS
    unsigned int cyl = lba / (2 * 18);
    unsigned int head = (lba / 18) % 2;
    unsigned int sec = (lba % 18) + 1;

    // 2. Encender motor y seleccionar unidad 0
    outb(FDC_DOR, 0x1C); // motor A on, DMA/IRQ enable, drive 0

    // 3. Data rate 500 kbps
    outb(FDC_DSR, 0x00);

    // 4. CONFIGURE (0x13): FIFO habilitado, threshold 8, implied seek off
    fdc_write(0x13);
    fdc_write(0x00); // msr config
    fdc_write(0x0F); // FIFO threshold
    fdc_write(0x00); // precomp

    // 5. SPECIFY (0x03): tiempos
    fdc_write(0x03);
    fdc_write(0xDF); // SRT=13ms, HUT=240ms (ejemplo)
    fdc_write(0x02); // HLT=2ms, ND=0

    // 6. RECALIBRATE (0x07) unidad 0 y esperar IRQ6 (usa tu handler/flag)
    fdc_write(0x07);
    fdc_write(0x00);
    // espera_irq6();

    // 7. SEEK (0x0F) al cilindro
    fdc_write(0x0F);
    fdc_write((head << 2) | 0x00); // head+drive
    fdc_write(cyl);
    // espera_irq6();

    // 8. Programar DMA canal 2 para leer 512 bytes a buffer_phys
    // Nota: buffer debe estar en memoria baja física (<1MiB), alineado.
    outb(DMA_MASK, 0x06);       // mask canal 2
    outb(DMA_CLEARFF, 0x00);    // clear flip-flop
    outb(DMA_MODE, 0x56);       // canal 2, read, single, address inc
    outb(DMA_CH2_ADDR, (uint8_t)( (uint32_t)buffer_phys & 0xFF ));
    outb(DMA_CH2_ADDR, (uint8_t)( ((uint32_t)buffer_phys >> 8) & 0xFF ));
    outb(DMA_CH2_PAGE, (uint8_t)( ((uint32_t)buffer_phys >> 16) & 0xFF ));
    outb(DMA_CLEARFF, 0x00);
    outb(DMA_CH2_COUNT, 0xFF);  // 512-1 = 0x1FF -> low
    outb(DMA_CH2_COUNT, 0x01);  // high
    outb(DMA_MASK, 0x02);       // unmask canal 2

    // 9. READ DATA (0x06)
    fdc_write(0x06);
    fdc_write((head << 2) | 0x00); // head+drive
    fdc_write(cyl);
    fdc_write(head);
    fdc_write(sec);
    fdc_write(0x02); // tamaño = 512 bytes (2^2)
    fdc_write(18);   // EOT
    fdc_write(0x1B); // GAP3
    fdc_write(0xFF); // DTL

    // 10. Esperar IRQ6 que indica finalización de la transferencia
    // espera_irq6();

    // 11. Leer result bytes (7) para chequear estado
    uint8_t st0 = fdc_read();
    uint8_t st1 = fdc_read();
    uint8_t st2 = fdc_read();
    uint8_t st3 = fdc_read();
    uint8_t rcyl = fdc_read();
    uint8_t rhead = fdc_read();
    uint8_t rsec = fdc_read();
    // Validar ST1/ST2 sin errores, y posición devuelta

    // 12. Apagar motor si deseas
    outb(FDC_DOR, 0x0C);

    return KernelStatusSuccess;
}
KernelStatus InternalCdRomDiskReadSector(unsigned int lba, unsigned char* buffer)
{
    // 1. Seleccionar unidad ATAPI (maestro/esclavo)
    outb(0x1F6, 0xA0); // maestro ATAPI

    // 2. Enviar PACKET command
    outb(0x1F7, 0xA0);

    // 3. Esperar DRQ (device ready to receive packet)
    int timeout = 100000;
    while (!(inb(0x1F7) & 0x08) && timeout--) {
        if (timeout < 1) return KernelStatusInfiniteLoopTimeouted;
    }

    // 4. Construir paquete SCSI READ(10) (12 bytes)
    uint8_t packet[12] = {0};
    packet[0] = 0x28; // READ(10)
    packet[2] = (lba >> 24) & 0xFF;
    packet[3] = (lba >> 16) & 0xFF;
    packet[4] = (lba >> 8) & 0xFF;
    packet[5] = lba & 0xFF;
    packet[9] = 1; // número de bloques a leer

    // 5. Escribir el paquete en el puerto de datos (0x1F0)
    for (int i = 0; i < 6; i++) {
        outw(0x1F0, ((uint16_t*)packet)[i]);
    }

    // 6. Esperar DRQ para datos
    timeout = 100000;
    while (!(inb(0x1F7) & 0x08) && timeout--) {
        if (timeout < 1) return KernelStatusInfiniteLoopTimeouted;
    }

    // 7. Leer 2048 bytes (1024 palabras)
    for (int i = 0; i < 1024; i++) {
        ((uint16_t*)buffer)[i] = inw(0x1F0);
    }

    return KernelStatusSuccess;
}
KernelStatus InternalAtaDiskReadSector(unsigned int lba, unsigned char* buffer) 
{
	// lo lee 1
    outb(0x1F6, 0xE0 | ((lba >> 24) & 0x0F)); 
	// lo lee 2
    outb(0x1F2, 1); outb(0x1F3, (uint8_t) lba); outb(0x1F4, (uint8_t)(lba >> 8)); 
	// lo lee 3
	outb(0x1F5, (uint8_t)(lba >> 16)); outb(0x1F7, 0x20);     

	// tiempo de acabado
	int timeout = 100000;

    // esperar a que DRQ esté listo
    while (!(inb(0x1F7) & 0x08) && (timeout--)) if (timeout < 1) return KernelStatusInfiniteLoopTimeouted;

    // leer 256 palabras (512 bytes)
    for (int i = 0; i < 256; i++) ((uint16_t*)buffer)[i] = inw(0x1F0);

	// retornar status
	return KernelStatusSuccess;
}
KernelStatus InternalRealDiskReadSector(unsigned int lba, unsigned char* buffer, DiskTypePort FileSystem)
{
	return 	(
			(FileSystem == DiskTypeHardDisk) ? InternalAtaDiskReadSector(lba, buffer) : 
			(FileSystem == DiskTypeFloppy) ? InternalFloppyDiskReadSector(lba, buffer) :
			(FileSystem == DiskTypeCdRom) ? InternalCdRomDiskReadSector(lba, buffer) :
			KernelStatusInvalidParam);
}
KernelStatus InternalDiskReadSector(unsigned int lba, unsigned char* buffer) 
{
	InternalRealDiskReadSector(lba, buffer, SystemCwkDisk);
}
void InternalCloseFile(FatFile File)
{
	FreePool(File.dir);
	FreePool(File.bs);
}
FatFile InternalDiskFindFile(char* name, char* ext) 
{
	// sector
    uint8_t sector0[512];
	// leerlo
    InternalDiskReadSector(0, sector0);

	// estructura
    struct _FAT12_BootSector* bs_local = (struct _FAT12_BootSector*) sector0;

	// Copia el boot sector a heap (memoria persistente)
    struct _FAT12_BootSector* bs = (struct _FAT12_BootSector*) AllocatePool(sizeof(*bs));
    if (!bs) {
		// vacio
        FatFile empty = {0};
		// retornar vacio
        return empty;
    }
	// copiar
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

	// leer sector
    for (int s = 0; s < root_sectors; s++) InternalDiskReadSector(root_start + s, root_buffer + s * bs->bytes_per_sector);

	// entrada
    struct _FAT12_DirEntry* dir = (struct _FAT12_DirEntry*) root_buffer;
	// entradas totales
    int total_entries = bs->root_entries;

	// ver
    for (int i = 0; i < total_entries; i++) {
		// si no existe o elminado
        if (dir[i].name[0] == 0x00 || dir[i].name[0] == 0xE5) continue;

		// si coincide el nombre
        if (InternalMemoryComp(dir[i].name, name, 8) == 0 &&
            InternalMemoryComp(dir[i].name + 8, ext, 3) == 0) {
			// estructura
            FatFile File;
			// archivo
            File.bs = bs; 
			// sector
            File.sector = dir[i];
			// dir
			File.dir = dir;
            // retornarlo
            return File;
        }
    }

    // no encontrado
    FatFile empty = {0};
	// retornar
    return empty;
}
void InternalMusicSetTone(unsigned int freq) {
    if (freq == 0) return;

    // calcular divisor
    uint32_t divisor = 1193180 / freq;

    // configurar PIT canal 2 (modo 3: square wave)
    outb(0x43, 0xB6);              // comando: canal 2, modo 3, acceso bajo/alto
    outb(0x42, (uint8_t)(divisor & 0xFF));       // parte baja
    outb(0x42, (uint8_t)((divisor >> 8) & 0xFF)); // parte alta

    // activar speaker (bits 0 y 1 del puerto 0x61)
    uint8_t tmp = inb(0x61);
    if ((tmp & 3) != 3) {
        outb(0x61, tmp | 3);
    }
}
void InternalMusicMuteTone() {
    uint8_t tmp = inb(0x61) & 0xFC; // limpiar bits 0 y 1
    outb(0x61, tmp);
}
unsigned int InternalGetFreeHeapSpace() 
{
    unsigned int total = 0;
    KernelPool* current = free_list;

    while (current) {
        if (current->free) {
            total += current->size;
        }
        current = current->next;
    }

    return total;
}
char* InternalReadLine()
{
	// buffers
	char bufcmd[256]; int char_set = 0;
	for (;;)
	{
		// tecla
		char key = GlobalServices->InputOutput->WaitKey(); 

		// si es backspace
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
			// escapes de funciones
			if (key == KernelSimpleIoFuncKey(1)) key = '_';
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
			// crearlo
			char* retval = AllocatePool(sizeof(char) * 256);
			// copiarlo
			for (short index = 0; index < 256; index++) retval[index] = bufcmd[index];
			// retornarlo
			return retval;
		}
	}
}
KernelStatus InternalDiskGetFile(FatFile file, void** content, int* size)
{
	// si el archivo esta eliminado no se esta
    if (file.sector.name[0] == 0x00) return KernelStatusNotFound;

	// sector
    struct _FAT12_BootSector* bs = file.bs;
	// directorio
    struct _FAT12_DirEntry dir   = file.sector;

	// leer fat
    unsigned int fat_bytes = bs->fat_size_sectors * bs->bytes_per_sector;
	// fat
    uint8_t* fat = (uint8_t*) AllocatePool(fat_bytes);
	// si no hay presupuesto
    if (!fat) return KernelStatusNoBudget;

    for (unsigned int s = 0; s < bs->fat_size_sectors; s++)
    {
        KernelStatus st = InternalDiskReadSector(bs->reserved_sectors + s,
                         fat + s * bs->bytes_per_sector);

		// si hubo un error retornar el status
        if (_StatusError(st)) return st;
    }

	// data region
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
		// lba
        unsigned int lba = data_start + (cluster - 2) * bs->sectors_per_cluster;

        for (unsigned int s = 0; s < bs->sectors_per_cluster && remaining > 0; s++)
        {
            KernelStatus st = InternalDiskReadSector(lba + s, sector);

			// si hay error retornarlo
            if (_StatusError(st)) return st;

			// copia
            unsigned int copy = (remaining > bs->bytes_per_sector ? bs->bytes_per_sector : remaining);

			// copiar memoria
            InternalMemoryCopy(out + offset, sector, copy);

			// obtenerlo
            offset    += copy;
            remaining -= copy;
        }

        // siguiente cluster
        cluster = get_fat_entry(cluster, fat);
    }

    *content = out;
    *size    = dir.file_size;

	FreePool(fat);
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
	// deshabilitar interrupciones
    __asm__ __volatile__("cli");

	// funcion de apagar
    if (func == 1) 
	{
		// opcion 1
        outw(0x604, 0x2000);
		// opcion 2
        outw(0xB004, 0x2000);

		// si no tripe fault
        goto triple_fault;
    }

	// esperar no se que cosa
    while (inb(0x64) & 0x02);

	// reiniciar
	outb(0x64, 0xFE);

	// tripe error
	triple_fault:
		// retornar que fue un desastre
		return KernelStatusDisaster;
}
unsigned char InternalKeyboardReadChar()
{
	// si es extendido
	int extended = 0;
	// esperar
	while(1) {
		// estado
		uint8_t status = inb(0x64);

		// si es verdadero
		if(status & 0x01) {
			// codigo de escaneo
			uint8_t scancode = inb(0x60);

			// caracter
			char character = 0;

			// codigos

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
			else if(scancode == 0x35) character = LowerUpper ? '<' : '/';
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
unsigned char InternalKeyboardReadCharNonBlocking() {
    uint8_t status = inb(0x64);

    if (!(status & 0x01)) {
        return 0; // no hay tecla disponible
    }

    uint8_t scancode = inb(0x60);
    static int extended = 0;
    char character = 0;

    if (scancode == 0xE0) {
        extended = 1;
        return 0;
    }

    if (extended) {
        extended = 0;
        if (scancode == 0x48) return KernelSimpleIoSpecKey(1); // arriba
        if (scancode == 0x50) return KernelSimpleIoSpecKey(2); // abajo
        if (scancode == 0x4B) return KernelSimpleIoSpecKey(3); // izquierda
        if (scancode == 0x4D) return KernelSimpleIoSpecKey(4); // derecha
        return 0;
    }

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
	else if(scancode == 0x35) character = LowerUpper ? '<' : '/';
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

    return LowerUpper ? CharToUpCase(character) : character;
}
void InitializeKernel(KernelServices* Services)
{
	//putpixel(10, 10, 255);

    // opcional: limpia la estructura principal (si vive en stack)
    InternalMemorySet(Services, 0, sizeof(KernelServices));

    // reservar subestructuras
    DisplayServices* 	Dsp 	= AllocatePool(sizeof(DisplayServices));
    MemoryServices* 	Mem 	= AllocatePool(sizeof(MemoryServices));
    IoServices* 		IO     	= AllocatePool(sizeof(IoServices));
    DiskServices* 		Dsk  	= AllocatePool(sizeof(DiskServices));
    KernelMiscServices* Msc 	= AllocatePool(sizeof(KernelMiscServices));
    TimeServices* 		Tim 	= AllocatePool(sizeof(TimeServices));
    MusicServices* 		Music 	= AllocatePool(sizeof(MusicServices));
	KernelSystemInfo* 	Info 	= AllocatePool(sizeof(KernelSystemInfo));

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
	InternalMemorySet(Tim, 0, sizeof(TimeServices));
	InternalMemorySet(Music,0, sizeof(MusicServices));
	InternalMemorySet(Info, 0, sizeof(KernelSystemInfo));

    // conectar subestructuras
    Services->Display 		= Dsp;
    Services->Memory  		= Mem;
    Services->InputOutput 	= IO;
    Services->File    		= Dsk;
    Services->Misc    		= Msc;
	Services->Time			= Tim;
	Services->Music			= Music;
	Services->Info			= Info;

    // servicios principales
    Services->Misc->Run       = &InternalSysCommandExecute;
	Services->Misc->Reset	  = &InternalKernelReset;
	Services->Misc->RunBinary = &InternalRunBinary;
	Services->Misc->GetTime   = &InternalGetDateTime;
	Services->Misc->Paramaters= &InternalParams;
	Services->Misc->ParamsCount=&InternalParamsCount;
	Services->Misc->GetTicks  = &InternalGetNumberOfTicksFromMachineStart;
	Services->Misc->Throw 	  = &InternalModuPanic;
	Services->Misc->Halt	  = &InternalKernelHalt;
    Services->ServicesVersion = &InternalServicesVersion;

    // pantalla
    Dsp->printg_i          = &InternalPrintg;
    Dsp->printg            = &InternalPrintgNonLine;
    Dsp->setCursorPosition = &InternalCursorPos;
    Dsp->clearScreen       = &InternalClearScreen;
    Dsp->Set               = &InternalSetActualDisplayService;
    Dsp->setAttrs          = &InternalSetAttriubtes;
	Dsp->ActivatePixel	   = &InternalGopScreenInit;
	Dsp->DrawRectangle	   = &InternalDrawPixel;
	Dsp->DrawBackgroundColor=&InternalDrawBackground;
	Dsp->DrawLetter 	   = &DrawLetterOffset;
	Dsp->GetPixel		   = &InternalPixelGetColorOf;
	Dsp->Blt			   = &InternalBlitingRectangle;

    Dsp->CurrentLine      = 0;
    Dsp->CurrentCharacter = 0;
    Dsp->CurrentAttrs     = 0;
	Dsp->IsInPixelsMode   = &InternalGrapichalFlag;

    // IO
    IO->Input   = &inb;
    IO->Outpud  = &outb;
    IO->WaitKey = &InternalKeyboardReadChar;
    IO->ReadLine= &InternalReadLine;
	IO->ReadKey = &InternalKeyboardReadCharNonBlocking;

    // memoria
    Mem->AllocatePool = &AllocatePool;
    Mem->MoveMemory   = &InternalMemMove;
    Mem->CoppyMemory  = &InternalMemoryCopy;
    Mem->CompareMemory= &InternalMemoryComp;
    Mem->FreePool     = &FreePool;
	Mem->GetFreeHeap  = &InternalGetFreeHeapSpace;
	Mem->MallocType	  = &MemoryCurrentSystem;
	Mem->RepairMemory = &InternalSleepDream;
	Mem->MemorySet	  = &InternalMemorySet;

    // disco
    Dsk->RunFile    	= &ProcessCrtByFile;
    Dsk->FindFile   	= &InternalDiskFindFile;
    Dsk->GetFile    	= &InternalDiskGetFile;
    Dsk->ReadSector 	= &InternalDiskReadSector;
	Dsk->OpenFile 		= &InternalExtendedFindFile;
	Dsk->CloseFile		= &InternalCloseFile;

	Dsk->CurrentDiskType= &SystemCwkDisk;

	// tiempo
	Tim->TaskDelay 	= &InternalWaitEticks;

	// musica
	Music->PlayTone	= &InternalMusicSetTone;
	Music->Mute		= &InternalMusicMuteTone;

	// informacion
	Info->ModuWorldPtr  = ((uint8_t*)0x0AFB032C);
	Info->ProgramSizePtr= &ProgramMainSize;
	Info->VideoMemPtr	= ((uint8_t*)0xb8000);

    // hacer global la estructura principal
    GlobalServices = Services;
}
FatFile InternalExtendedFindFile(char* path)
{
    // Abrir tabla FSLST.IFS
    FatFile file = GlobalServices->File->FindFile("FSLST   ", "IFS");

    void* buffer;
    int size;

    KernelStatus status = GlobalServices->File->GetFile(file, &buffer, &size);

    if (!_StatusError(status))
    {
        char* text = (char*)buffer;
        char* parts[128]; // espacio para líneas

        int n = StrSplit(text, parts, '\n');

        for (int i = 0; i < n; i++)
        {
            char* part = parts[i];
            if (StrCmp(part, "") == 0) continue;

            char* entry[3];
            int entryLen = StrSplit(part, entry, ';');

            if (entryLen != 3) continue;

            char* route = entry[0];
            char* nameRaw = entry[1];
            char* extRaw  = entry[2];

            // comparar ruta lógica con path
            if (StrCmp(route, path) == 0)
            {
                char name[9]; InternalMemorySet(name, ' ', 8); name[8] = '\0';
                for (int j = 0; j < StrLen(nameRaw) && j < 8; j++) name[j] = nameRaw[j];

                char ext[4]; InternalMemorySet(ext, ' ', 3); ext[3] = '\0';
                for (int j = 0; j < StrLen(extRaw) && j < 3; j++) ext[j] = extRaw[j];

				char* nm = AllocatePool(sizeof(char) * 9);
				char* xt = AllocatePool(sizeof(char) * 4);
				InternalMemoryCopy(nm, name, (sizeof(char) * 9));
				InternalMemoryCopy(xt, ext, (sizeof(char) * 4));

        		GlobalServices->Memory->FreePool(buffer);
				FatFile FileFinded = GlobalServices->File->FindFile(nm, xt);

				FreePool(nm);
				FreePool(xt);
				return FileFinded;
            }
        }

        GlobalServices->Memory->FreePool(buffer);
    }
	else
	{
	    GlobalServices->Memory->FreePool(buffer);

		char* entry[2];
	    int entryLen = StrSplit(path, entry, '.');

		char* nameRaw = entry[0];

		if (nameRaw[0] == '/') nameRaw++;

        char* extRaw  = entry[1];

		char name[9]; InternalMemorySet(name, ' ', 8); name[8] = '\0';
        for (int j = 0; j < StrLen(nameRaw) && j < 8; j++) name[j] = nameRaw[j];

        char ext[4]; InternalMemorySet(ext, ' ', 3); ext[3] = '\0';
        for (int j = 0; j < StrLen(extRaw) && j < 3; j++) ext[j] = extRaw[j];

		char* nm = StrUpr(name);
		char* xt = StrUpr(ext);

        return GlobalServices->File->FindFile(nm, xt);
	}

	FatFile fileNull = {0};
    return fileNull; // no encontrado
}
KernelStatus InternalRunBinary(void* buffer, int size, KernelServices* Services) {
	// bandera de si salio mas pequeño que el header
	bool ProgramIsTooSmall = 0;
	// bandera de que si el header fue invalido
	bool InvalidHeaderFlag = 0;

	// depurar
	SystemInternalMessage("Cargando programa");

	// incrementar nivel de privilegios
	InternalRingLevel++;

	// datos y cosas
	uint8_t* ProgramDataAndThings = 0;
	// si hacer paginacion
	uint8_t MakePagingEmulator = 0; 

	// longitud anterior del programa
	int OldSizeProgram = ProgramMainSize;

	// depurar
	SystemInternalMessage("Verificando pila de programas y ejecucion para apilar");	

	// si ya esta en un programa
	if (MemoryCurrentSystem == MemAllocTypePrograms) MakePagingEmulator = 1;

	// donde carga el programa
	#define USER_LOAD_ADDR Services->Info->ModuWorldPtr

	// si se va a hacer paginacion
	if (MakePagingEmulator) 
	{
		// depurar
		SystemInternalMessage("Se necesita copiar el programa a la pila de usuario");	

		// hacer paginacion
		ProgramDataAndThings = (uint8_t*)InternalAllocatePool(OldSizeProgram, MemAllocTypeProgramsStackMemory);
		
		// depurar
		SystemInternalMessage("Pila creada");	

		// copiar
		InternalMemoryCopy(ProgramDataAndThings, USER_LOAD_ADDR, OldSizeProgram);

		// depurar
		SystemInternalMessage("Programa anterior guardado con exito");	
	}

	// la longitud
	ProgramMainSize = size;

	// memoria antigua
	ModuAllocType OldMallocType = MemoryCurrentSystem;

	// memoria de usuario
	MemoryCurrentSystem = MemAllocTypePrograms;

	// estructura del tipo de programa
    typedef struct {
		// "ModuBin\0"
        char magic[8];
		// dirección de ErickMain
        uint32_t entry;
		// inicio .bss
        uint32_t bss_start;
		// fin .bss
        uint32_t bss_end;
    } UserHeader;

	// si es menor el tamaño
    if (size < sizeof(UserHeader)) 
	{
		ProgramIsTooSmall = 1;
		goto end_program;
	}

	// depurar
	SystemInternalMessage("Copiando programa a la memoria");	

	// copiar al user load address el buffer
    InternalMemoryCopy(USER_LOAD_ADDR, buffer, size);

	// el header
    UserHeader* hdr = (UserHeader*)USER_LOAD_ADDR;

	// depurar
	SystemInternalMessage("Verificando header...");	

	// parametro invalido
    if (Services->Memory->CompareMemory(hdr->magic, "ModuBin", 7) != 0) 
	{
		InvalidHeaderFlag = 1;
		goto end_program;
	}

	// depurar
	SystemInternalMessage("Verificando bss...");	

	// si es mayor el end que el start
    if (hdr->bss_end > hdr->bss_start) {
		// tamaño
        size_t bss_size = (size_t)(hdr->bss_end - hdr->bss_start);
		// setear memoria
        InternalMemorySet((void*)hdr->bss_start, 0, bss_size);
    }

	// depurar
	SystemInternalMessage("Ejecutando programa...");	

	// el tipo de programa
    typedef KernelStatus (*ProgramEntry)(KernelServices*);

	// entrada
    ProgramEntry entry = (ProgramEntry)(uintptr_t)hdr->entry;

	// la ejecucion
	KernelStatus Status = entry(Services);

end_program:

	// cambiar a ejecucion anterior
	MemoryCurrentSystem = OldMallocType;

	// setear la longitud
	ProgramMainSize = OldSizeProgram;

	// regresarlo
	if (MakePagingEmulator) 
	{
		// depurar
		InternalDebug("[SysInternal]    Cargando programa anterior...");	

		// copiarlo de nuevo
		InternalMemoryCopy(USER_LOAD_ADDR, ProgramDataAndThings, OldSizeProgram);
	
		// liberarlo
		FreePool(ProgramDataAndThings);

		// depurar
		SystemInternalMessage("Cargado con exito...");	
	}

	// decrementar nivel de privilegios
	InternalRingLevel--;

	// si fue menor
	if (ProgramIsTooSmall) return KernelStatusThingVerySmall;

	// si la bandera fue invalida
	if (InvalidHeaderFlag) return KernelStatusInvalidParam;

	// retornar el status
    return Status;
}
void InternalExecuteScript(char* buffer)
{
	int StackI[100];
	int StackLine = 0;

	char* text = (char*)buffer;
	char* parts[512];
    int n = StrSplit(text, parts, '\n');
    for (int i = 0; i < n; i++) 
	{
		char* Line = StrTrim(parts[i]);

		if (StrnCmp(Line, "call ", 5) == 0)
		{
			char* Function = Line + 5;
			int NewLen = (sizeof(char) * (StrLen(Function) + 2));

			char* fnline = AllocatePool(NewLen);

			InternalMemoryCopy(fnline, Function, NewLen);

			fnline[NewLen - 1] = 0;
			fnline[NewLen - 2] = ':';

			bool Founded = 0;
			int LineIn = 0;

			for (size_t j = 0; j < n; j++)
			{
				char* Line2 = StrTrim(parts[j]);

				if (StrCmp(Line2, fnline) == 0)
				{
					Founded = 1;
					LineIn = j;
				}
			}

			if (Founded)
			{
				int OldLine = i;
				StackI[StackLine++] = OldLine;
				i = LineIn;
			}
		}
		else if (StrnCmp(Line, "jmp ", 4) == 0)
		{
			char* Function = Line + 4;
			int NewLen = (sizeof(char) * (StrLen(Function) + 2));

			char* fnline = AllocatePool(NewLen);

			InternalMemoryCopy(fnline, Function, NewLen);

			fnline[NewLen - 1] = 0;
			fnline[NewLen - 2] = ':';

			bool Founded = 0;
			int LineIn = 0;

			for (size_t j = 0; j < n; j++)
			{
				char* Line2 = StrTrim(parts[j]);

				if (StrCmp(Line2, fnline) == 0)
				{
					Founded = 1;
					LineIn = j;
				}
			}

			if (Founded)
			{
				i = LineIn;
			}
		}
		else if (StrCmp(Line, "ret") == 0) i = StackI[StackLine--];
		else InternalSysCommandExecute(GlobalServices, Line, 0);
	}
}
void InternalDumpHexMemory(void* buffer, size_t size) {
    uint8_t* data = (uint8_t*)buffer;

    for (size_t i = 0; i < size; i += 8) {
        // 1. imprimir dirección
        char addr[16];
        IntToHexString((uint32_t)i, addr);
        GlobalServices->Display->printg(addr);
		GlobalServices->Display->CurrentCharacter = 10;

        // 2. imprimir 8 bytes en hex
        for (size_t j = 0; j < 8; j++) {
            if (i + j < size) {
                char hex[4];
                IntToHexString(data[i + j], hex);
                // asegurar que sean 2 dígitos
                if (StrLen(hex) == 1) {
                    GlobalServices->Display->printg("0");
                }
                GlobalServices->Display->printg(hex);
                GlobalServices->Display->printg(" ");
            } else {
                // relleno si no hay más datos
                GlobalServices->Display->printg("   ");
            }
        }

        // 3. imprimir ASCII entre corchetes
        GlobalServices->Display->printg(" [");
        for (size_t j = 0; j < 8; j++) {
            if (i + j < size) {
                char c = data[i + j];
                char em[2] = { (c >= 0x20 && c <= 0x7E) ? c : '.', 0 };
                GlobalServices->Display->printg(em);
            } else {
                GlobalServices->Display->printg(" ");
            }
        }
        GlobalServices->Display->printg("]\n");
    }
}
void InternalSysCommandExecute(KernelServices* Services, char* command, int lena)
{
	int len = StrLen(command);

	SystemInternalMessage(command);

	if (StrCmp(command, "cls") == 0) Services->Display->clearScreen();
	else if (command[0] == '#') return;
	else if (StrnCmp(command, "cd ", 3) == 0)
	{
		char* directory = command + 3;

		// Si es exactamente "cd /"
		if (StrCmp(directory, "/") == 0) {
			CurrentWorkDirectory[0] = '/';
			CurrentWorkDirectory[1] = 0;
			CwdCurrentCharacter = 1;
			CwdLevelDir = 0;
			return;
		}

		// Si es "cd .."
		if (StrCmp(directory, "..") == 0) {
			if (CwdCurrentCharacter > 1) {
				CurrentWorkDirectory[(CwdCurrentCharacter--)-1] = 0;
				while (CwdCurrentCharacter > 0 && CurrentWorkDirectory[CwdCurrentCharacter] != '/')
					CurrentWorkDirectory[CwdCurrentCharacter--] = 0;

				if (CwdCurrentCharacter == 0) CurrentWorkDirectory[1] = 0;
				else CurrentWorkDirectory[CwdCurrentCharacter+1] = 0;

				CwdCurrentCharacter++;
			}

			CwdLevelDir--;
			return;
		}

		// Si es "cd /algo"
		if (directory[0] == '/') {
			CurrentWorkDirectory[0] = '/';
			CurrentWorkDirectory[1] = 0;
			CwdCurrentCharacter = 1;
			CwdLevelDir = 0;
			directory++; // saltar el slash inicial
		}

		if (directory[0] == 0) return;

		// Agregar el nuevo subdirectorio
		unsigned int DirectoryLen = StrLen(directory);
		for (int i = 0; i < DirectoryLen; i++)
			CurrentWorkDirectory[CwdCurrentCharacter++] = directory[i];

		CurrentWorkDirectory[CwdCurrentCharacter++] = '/';
		CurrentWorkDirectory[CwdCurrentCharacter] = 0;
		CwdLevelDir++;
	}
	else if (StrCmp(command ,"ls") == 0)
	{ 
		FatFile StructureFs = Services->File->FindFile("FSLST   ", "IFS");
		void* StructFs; int FsSize;
		KernelStatus StructureFound = Services->File->GetFile(StructureFs, &StructFs, &FsSize);

		char Recorrer = 0;
		char MaxRecorrer = 5;
		char* Directory = CurrentWorkDirectory;

		if (!_StatusError(StructureFound))
		{
			char* text = (char*)StructFs;
			char* parts[128]; // espacio para líneas

			char* putedDirs[128];
			int indexPutDirs = 0;

			int n = StrSplit(text, parts, '\n');

			for (int i = 0; i < n; i++)
			{
				char* part = parts[i];
				if (StrCmp(part, "") == 0) continue;

				char* entry[3];
				int entryLen = StrSplit(part, entry, ';');

				if (entryLen != 3) continue;

				char* route = entry[0];
				char* nameRaw = entry[1];
				char* extRaw  = entry[2];

				char name[9]; InternalMemorySet(name, ' ', 8); name[8] = '\0';
				for (int j = 0; j < StrLen(nameRaw) && j < 8; j++) name[j] = nameRaw[j];

				char ext[4]; InternalMemorySet(ext, ' ', 3); ext[3] = '\0';
				for (int j = 0; j < StrLen(extRaw) && j < 3; j++) ext[j] = extRaw[j];

				char* ExtensionRt = ext;

				if (StrCmp(ExtensionRt, "NSH") == 0) Services->Display->setAttrs(0, 0x3);
				else if (StrCmp(ExtensionRt, "BIN") == 0) Services->Display->setAttrs(0, 0x2);
				else Services->Display->setAttrs(0, 0x7);

				if (StrnCmp(route, CurrentWorkDirectory, CwdCurrentCharacter) == 0) {
					char* rtm = route + CwdCurrentCharacter;

					// ¿hay otro '/' en rtm?
					char* slash = StrChr(rtm, '/');
					if (slash) {
						int len = slash - rtm;
						char* temp = AllocatePool(len + 1);
						InternalMemoryCopy(temp, rtm, len);
						temp[len] = '\0';

						int founded = 0;
						for (int a=0; a < indexPutDirs; a++) {
							if (StrCmp(putedDirs[a], temp) == 0) {
								founded = 1;
								FreePool(temp);
								break;
							}
						}

						if (!founded) {
							Services->Display->setAttrs(0, 9);

							putedDirs[indexPutDirs++] = temp;
							Services->Display->printg(temp);
							Services->Display->printg((*Services->Display->IsInPixelsMode) ? "/" : "/    ");
							Recorrer++;
						} else {
							GlobalServices->Memory->FreePool(temp); // no lo guardes si ya estaba
						}
					} else {
						Services->Display->printg(rtm);
						Services->Display->printg((*Services->Display->IsInPixelsMode) ? "" : "    ");
						Recorrer++;
					}

					if (Recorrer == MaxRecorrer)
					{
						Recorrer = 0;
						if ((*Services->Display->IsInPixelsMode) == 0) Services->Display->printg("\n");
					}

					if ((*Services->Display->IsInPixelsMode)) Services->Display->printg("  ");

				}

			}

			GlobalServices->File->CloseFile(StructureFs);

			for (size_t i = 0; i < indexPutDirs; i++)
			{
				FreePool(putedDirs[i]);
			}
			
		}
		else {
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

				if (
					dir[i].name[8] == 'B' && dir[i].name[9] == 'I' && dir[i].name[10] == 'N'
				)
					Services->Display->setAttrs(0, 0x2);
				else if (
					dir[i].name[8] == 'N' && dir[i].name[9] == 'S' && dir[i].name[10] == 'H'
				)
						Services->Display->setAttrs(0, 0x3);
				else
					Services->Display->setAttrs(0, 0x7);

				// Mostrar nombre y extensión
				Services->Display->printg(dir[i].name);

				Services->Display->setAttrs(0, 0x7);
				Services->Display->printg(" + ");
			}
		}
		Services->Display->printg("\n");
	
		GlobalServices->Memory->FreePool(StructFs);
	}
	else if (StrnCmp(command, "drvload ", 8) == 0)
	{
		char* drv_path = command + 8;

		int Magic = 0x3a5b;
		Services->Misc->Paramaters[0] = &Magic;

		void* buffer = NULL;
		int size = 0;

        FatFile file = Services->File->OpenFile(drv_path);
        KernelStatus status = Services->File->GetFile(file, &buffer, &size);

        if (!_StatusError(status)) {
            KernelStatus result = Services->Misc->RunBinary(buffer, size, Services);
        }

		Services->Memory->FreePool(buffer);

		Magic = 0x043b;
		Services->Misc->Paramaters[0] = &Magic;
	}
	else if (StrnCmp(command, "echo ", 5) == 0) { if (len == 5); else Services->Display->printg(command + 5);Services->Display->printg("\n"); }
	else if (StrCmp(command, "prp") == 0)
	{
		Services->Display->setAttrs(0, 10); 
		Services->Display->printg("ModuKernel");
		Services->Display->setAttrs(0, 9); 
		Services->Display->printg(":");
		char* Directory = CurrentWorkDirectory;
		Services->Display->printg(Directory);
		Services->Display->setAttrs(0, 7); 
		Services->Display->printg("# ");
	}
	else if (StrCmp(command, "reset") == 0) Services->Misc->Reset(0);
	else if (StrCmp(command, "shutdown") == 0) Services->Misc->Reset(1);
	else if (StrCmp(command, "heap") == 0)
	{
		unsigned int HeapRest = Services->Memory->GetFreeHeap();
		char freesp[16];

		UIntToString(HeapRest, freesp);

		Services->Display->printg("Espacio libre: ");
		Services->Display->printg(freesp);
		Services->Display->printg("\n");
	}
	else if (StrnCmp(command, "modush ", 7) == 0)
	{

		// incrementar nivel de privilegios
		InternalRingLevel++;

		char* shell_file =command + 7;
		void* buffer = NULL; int size = 0;

		char* Directory = CurrentWorkDirectory;
		size_t dirLen = StrLen(Directory);
		size_t fileLen = StrLen(shell_file);

		char* NameExtended = AllocatePool(sizeof(char) * (dirLen + fileLen + 2));
		InternalMemoryCopy(NameExtended, Directory, dirLen);
		InternalMemoryCopy(NameExtended + dirLen, shell_file, fileLen);
		NameExtended[dirLen + fileLen] = '\0';

        FatFile file = Services->File->OpenFile(NameExtended);
        KernelStatus status = Services->File->GetFile(file, &buffer, &size);

        if (!_StatusError(status)) {
			char* text = (char*)buffer; text[file.sector.file_size] = 0;
			InternalExecuteScript(text);
		}

		// decrementar nivel de privilegios
		InternalRingLevel--;

		Services->Memory->FreePool(buffer);
		Services->File->CloseFile(file);
		Services->Memory->FreePool(NameExtended);

		if (status) Services->Display->printg("no se pudo ejecutar");
	}
	else if (StrnCmp(command, "cat ", 4) == 0)
	{
		char* shell_file =command + 4;
		void* buffer = NULL; int size = 0;

		char* Directory = CurrentWorkDirectory;
		size_t dirLen = StrLen(Directory);
		size_t fileLen = StrLen(shell_file);

		char* NameExtended = AllocatePool(sizeof(char) * (dirLen + fileLen + 2));
		InternalMemoryCopy(NameExtended, Directory, dirLen);
		InternalMemoryCopy(NameExtended + dirLen, shell_file, fileLen);
		NameExtended[dirLen + fileLen] = '\0';

        FatFile file = Services->File->OpenFile(NameExtended);
        KernelStatus status = Services->File->GetFile(file, &buffer, &size);

        if (!_StatusError(status)) {
			char* text = (char*)buffer;
			text[file.sector.file_size] = 0;
			Services->Display->printg(text);
			Services->Display->printg("\n");
			Services->Memory->FreePool(buffer);
			return;
		}

		Services->Memory->FreePool(file.bs);
		Services->Memory->FreePool(buffer);
	}
	else if (StrCmp(command, "time") == 0)
	{
		KernelDateTime Time;
		Services->Misc->GetTime(&Time);

		char Hour[3];
		char Minute[3];
		char Second[3];

		IntToString2Digits(Time.hour, Hour);
		IntToString2Digits(Time.minute, Minute);
		IntToString2Digits(Time.second, Second);

		Services->Display->printg(Hour);
		Services->Display->printg(":");
		Services->Display->printg(Minute);
		Services->Display->printg(":");
		Services->Display->printg(Second);
		Services->Display->printg("\n");
	}
	else if (StrCmp(command, "date") == 0)
	{
		KernelDateTime Time;
		Services->Misc->GetTime(&Time);

		char Year[3];
		char Month[3];
		char Day[3];

		IntToString(Time.year, Year);
		IntToString2Digits(Time.month, Month);
		IntToString2Digits(Time.day, Day);

		Services->Display->printg(Day);
		Services->Display->printg("/");
		Services->Display->printg(Month);
		Services->Display->printg("/");
		Services->Display->printg(Year);
		Services->Display->printg("\n");
	}
	else if (StrCmp(command, "te") == 0)
	{
		
		Services->Display->ActivatePixel();

		Services->Display->setAttrs(2, 7);
		DrawLetter(0,0, 'C', 15);
		DrawLetter(1,0, 'c', 15);
		DrawLetter(2,0, 'c', 15);
	}
	else if (StrCmp(command, "modupanic") == 0) InternalModuPanic(KernelStatusSuccess);
	else if (StrCmp(command, "") == 0);
	else if (StrCmp(command, "memmap") == 0)
	{
		KernelPool* Index = heap_start;

		while (Index->next)
		{
			Services->Display->setAttrs(0, 7);

			// tamaño
			char Size[9];
			// puntero
			char Pointer[9];

			// resolver tamaño
			IntToHexString(Index->size, Size);
			// resolver puntero
			IntToHexString(Index->MemoryPtr, Pointer);

			// buena suerte liberando eso bro 💀
			if (Index->Type == MemAllocTypePrograms && ((*Services->Memory->MallocType) != MemAllocTypePrograms)) Services->Display->setAttrs(0, 4);

			// memorias
			switch (Index->Type)
			{
				// de servicios
			case MemAllocTypeKernelServices:
				Services->Display->printg("Services");
				break;
				// de programas
			case MemAllocTypePrograms:
				Services->Display->printg("UserMemory");
				break;
				// de sistema (Shell o otra app integrada mas no hecha por el usuario, estas ya vienen en codigo fuente del kernel)
			case MemAllocTypeSystem:
				Services->Display->printg("SystemMemory");
				break;
				// memoria liberada o inexistente
			case MemAllocTypeFreeOrNotExist:
				Services->Display->printg("FreeMemory");
				break;
				// por defecto
			default:
				Services->Display->printg("Unknown");
				break;
			}

			Services->Display->CurrentCharacter = 30;

			// informacion
			Services->Display->setAttrs(0, 8);
			Services->Display->printg("("); 
					Services->Display->setAttrs(0, 9);
					Services->Display->printg(Size);
				Services->Display->setAttrs(0, 7);
				Services->Display->printg(", "); 
					Services->Display->CurrentCharacter = 49;
					Services->Display->setAttrs(0, 10);
					Services->Display->printg(Pointer); 
				Services->Display->setAttrs(0, 8);
			Services->Display->printg(")");

			Services->Display->setAttrs(0, 7);
			Services->Display->printg(",\n");

			// sumar
			Index = Index->next;
		}
	}
	else if (StrCmp(command, "sys") == 0)
	{
		// Dirección absoluta del puntero Services
		uintptr_t Direction = (uintptr_t)Services - (uintptr_t)&_heap_start;

		char buf[19];
		IntToHexString(Direction, buf);

		Services->Display->printg(buf);
		Services->Display->printg("\n");
	}
	else if (StrnCmp(command, "mall ", 5) == 0)
	{
		ModuAllocType OldMem = *Services->Memory->MallocType;

		*Services->Memory->MallocType = MemAllocTypePrograms;

		// el str
		char* str = command + 5;
		// el buffer
		char* Buffer = Services->Memory->AllocatePool(sizeof(char) * (StrLen(str) + 1));

		Buffer[(StrLen(str))] = 0;

		// lo pone
		for (size_t i = 0; i < StrLen(str); i++) Buffer[i] = str[i];
		
		// el buffer
		int Direction = Buffer-&_heap_start;

		char buf[19]; IntToHexString(Direction, buf);

		Services->Display->printg(buf); Services->Display->printg("\n");

		*Services->Memory->MallocType = OldMem;
	}
	else if (StrnCmp(command, "free ", 5) == 0)
	{
		char* adirection = command + 5;

		// libera todo de la shell y programas del sistema
		if (StrCmp(adirection, "/sys") == 0)
		{
			// indice
			KernelPool* Index = heap_start;

			// liberar
			while (Index->next) 
			{
				// liberar
				if (Index->Type == MemAllocTypeSystem) FreePool(Index->MemoryPtr);
				Index = Index->next;
			}
		}
		// libera todo los servicios del sistema
		else if (StrCmp(adirection, "/services") == 0)
		{
			// indice
			KernelPool* Index = heap_start;

			// liberar
			while (Index->next) 
			{
				// liberar
				if (Index->Type == MemAllocTypeKernelServices) FreePool(Index->MemoryPtr);
				Index = Index->next;
			}
		}
		// libera la memoria de programas no usada
		else if (StrCmp(adirection, "/user") == 0)
		{
			// indice
			KernelPool* Index = heap_start;

			// liberar
			while (Index->next) 
			{
				// liberar
				if (Index->Type == MemAllocTypePrograms) FreePool(Index->MemoryPtr);
				Index = Index->next;
			}
		}
		// liberar todo
		else if (StrCmp(adirection, "/") == 0)
		{
			// indice
			KernelPool* Index = heap_start;

			// liberar
			while (Index->next) 
			{
				// liberar
				FreePool(Index->MemoryPtr);
				Index = Index->next;
			}
		}
		else 
		{
			uintptr_t Direction = HexStringToInt(adirection);

			void* Buffer = (void*)Direction;

			FreePool(Buffer);
		}
	}
	else if (StrCmp(command, "dheap") == 0)
	{
		Services->Display->printg("Region de Servicios del Kernel (en heap)\n");
		InternalDumpHexMemory((void*)Services, sizeof(KernelServices));
	}
	else if (StrnCmp(command, "dheap ", 6) == 0)
	{
		Services->Display->printg("Mostrar heap personalizado\n");

		char* Sizea = command + 6;

		if (StrnCmp(Sizea, "/mw ", 4) == 0) InternalDumpHexMemory(Services->Info->ModuWorldPtr, (size_t)HexStringToInt(Sizea + 4));
		else InternalDumpHexMemory(heap_start, (size_t)HexStringToInt(Sizea));
	}
	else if (StrCmp(command, "shift") == 0)
	{
		char* vidmem = (char *)0xA0000;
		InternalMemMove(vidmem, vidmem + 1, 320*200);
	}
	else if (StrCmp(command, "logoff") == 0)
	{
		Services->Display->printg("login off, yo can press ENTER to wake up the system\n");
		Services->Memory->RepairMemory(Services);
	}
	else if (StrCmp(command, "ugrm") == 0) unconfig_mode();
	else if (StrCmp(command, "grm") == 0) config_mode();
	else if (StrCmp(command, "ulr") == 0) 
	{
		char esb[30];
		UIntToString(InternalRingLevel, esb);

		if (InternalRingLevel < 2) Services->Display->printg("In-C env");
		else if (InternalRingLevel == 0) Services->Display->printg("Assembly (Post-C) env");
		else Services->Display->printg(esb);
		Services->Display->printg("\n");
	}
	else if (StrCmp(command , "map") == 0)
	{
		Services->Display->printg("HDA:   (Hard Disk)\n");
		Services->Display->printg("FDA:   (Floppy Disk)\n");
		Services->Display->printg("CDROM: (CD-ROM)\n");
	}
	else if (command[len - 1] == ':')
	{
		// reservar espacio (+1 para el terminador)
		char* NewStr = Services->Memory->AllocatePool(sizeof(char) * (len + 1));

		// copiar la cadena original
		Services->Memory->CoppyMemory(NewStr, command, len);

		// añadir terminador nulo
		NewStr[len] = '\0';

		// convertir a mayúsculas
		char* Upper = StrUpr(NewStr);

		// quitar el ':' final
		Upper[len - 1] = '\0';

		// cambiar a floppy
		if (StrCmp(Upper, "FDA") == 0) *(Services->File->CurrentDiskType) = DiskTypeFloppy;
		// cambiar a hard disk
		else if (StrCmp(Upper, "HDA") == 0) *(Services->File->CurrentDiskType) = DiskTypeHardDisk;
		// cambiar a cdrom
		else if (StrCmp(Upper, "CDROM") == 0) *(Services->File->CurrentDiskType) = DiskTypeCdRom;
	
		// liberar pool
		Services->Memory->FreePool(Upper);
	}
	else {
		char* Params[128];
		int ParamsCount = StrSplit(command, Params, ' ');

        char name[9];
        InternalMemorySet(name, ' ', 8);
        name[8] = '\0';
        for (int i = 0; i < len && i < 8; i++) name[i] = command[i];

        void* buffer = 0;
        int size = 0;

		char* Directory = CurrentWorkDirectory;
		size_t dirLen = StrLen(Directory);
		size_t fileLen = StrLen(Params[0]);

		char* NameExtended = AllocatePool(sizeof(char) * (dirLen + fileLen + 2));
		InternalMemoryCopy(NameExtended, Directory, dirLen);
		InternalMemoryCopy(NameExtended + dirLen, Params[0], fileLen);
		NameExtended[dirLen + fileLen] = '\0';

        FatFile file = Services->File->OpenFile(NameExtended);
        KernelStatus status = Services->File->GetFile(file, &buffer, &size);
        if (!_StatusError(status)) {
			*Services->Misc->ParamsCount = ParamsCount + 2; // tipo + count + args

			int Magic = 0x043b;
			Services->Misc->Paramaters[0] = (void*)(intptr_t)Magic;   // guardar valor como entero

			Services->Misc->Paramaters[1] = (void*)(intptr_t)ParamsCount; // cantidad

			for (int i = 0; i < ParamsCount; i++) {
				Services->Misc->Paramaters[i + 2] = (void*)Params[i]; // cada string
			}
			
            KernelStatus result = Services->Misc->RunBinary(buffer, size, Services);
            
			Services->Memory->FreePool(NameExtended);
			Services->File->CloseFile(file);
			Services->Memory->FreePool(buffer);

			Services->Display->printg("\nReturn Status: ");
			char* StatusStr = StatusToString(result);
			Services->Display->printg(StatusStr);
			Services->Display->printg("\n");
			Services->Memory->FreePool(StatusStr);
			return;
        }		
		Services->Memory->FreePool(buffer);
		Services->File->CloseFile(file);
		Services->Memory->FreePool(NameExtended);

        Services->Display->printg("no se pudo leer archivo\n");
	}
}
void k_main() 
{ 
	InternalDebug("ModuKernel Debug Console\n\nBienvenido a la consola de desarrollo de tu kernel basado en ModuKernel, aqui veras las noticias que mande tu sistema operativo en tiempo real, puede tambien proximamente mandar acciones al kernel y interrumpir\n");

	// debuggear
	SystemInternalMessage("ModuKernel ha entrado en C");

	// etapa de arranque prematura aqui se inicializan los servicios basicos
	// mas no los servicios del kernel donde se iniciaran prototipadamente

	// el tipo de disco actual
	SystemCwkDisk = DiskTypeHardDisk;

	// memoria actual
	MemoryCurrentSystem = MemAllocTypeKernelServices;

	// debuggear
	SystemInternalMessage("Inicializando Heap...");

	// inicializar heap para AllocatePool y FreePool
	InitHeap();

	// debuggear
	SystemInternalMessage("Inicializando Servicios...");

	// los servicios
    KernelServices Services;

	// inicializar servicios
	InitializeKernel(&Services);

	// etapa de arranque silencioso aqui se seleccionan diferentes configuraciones
	// y otras cosas para poder inicializar los servicios de manera compleja, como
	// la pantalla

	// debuggear
	SystemInternalMessage("Inicializando Pantalla...");

	// setear el servicio acutal de la pantalla
    Services.Display->Set(Services.Display);
	// setear atributos
    Services.Display->setAttrs(0, 7);
	// limpiar la pantalla
    Services.Display->clearScreen();

	// etapa de arranque en esta etapa ya es visible para el usuario, ya casi todo
	// el kernel esta medio despierto, incluyendo los serviicos de pantalla, disco,
	// memoria y otros, asi que ya le pueden mostarar al usuario

	// ir a la posicion x0y0
	Services.Display->setCursorPosition(0,0);

	Services.Music->PlayTone(440);
	Services.Time->TaskDelay(20);
	Services.Music->Mute();

	// debuggear
	SystemInternalMessage("Empezando a arrancan");

	// imprimir la promocion
	Services.Display->printg("ErickCraftStudios ModuKernel (Operating System)\n\n");
	Services.Display->printg("powered By ModuKernel - https://github.com/ErickStudios/ModuKernel\n\n");
	Services.Time->TaskDelay(20);
	Services.Display->printg("puedes apoyarnos en github con el enlaze, estamos arrancando el Kernel, espere\n");
	Services.Display->printg("[ KSuccess   ]  Kernel Cargado\n");
	Services.Display->printg("[ KSuccess   ]  Kernel Configurado\n");
	Services.Display->printg("cargando kernel\n");
	Services.Time->TaskDelay(40);

	// limpiar la pantalla
	Services.Display->clearScreen();
	
	// proteger regiones
	InternalProtectRegion(heap_start, sizeof(heap_start), GdtLevelInRing0);
	InternalProtectRegion(heap_ptr, sizeof(heap_ptr), GdtLevelInRing0);

	// esto no es otra etapa de arranque, sigue siendo la etapa de arranque normal
	// aunque aqui se hace una animacion para que no se vea muy cutre, recuerden, pueden
	// personalizarla si se basan en el kernel

	InternalRingLevel -= 2;

	// archivo init
	FatFile InitFile = Services.File->OpenFile("/kernel/init");
	// contenido
	ObjectAny InitCode; 
	// tamaño
	int32_t InitSize;
	// abrir
	KernelStatus OpenInit = Services.File->GetFile(InitFile, &InitCode, &InitSize);

	// si salio bien
	if (!_StatusError(OpenInit)) Services.Misc->RunBinary(InitCode, InitSize, &Services);

	// liberar
	Services.Memory->FreePool(InitCode);
	Services.File->CloseFile(InitFile);

	// ahora que hacemos si no esta
	if (_StatusError(OpenInit)) Services.Misc->Throw(KernelStatusNotFound);
}
KernelStatus ProcessCrtByFile(char* name, char* ext, KernelServices* Services)
{
	FatFile file = InternalDiskFindFile(name, ext);

	// después de cargar el archivo en memoria
	void* data;
	int size;

	if (InternalDiskGetFile(file, &data, &size) == 0) {
		InternalRunBinary(data, size, Services);
	}
}
void* InternalAllocatePool(unsigned int size, ModuAllocType Type) 
{
    KernelPool* current = free_list;

    while (current) {
        if (current->free && current->size >= size) {

            if (current->size > size + sizeof(KernelPool)) {
                // dividir el bloque
                KernelPool* new_block = (KernelPool*)((char*)current + sizeof(KernelPool) + size);
                new_block->size = current->size - size - sizeof(KernelPool);
                new_block->free = 1;
                new_block->next = current->next;

                current->next = new_block;
            }

            current->free = 0;
            current->size = size;
			current->Type = Type;
		
			current->MemoryPtr = (char*)current + sizeof(KernelPool);

            return (char*)current + sizeof(KernelPool);
        }

        current = current->next;
    }

	GlobalServices->Display->printg("\n");

	InternalModuPanic(KernelStatusNoBudget);
	return 0; // out of memory
}
void* AllocatePool(unsigned int size) 
{
   return InternalAllocatePool(size, MemoryCurrentSystem);
}
void FreePool(void* ptr) {
    if (!ptr) return;

    KernelPool* block = (KernelPool*)((char*)ptr - sizeof(KernelPool));
    block->free = 1;
	block->Type = MemAllocTypeFreeOrNotExist;

    KernelPool* current = free_list;

    while (current && current->next) {
        KernelPool* next = current->next;

        // verificar que estén contiguos en memoria
        if (current->free && next->free &&
            (char*)current + sizeof(KernelPool) + current->size == (char*)next) {

            // unir bloques
            current->size += sizeof(KernelPool) + next->size;
            current->next = next->next;
        } else {
            current = current->next;
        }
    }
}
void InternalClearScreen()
{
	*row_selected = 0;
	*line_selected = 0;

	if (InternalGrapichalFlag)
	{
		InternalDrawBackground(
			(*text_attr >> 4) & 0x07
		);
		return;
	}
	
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
	if (InternalGrapichalFlag)
	{
		int letter = 0;
		int line = *line_selected;
		int column = (*row_selected) / 2;

		while (message[letter])
		{
			if (line >= 32) {
				char* vidmem = (char *)0xA0000;
				InternalMemMove(vidmem, vidmem + (320 * 4), 320*200);
				line = 31;
			}
			if (message[letter] == '\n')
			{
				line++;
				column = 0;
			}
			else	
			{
				DrawLetter(column,line, '\a', (*text_attr >> 4) & 0x07);
				DrawLetter(column,line, message[letter], *text_attr % 0x10);
				column++;
			}

			if (column >= 28)
			{
				line++;
				column = 0;
			}

			letter++;
		}

		*line_selected = line;
		*row_selected = column * 2;
	}
	else
	{
		char *vidmem = (char *)0xb8000;
		int line = *line_selected;
		int column = (*row_selected) / 2; // convertir a columna

		while (*message != 0)
		{
			if (*message == '\n' || column >= 80)
			{
				line++;
				column = 0;
				message++;

				if (line >= 25) {
					InternalMemMove(vidmem, vidmem + 80*2, 24*80*2);
					for (int j = 0; j < 80; j++) {
						vidmem[(24*80 + j)*2] = ' ';
						vidmem[(24*80 + j)*2 + 1] = *text_attr;
					}
					line = 24;
				}
			}
			else
			{
				int pos = (line * 80 + column) * 2;
				vidmem[pos] = *message;
				vidmem[pos+1] = *text_attr;
				message++;
				column++;
			}
		}

		*line_selected = line;
		*row_selected = column * 2; // mantener compatibilidad
	}
}
void InternalSleepDream(KernelServices* Serv)
{
	int TimeOut = 30;

	// espearar a que el usuario presione una tecla si no se acabara
	// el tiempo y el OS entrara en otra etapa

	while (TimeOut--) 
	{
		// tecla
		char Key = Serv->InputOutput->ReadKey();

		// etapa terminada
		if (Key == '\n') return;

		// delay
		Serv->Time->TaskDelay(2);
	}

	Serv->Display->printg("[ Kernel ]  Entrando a el Free All Unecesary Memory, una vez que termine puede presionar ENTER para despertar al sistema por ahora no lo puede hacer, espere\n");

	// el sistema entra en la etapa FAUM (Free. All. Unecesary. Memory) donde
	// aprovechara un garbage collector para liberar memoria del Heap que
	// o es basura/cache o no fue liberada por que a algun tonto se le haya ocurrido
	// dejar errores de memoria en los programas, asi que esto es necesario, no se
	// preocupe pronto estara soñando y podras despertarlo

	// indice
	KernelPool* Index = heap_start;

	// liberar
	while (Index->next) 
	{
		// liberar memoria basuara, o no liberada
		if (
			Index->Type == MemAllocTypeSystem || Index->Type == MemAllocTypePrograms 
		)
		 FreePool(Index->MemoryPtr);

		Index = Index->next;

		Serv->Time->TaskDelay(2);
	}

	Serv->Display->printg("[ Kernel ]  Ya puede despertar al sistema presionando ENTER\n");

	// el sistema ya no tiene mejor cosa que hacer que quedarse dormido
	// por que ya se libero todo y ya lo puedes despertar sin riesgo de que
	// si no duerme todo el rato muchas veces se terminara llenando de basuar

	while (1)
	{
		char Key = Serv->InputOutput->ReadKey();
		if (Key == '\n') return;
		Serv->Time->TaskDelay(1);
	}
	
}