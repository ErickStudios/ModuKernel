#include "../../library/lib.hpp"
#include "../../library/modubmp.h"

void DrawImage(int x, int y, int wd, unsigned char* img, int size)
{
	if (*(gDS->IsInPixelsMode) != 1) return;
    int xM = x;
    int yM = y;

    for (int i = 0; i < size; i++) {
        uint8_t color = img[i];

		char bf[20];
		IntToString((int)color, bf);

        if (xM >= (x + wd)) {
            xM = x;
            yM++;
        }

        gDS->DrawRectangle(color, xM, yM, 1);
        xM++;
    }
}

extern "C" KernelStatus ErickMain(KernelServices* Services) {
    // inicializar libreria
    InitializeLibrary(Services);

    GetArgs(Services, Count);

    // archivo
    char* file;

    // recorrer parametros
    for (int i = 1; i < count; i++) {
        // argumento
        char* arg = (char*)Services->Misc->Paramaters[i + 2];

        // si el parametro es el primero, es el archivo
        if (i == 1) file = arg;
    }


    // archivo
    FatFile ImageFile = Services->File->OpenFile(file);
    ObjectAny ImageFileContent;
    Entero ImageFileSize;
    KernelStatus ImageFileOpen = (KernelStatus)gSys->File->GetFile(ImageFile, &ImageFileContent, &ImageFileSize);

    // si no se pudo abrir
    if (ImageFileOpen) return ImageFileOpen;

    _ImageHeader* hdr;
    bytes raw;
    uint32_t SizeX;
    uint32_t SizeY;
    GetImageThing(ImageFileContent, &hdr, &raw, (uint32_t*)&ImageFileSize, &SizeX, &SizeY);

    DrawImage(0,0, SizeX, raw, ImageFileSize);

    return KernelStatusSuccess;
}