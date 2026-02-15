#include "../../library/lib.hpp"

/**
 * @dead beef
 */
#define MAX_VIEW_LINES 27

/// @brief scroll de la linea
int ScrollLine = 0;

/// @brief la linea actual
int CurrentLine = 0;

/// @brief dibja el editor
/// @param Lines las lineas
void DrawEditor(ModuLibCpp::Array<ModuLibCpp::String>& Lines) {
    gDS->clearScreen();
    // asegurarse de no pasar del final
    for (int i = ScrollLine; ScrollLine + MAX_VIEW_LINES; i++) {
        if (i > ScrollLine + MAX_VIEW_LINES) break;
        auto line = Lines[i].InternalString;

        if (i < Lines.size()) ModuLibCpp::Display::Print(line);
        else ModuLibCpp::Display::Print(".");
        if (i == (CurrentLine)) {
            gDS->DrawLetter(gDS->CurrentCharacter / 2, gDS->CurrentLine, '_', 0b11101011, 0,1);
        }
        ModuLibCpp::Display::Print("\n");

    }
}

KernelStatus Editor(ModuLibCpp::String& Text, ModuLibCpp::String& outtext) {
    
    /// @brief si esta en modo editor
    bool InEditMode = false;

    ModuLibCpp::Array<char*> LinesReal = Text / '\n';
    ModuLibCpp::Array<ModuLibCpp::String> Lines{};

    for (auto line : LinesReal) Lines.push(ModuLibCpp::String{line});
    
    while (true) {
        DrawEditor(Lines);
        unsigned char key = gIOS->WaitKey();

        char k[2] = {key, 0};

        if (key == EscaneoFlechaArriba && InEditMode == false) 
           CurrentLine--;
        else if (key == EscaneoFlechaAbajo && InEditMode == false) 
            CurrentLine++;
        else if (key == '\n' && InEditMode == false) 
            InEditMode = true;
        else if (key == KernelSimpleIoFuncKey(1) && InEditMode) 
            InEditMode = false;
        else if (key == ':' && InEditMode == false)
        {
            gDS->printg(":");
            char* Command = gIOS->ReadLine();

            if (StrCmp(Command, "q") == 0)
            {
                return KernelStatusDisaster;
            }
            else if (StrCmp(Command, "qs") == 0)
            {
                ModuLibCpp::String LinesCombined{};
                
                for (auto& line : Lines)
                {
                    LinesCombined << line << "\n";
                }
                LinesCombined.RemoveLastChar();
                outtext.SetString(LinesCombined.InternalString);
                return KernelStatusSuccess;
            }
        }
        else if (key == '\b' && InEditMode) 
        {
            if (StrLen(Lines[CurrentLine].InternalString) == 0) Lines.remove((CurrentLine--));
            else Lines[CurrentLine].RemoveLastChar();
        }
        else if (key == '\n' && InEditMode)
            Lines.insert((CurrentLine++)+1, ModuLibCpp::String{""});
        else if (InEditMode) { 
            // get started
            if (Lines.getCount() == 0 && CurrentLine == 0)
            {
                Lines.push(ModuLibCpp::String{""});
            }
            // ya hay texto
            Lines[CurrentLine].AddChar(key); 
        }

        // ajustar scroll
        if (CurrentLine >= ScrollLine + MAX_VIEW_LINES) {
            ScrollLine = CurrentLine - MAX_VIEW_LINES + 1;
        }
        else if (CurrentLine < ScrollLine) {
            ScrollLine = CurrentLine;
        }

    }
}

/// @brief el ErickMain
/// @param Services los servicios
/// @return 
extern "C" KernelStatus ErickMain(KernelServices* Services) {
    InitializeLibrary(Services);

    // argumentos
    GetArgs(Services, count);

    // archivo
    char* file = nullptr;

    // recorrer parametros
    for (int i = 1; i < count; i++) {
        // argumento
        char* arg = (char*)Services->Misc->Paramaters[i + 2];

        // si el parametro es el primero, es el archivo
        if (i == 1) 
            file = arg;
    }

    ModuLibCpp::File<char*> File{file};

    // si no se pudo abrir
    if (File.data() == nullptr) {
        ModuLibCpp::String Text{};
        Editor(Text, Text);
        return KernelStatusSuccess;
    }

    // el archivo
    ModuLibCpp::String FileContent = ModuLibCpp::String{};

    File >> FileContent.InternalString;

    ModuLibCpp::String ContentView{};
    KernelStatus Ed = Editor(FileContent, ContentView);

    if (Ed == KernelStatusSuccess)
    {
        ModuLibCpp::Display::Print(ContentView.InternalString);
        int Size = StrLen(ContentView.InternalString) + 1;
        File << ModuLibCpp::Pair<char*, int>{ContentView.InternalString, Size};
    }
    return KernelStatusSuccess;
}