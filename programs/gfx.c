/* graficos para ModuKernel, el programa necesita tener parametros el primer parametro del programa
es la posicion x del rectangulo, en pixeles, el segundo parametro del programa es la posicion y
del rectangulo en pixeles, el tercer parametro es el tamaño en x del rectangulo, en pixeles, el cuarto
parametro es el tamaño en y del rectangulo, el quinto es el color */

#include "../library/lib.h"

KernelStatus DrawPixelScreen
(
    DisplayServices* Grapichs,
    Entero XPos,
    Entero YPos,
    Entero XSize,
    Entero YSize,
    MiniPositivo Color
)
{
    // si esta en modo texto
    if (*(Grapichs->IsInPixelsMode) != 1) return KernelStatusInvalidParam;

    Entero XEnd = XPos + XSize;
    Entero YEnd = YPos + YSize;

    Entero PutPixelX = XPos;
    Entero PutPixelY = YPos;

    while (1)
    {

        Grapichs->DrawRectangle(Color, PutPixelX, PutPixelY, 1);

        if (PutPixelX >= XEnd)
        {
            PutPixelX = XPos;
            PutPixelY++;
        }
        
        if (PutPixelY >= YEnd) break;
        PutPixelX++;
    }
    
}

KernelStatus ErickMain(KernelServices* Services)
{
    GetArgs(Services, count);
    GetMagic(Services, execType);

    Entero XPos;
    Entero YPos;
    Entero XSize;
    Entero YSize;
    MiniPositivo Color;

    for (int i = 1; i < count; i++) {
        char* arg = (char*)Services->Misc->Paramaters[i + 2];
    
        if (i == 1) XPos = StringToInt(arg);
        else if (i == 2) YPos = StringToInt(arg);
        else if (i == 3) XSize = StringToInt(arg);
        else if (i == 4) YSize = StringToInt(arg);
        else if (i == 5) Color = (MiniPositivo)StringToInt(arg);
    }

    return DrawPixelScreen(Services->Display, XPos, YPos, XSize, YSize, Color);
}