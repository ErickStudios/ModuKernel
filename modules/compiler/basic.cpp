// la libreria
#include "../../library/lib.hpp"
// el inline
#include "inline.hpp"
/// punto de entrada
extern "C" KernelStatus ErickMain(KernelServices* Services)
{
    // inicializar libreria
    InitializeLibrary(Services);

    // argumentos
    GetArgs(Services, count);

    // archivo
    char* file;

    bool InmediateCode = false;

    // recorrer parametros
    for (int i = 1; i < count; i++) {
        // argumento
        char* arg = (char*)Services->Misc->Paramaters[i + 2];

        // si el parametro es el primero, es el archivo
        if (i == 1)
        {
            file = arg;
            if (StrCmp(file, "--x") == 0) {
                InmediateCode = true;
                file = (char*)Services->Misc->Paramaters[i + 3];
            } 
        }
    }

    if (InmediateCode)
    {
        // el archivo
        ModuLibCpp::String* FileContent = new ModuLibCpp::String((const char*)file);

        int len = 0;
        uint8_t* Buffer = CodifiqueProgram(*FileContent, &len);

        for (size_t i = 0; i < len; i++)
        {
            char ll[20];
            IntToHexString(Buffer[i], ll);
            ModuLibCpp::Display::Print(ll);
            ModuLibCpp::Display::Print(" ");
        }
        ModuLibCpp::Display::Print("\n");

        delete FileContent;
        return KernelStatusSuccess;
    }

    // archivo
    FatFile AsmFile = Services->File->OpenFile(file);
    char* AsmFileContent;
    Entero AsmFileSize;
    KernelStatus AsmOpen = (KernelStatus)gSys->File->GetFile(AsmFile, (void**)&AsmFileContent, &AsmFileSize);

    // si no se pudo abrir
    if (AsmOpen) return AsmOpen;

    AsmFileContent[AsmFileSize] = 0;
    int len = 0;

    // el archivo
    ModuLibCpp::String* FileContent = new ModuLibCpp::String((const char*)AsmFileContent);

    uint8_t* Buffer = CodifiqueProgram(*FileContent, &len);

    for (size_t i = 0; i < len; i++)
    {
        char ll[20];
        IntToHexString(Buffer[i], ll);
        ModuLibCpp::Display::Print(ll);
        ModuLibCpp::Display::Print(" ");
    }
    ModuLibCpp::Display::Print("\n");

    delete FileContent;

    // liberar contenido
    gMS->FreePool(AsmFileContent);
    // cerrar archivo
    gSys->File->CloseFile(AsmFile);

    return KernelStatusSuccess;
}