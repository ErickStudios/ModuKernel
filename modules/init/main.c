/* main de init, esa es el proceso que el sistema ejecuta al terminar de cargar y iniciarse
este muestra la animacion de arranque, init se encarga de darle al usuario un espacio de trabajo
aniado con una shell minimalista todo esto a nivel de usuario y kernel

haciendo una animacion para el sistema operativo para despues cargar con el fs la shell de 
ModuKernel que esta definida en modules/shell/main.c */

// incluir libreria
#include "../../library/lib.h"
#include "serial.h"
#include "../../library/modubmp.h"

/* ejecutor */
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

			char* fnline = gMS->AllocatePool(NewLen);

			gMS->CoppyMemory(fnline, Function, NewLen);

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

			char* fnline = gMS->AllocatePool(NewLen);

			gMS->CoppyMemory(fnline, Function, NewLen);

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
		else gSys->Misc->Run(gSys, Line, 0);
	}
}

void DrawImage(int x, int y, int wd, unsigned char* img, int size, uint8_t chroma, int range)
{
    int xM = x;
    int yM = y;

    for (int i = 0; i < size; i++) {
        uint8_t color = img[i];

		char bf[20];
		IntToString((int)color, bf);

		InternalDebug(bf);

        if (xM >= (x + wd)) {
            xM = x;
            yM++;
        }

        if (color != chroma)
		{
			if ((xM - x) > range && (xM - x) < range + 5)
				gDS->DrawRectangle(1, xM, yM, 1);
			else 
				gDS->DrawRectangle(color, xM, yM, 1);
		}
        xM++;
    }
}

/* funcion principal */
KernelStatus ErickMain(KernelServices *Services)
{
    // hacer algo
    Services->Display->clearScreen();

    // iniciazliar el arranque
    InitializeLibrary(Services);

	// debuggear
	SystemInternalMessage("Mostrando logo...");

	char interrupted = 0;

	// zona
	int IndexZone = 0;
	// logo
	char LogoABC[] = "ModuKernel";
	// logo para operaciones
	char* LogoABCForOperations = LogoABC;
	// longitud
	int LogoLen = 70;

	Services->Display->ActivatePixel();

	FatFile ImageLogo = Services->File->OpenFile("/kernel/logo.bmp");
	Entero ImageSize;
	ObjectAny ImageContent;
	KernelStatus OpenImageLogo = Services->File->GetFile(ImageLogo, &ImageContent, &ImageSize);
	
	struct _ImageHeader* hdr;
	bytes raw;
	uint32_t SizeX;
	uint32_t SizeY;
	GetImageThing(ImageContent, &hdr, &raw, &ImageSize, &SizeX, &SizeY);
	// mostrar logo
	for (int ab = 0; ab < (LogoLen * 5); ab++)
	{
		char Key = Services->InputOutput->ReadKey();

		if (Key == 'c' || Key == 'C') 
		{
			interrupted = 1;
			break;
		}

		// setear linea actual

		DrawImage(
			45,                // X destino
			120,               // Y destino
			SizeX,       	   // ancho
			raw,               // datos crudos
			ImageSize,    	   // tamaño de los datos
			0b11001000,        // color de fondo/transparencia
			IndexZone          // zona destino
		);
		// esperar
		Services->Time->TaskDelay(1);

		// sumar zona
		IndexZone++;
		// si ya recorrio el logo volver al inicio
		if (IndexZone > LogoLen) IndexZone = 0;
	}	

	// debuggear
	SystemInternalMessage("Kernel Cargado");
	InternalDebug("\x1B[96m--- *** logs de modo usuario *** ---\x1B[0m\n\n");

	// etapa de usuario esta no es una etapa de arranque si no la recta final pero
	// aqui el usuario ya tiene control completo del sistema y ya el sistema esta totalmente
	// despierto, asi que lanza la shell por que si no que mas lanzar, recuerden que pueden
	// personalizar su mini programa

	// tipo de memoria del sistema
	*(Services->Memory->MallocType) = MemAllocTypeSystem;

	Services->Display->printg("\n\n");

    FatFile OtherFile = Services->File->OpenFile("/sys/init.sh");

    void* content; int size;

    KernelStatus OpenInitScript = Services->File->GetFile(OtherFile, &content, &size);

    // ejecutar
    if (!(_StatusError(OpenInitScript)))
    {
		char* bf = content;
		bf[size] = 0;
		InternalExecuteScript(bf);
        Services->Memory->FreePool(content);
        Services->File->CloseFile(OtherFile);
    }
    else 
    {
        Services->Memory->FreePool(content);
        return OpenInitScript;
    }

}