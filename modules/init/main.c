/* main de init, esa es el proceso que el sistema ejecuta al terminar de cargar y iniciarse
este muestra la animacion de arranque, init se encarga de darle al usuario un espacio de trabajo
aniado con una shell minimalista todo esto a nivel de usuario y kernel

haciendo una animacion para el sistema operativo para despues cargar con el fs la shell de 
ModuKernel que esta definida en modules/shell/main.c */

// incluir libreria
#include "../../library/lib.h"
#include "serial.h"

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
	int LogoLen = StrLen(LogoABCForOperations);

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

		Services->Display->CurrentLine = 12;
		Services->Display->CurrentCharacter = 70;

		// mostrar logo
		for (int i = 0; i < LogoLen; i++)
		{
			// si esta cerca pero no en la zona ,color cyan
			if (i == (IndexZone-1) || i == (IndexZone+1)) Services->Display->setAttrs(0, 3);
			// si esta en la zona ,color verde
			else if (i == IndexZone) Services->Display->setAttrs(0, 2);
			// si no esta en el rango ,color gris
			else Services->Display->setAttrs(0, 7);

			// escritura
			char Wrt[2] = { LogoABCForOperations[i], 0 }; Services->Display->printg(Wrt);
		}		

		// esperar
		Services->Time->TaskDelay(2);

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

    FatFile OtherFile = Services->File->OpenFile("/kernel/modush");

    void* content; int size;

    KernelStatus OpenFileBin = Services->File->GetFile(OtherFile, &content, &size);

    // ejecutar
    if (!(_StatusError(OpenFileBin)))
    {
        Services->Misc->RunBinary(content, size, Services);
        Services->Memory->FreePool(content);
        Services->File->CloseFile(OtherFile);
    }
    else 
    {
        Services->Memory->FreePool(content);
        return OpenFileBin;
    }

}