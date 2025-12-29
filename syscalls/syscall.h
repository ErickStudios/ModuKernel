/* archivo de el sys call */

#include "../handlers/regs.h"

extern void IdtSystemCall();

unsigned int SysCallParam3;
unsigned int SysCallParam4;
unsigned int SysCallParam5;

/* llamada al sistema 80h */
void IdtSystemCallHnd(regs_t* r)
{
    int call = r->eax;
    int function = r->ecx;
    unsigned int parameter = r->ebx;
    unsigned int parameter2 = r->edx;

    /* seteo de parametros*/
    if (call == 0)
    {
        switch (function)
        {
        case 0:
            SysCallParam3 = parameter;
            break;
        case 1:
            SysCallParam4 = parameter;
            break;
        case 2:
            SysCallParam5 = parameter;
            break;
        default:
            break;
        }
    }
    /* display */
    else if (call == 1)
    {
        // imprimir
        if (function == 0)
            GlobalServices->Display->printg((char*)parameter);
        // limpiar
        else if (function == 1)
            GlobalServices->Display->clearScreen();
        // ir a
        else if (function == 2)
            GlobalServices->Display->setCursorPosition(parameter, parameter2);
        // cambiar color a
        else if (function == 3)
            GlobalServices->Display->setAttrs((char)parameter, (char)parameter2);
        // entrar en modo grafico
        else if (function == 5)
            GlobalServices->Display->ActivatePixel();
    }
    /* memoria */
    else if (call == 2)
    {
        // localizar memoria
        if (function == 0)
            *(void**)parameter2 = GlobalServices->Memory->AllocatePool(parameter);
        // liberar memoria
        else if (function == 1)
            GlobalServices->Memory->FreePool(parameter);
    }
    /* input/outpud */
    else if (call == 3)
    {
        // para leer una tecla y esperar
        if (function == 0)
            *((char*)parameter) = GlobalServices->InputOutput->WaitKey();
        // para leer una linea
        else if (function == 1)
            *((char**)parameter) = GlobalServices->InputOutput->ReadLine();
        // para leer una tecla
        else if (function == 2)
            *((char*)parameter) = GlobalServices->InputOutput->ReadKey();
    }
    /* para leer archivos */
    else if (call == 4)
    {
        if (function == 0) // OpenFile
        {
            struct _FatFile* File = AllocatePool(sizeof(struct _FatFile));
            *File = GlobalServices->File->OpenFile((char*)parameter);
            *(struct _FatFile**)parameter2 = File;
        }
        else if (function == 1) // GetFile
        {
            struct _FatFile* file = *(struct _FatFile**)parameter;
            // parameter2 = void** content_out
            // SysCallParam3 = int* size_out (ya seteado con call=0,function=0)
            GlobalServices->File->GetFile(*file, (void**)parameter2, (int*)SysCallParam3);
        }
        else if (function == 2) // CloseFile
        {
            struct _FatFile* file = *(struct _FatFile**)parameter;
            GlobalServices->File->CloseFile(*file);
        }
    }
}