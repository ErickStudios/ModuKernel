/** moducandy.cpp, es el compilador de ModuCandy un lenguaje diseñado para modukernel y
 * para trabajar cerca del metal */

#include "../../library/lib.hpp"
// el inline
#include "inline.hpp"
/// @brief tipos de variables
enum VariableBuiltInTypes {
    BuiltIn_u8,
    BuiltIn_i8,
    BuiltIn_u32,
    BuiltIn_i32
};
/// @brief las variables
struct VariableModuCandy {
    ModuLibCpp::String Name;
    int Offset;
    VariableBuiltInTypes Type;

    // Constructor por defecto
    VariableModuCandy()
        : Name(""), Offset(0), Type(BuiltIn_u8) {}

    // Constructor con parámetros
    VariableModuCandy(const ModuLibCpp::String& n, int off, VariableBuiltInTypes t)
        : Name(n), Offset(off), Type(t) {}
};
/// @brief convierte un candy var (r1, r2, r3, ...) a registros (eax, ecx, edx, ...)
/// @param CandyVar el registro
/// @return el registro
ModuLibCpp::String CandyVarToRegister(ModuLibCpp::String& CandyVar)
{
    ///EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI
    if (CandyVar == "r1") return ModuLibCpp::String{"eax"};
    else if (CandyVar == "r2") return ModuLibCpp::String{"ecx"};
    else if (CandyVar == "r3") return ModuLibCpp::String{"edx"};
    else if (CandyVar == "r4") return ModuLibCpp::String{"ebx"};
    else if (CandyVar == "r5") return ModuLibCpp::String{"esp"};
    else if (CandyVar == "r6") return ModuLibCpp::String{"ebp"};
    else if (CandyVar == "r7") return ModuLibCpp::String{"esi"};
    else if (CandyVar == "r8") return ModuLibCpp::String{"edi"};
    else return CandyVar;
}
/// @brief si es una letra
/// @param c el caracter
/// @return si es una letra
bool IsLetter(char c) {
    // Letras mayúsculas
    if (c >= 'A' && c <= 'Z') return true;
    // Letras minúsculas
    if (c >= 'a' && c <= 'z') return true;
    // Dígitos
    if (c >= '0' && c <= '9') return true;
    // Guion bajo
    if (c == '_') return true;
    // si es
    if (c == '[' || c == ']') return true;

    return false;
}
/// @brief parsea un codigo ModuCandy a ensamblador MAS
/// @param Code el codigo
/// @return el codigo ensamblador
ModuLibCpp::String ParseCode(ModuLibCpp::String& Code)
{
    ModuLibCpp::String CodeRet{""};
    ModuLibCpp::String WordSymbol{""};
    ModuLibCpp::String TypeUse{""};
    ModuLibCpp::String DataSection{"DATA_MODUCANDY_DATA:\n   call DATA_MODUCANDY_DATA_GETEIP\n   ret\n   ;datas\n"};
    ModuLibCpp::String DatasReal{""};
    ModuLibCpp::String DatasFunctions{"; ModuCandy: eip get runtime for data calc\nDATA_MODUCANDY_DATA_GETEIP:\n   pop eax\n   push eax\n   inc eax\n   ret"};

    ModuLibCpp::Array<VariableModuCandy> Variables{};
    int offset_actual = 0;

    bool in_comment = false;
    bool type_usage = false;

    for (auto it = Code.begin(); it != Code.end(); ++it) {
        char c = *it;

        if (in_comment)
        {
            CodeRet << c;
            if (c == '\n') in_comment = false;
            continue;
        }

        if (IsLetter(c))
        {
            if (StrLen(WordSymbol.InternalString) == 0)
            {
                if ((c >= 'A' && c <= 'Z'))
                {
                    type_usage = true;
                }      
            }

            WordSymbol << c;
        }
        else {
            if (type_usage) 
            {
                TypeUse.SetString(WordSymbol.InternalString);
                type_usage = false;
            }
            // el string
            else if (StrCmp(TypeUse.InternalString, "") != 0)
            {                
                // funciones
                if (WordSymbol == "fn") {
                    // avanzar hasta el primer caracter no espacio
                    ++it;
                    while (*it == ' ' || *it == '\t') ++it;

                    // acumular el nombre de la función
                    ModuLibCpp::String FuncName{""};
                    while (IsLetter(*it)) {
                        FuncName << *it;
                        ++it;
                    }

                    CodeRet << ModuLibCpp::String{"; type="} << TypeUse << ModuLibCpp::String{"\n"} << FuncName << ModuLibCpp::String{":\n"};
                }
                // funciones
                else if (WordSymbol == "let") {
                    // avanzar hasta el primer caracter no espacio
                    ++it;
                    while (*it == ' ' || *it == '\t') ++it;

                    // acumular el nombre de la función
                    ModuLibCpp::String FuncName{""};
                    while (IsLetter(*it)) {
                        FuncName << *it;
                        ++it;
                    }

                    if (TypeUse == "BuiltIn_u32" || TypeUse == "BuiltIn_i32") { 
                        for (size_t i = 0; i < 4; i++) DatasReal << "   ret ;place for variable (u/i32)\n"; 
                        VariableModuCandy Variable;
                        Variable.Name.SetString(FuncName.InternalString);
                        Variable.Type == VariableBuiltInTypes::BuiltIn_u32;
                        Variable.Offset = offset_actual;
                        offset_actual+=4;
                        Variables.push(Variable);
                    }
                    else if (TypeUse == "BuiltIn_u8" || TypeUse == "BuiltIn_i8") { 
                        DatasReal << "   ret ;place for variable (u/i8)\n"; 
                        VariableModuCandy Variable;
                        Variable.Name.SetString(FuncName.InternalString);
                        Variable.Type == VariableBuiltInTypes::BuiltIn_u8;
                        Variable.Offset = offset_actual;
                        offset_actual++;
                        Variables.push(Variable);
                    }

                }
                TypeUse.ClearString();
            }

            if (WordSymbol == "return") {
                CodeRet << ModuLibCpp::String{"ret\n"};
            }
            else if (WordSymbol == "unsafe") {
                it++;
                if (*it == '"') {
                    ModuLibCpp::String AssemblyCode{""};
                    it++;
                    while ((*it) != '"') {AssemblyCode << *it;++it;}

                    CodeRet << AssemblyCode << ModuLibCpp::String{"\n"};
                }
            }
            else if (c == '/' && *(it + 1) == '/') {
                it++;
                in_comment = true;
                CodeRet << ';';
            }
            else if (c == '(' && *(it + 1) == ')')
            {
                it++;
                StrUpr(WordSymbol.InternalString);

                CodeRet << ModuLibCpp::String{"call "} << CandyVarToRegister(WordSymbol) << ModuLibCpp::String{"\n"};
            }
            else if (c == '<' && *(it + 1) == '=')
            {
                it+=2;
                // acumular el nombre de la función
                ModuLibCpp::String CandyVar{""};
                while (IsLetter(*it)) {CandyVar << *it;++it;}
                for (auto& i : Variables) {
                    if (i.Name == CandyVar)
                    {
                        char inta[30];
                        IntToString(i.Offset, inta);

                        CodeRet <<  ModuLibCpp::String{"call DATA_MODUCANDY_DATA\n"} <<
                            ModuLibCpp::String{"mov ebx,"} << ModuLibCpp::String{((const char*)inta)} << ModuLibCpp::String{"\n"} <<
                            ModuLibCpp::String{"add eax,ebx\n"};

                        if (i.Type == VariableBuiltInTypes::BuiltIn_u8) {
                            CodeRet << ModuLibCpp::String{"mov "} << CandyVarToRegister(WordSymbol) <<  ModuLibCpp::String{",[eax]\n"} ;
                        }
                        else {
                            CodeRet << ModuLibCpp::String{"movb al,[eax]\n"} <<
                                        ModuLibCpp::String{"mov "} << CandyVarToRegister(WordSymbol) << ModuLibCpp::String{",eax\n"};
                        }
                    }
                }
            }
            else if (c == '?')
            {
                it++;
                // acumular el nombre de la función
                ModuLibCpp::String CandyVar{""};
                while (IsLetter(*it)) {CandyVar << *it;++it;}

                CodeRet << ModuLibCpp::String{"cmp "} << CandyVarToRegister(WordSymbol) << ModuLibCpp::String{","} << CandyVarToRegister(CandyVar) << ModuLibCpp::String{"\n"};
            }
            else if (c == '=' && WordSymbol == "") {
                it++;
                // acumular el nombre de la función
                ModuLibCpp::String CandyVar{""};
                while (IsLetter(*it)) {CandyVar << *it;++it;}

                StrUpr(CandyVar.InternalString);
                CodeRet << ModuLibCpp::String{"je "} << CandyVarToRegister(CandyVar) << ModuLibCpp::String{"\n"};
            }
            else if (c == '>' && WordSymbol == "") {
                it++;
                // acumular el nombre de la función
                ModuLibCpp::String CandyVar{""};
                while (IsLetter(*it)) {CandyVar << *it;++it;}

                StrUpr(CandyVar.InternalString);
                CodeRet << ModuLibCpp::String{"jg "} << CandyVarToRegister(CandyVar) << ModuLibCpp::String{"\n"};
            }
            else if (c == '+')
            {
                it++;
                // acumular el nombre de la función
                ModuLibCpp::String CandyVar{""};
                while (IsLetter(*it)) {CandyVar << *it;++it;}

                CodeRet << ModuLibCpp::String{"add "} << CandyVarToRegister(WordSymbol) << ModuLibCpp::String{","} << CandyVarToRegister(CandyVar) << ModuLibCpp::String{"\n"};
            }
            else if (c == '*')
            {
                it++;
                // acumular el nombre de la función
                ModuLibCpp::String CandyVar{""};
                while (IsLetter(*it)) {CandyVar << *it;++it;}

                CodeRet << ModuLibCpp::String{"SimpleMultiplication "} << CandyVarToRegister(WordSymbol) << ModuLibCpp::String{","} << CandyVarToRegister(CandyVar) << ModuLibCpp::String{"\n"};
            }
            else if (c == '/')
            {
                it++;
                // acumular el nombre de la función
                ModuLibCpp::String CandyVar{""};
                while (IsLetter(*it)) {CandyVar << *it;++it;}

                CodeRet << ModuLibCpp::String{"push eax\n"} 
                        << ModuLibCpp::String{"push edx\n"} 
                        << ModuLibCpp::String{"mov eax,"} << CandyVarToRegister(WordSymbol) << ModuLibCpp::String{"\n"}
                        << ModuLibCpp::String{"cdq\n"} 
                        << ModuLibCpp::String{"idiv "} << CandyVarToRegister(CandyVar) << ModuLibCpp::String{"\n"}
                        << ModuLibCpp::String{"mov "} << CandyVarToRegister(WordSymbol) << ModuLibCpp::String{",eax\n"}
                        << ModuLibCpp::String{"pop edx\n"} 
                        << ModuLibCpp::String{"pop eax\n"} 
                    ;
            }
            else if (c == '<' && *(it + 1) == '-')
            {
                it+=2;

                CodeRet << ModuLibCpp::String{"push "} << CandyVarToRegister(WordSymbol) << ModuLibCpp::String{"\n"};
            }
            else if (c == '-' && *(it + 1) == '>')
            {
                it+=2;
                ModuLibCpp::String CandyVar{""};
                while (IsLetter(*it)) {CandyVar << *it;++it;}

                CodeRet << ModuLibCpp::String{"pop "} << CandyVarToRegister(CandyVar) << ModuLibCpp::String{"\n"};
            }
            else if (c == '-')
            {
                it++;
                // acumular el nombre de la función
                ModuLibCpp::String CandyVar{""};
                while (IsLetter(*it)) {CandyVar << *it;++it;}

                CodeRet << ModuLibCpp::String{"sub "} << CandyVarToRegister(WordSymbol) << ModuLibCpp::String{","} << CandyVarToRegister(CandyVar) << ModuLibCpp::String{"\n"};
            }
            else if (c == '=')
            {
                it++;
                // acumular el nombre de la función
                ModuLibCpp::String CandyVar{""};
                while (IsLetter(*it)) {CandyVar << *it;++it;}

                CodeRet << ModuLibCpp::String{"mov "} << CandyVarToRegister(WordSymbol) << ModuLibCpp::String{","} << CandyVarToRegister(CandyVar) << ModuLibCpp::String{"\n"};
            }

            WordSymbol.ClearString();
        }
    }

    ModuLibCpp::String FinalCode{CodeRet.InternalString};
    FinalCode << DataSection << DatasReal << DatasFunctions;

    return FinalCode;
}
/// @brief la shell para programar en ModuCandy
void ModuCandyShell()
{
    ModuLibCpp::String Code{""};

    while (true)
    {
        char* line = gSys->InputOutput->ReadLine();
        ModuLibCpp::String inlinea{line};

        if (inlinea == ":run") {
            ModuLibCpp::String CodeCompiled = ParseCode(Code);
            int len = 0;
            uint8_t* Buffer = CodifiqueProgram(CodeCompiled, &len);
            typedef int (*EntryPoint)();
            EntryPoint fn = (EntryPoint)Buffer;
            int Retval = fn();
            ModuLibCpp::Display::Print("\nReturnValue: ");
            ModuLibCpp::Display::PrintInt(Retval);
            ModuLibCpp::Display::Print("\n");
        }
        else if (inlinea == ":cls")
        {
            Code.ClearString();
        }
        else if (inlinea == ":asm")
        {
            ModuLibCpp::String CodeCompiled = ParseCode(Code);
            ModuLibCpp::Display::Print("\n");
            ModuLibCpp::Display::Print(CodeCompiled.InternalString);
            ModuLibCpp::Display::Print("\n");
        }
        else if (inlinea == ":q")
        {
            ModuLibCpp::Display::Print("\n");
            return;
        }
        else {
            Code << inlinea << "\n";
        }
        ModuLibCpp::Display::Print("\n");
    }
}
/// @brief funcion principal
/// @param Services los servicios
/// @return el estado
extern "C" KernelStatus ErickMain(KernelServices* Services)
{

    // inicializar libreria
    InitializeLibrary(Services);

    // argumentos
    GetArgs(Services, count);

    // archivo
    char* file;

    // recorrer parametros
    for (int i = 1; i < count; i++) {
        // argumento
        char* arg = (char*)Services->Misc->Paramaters[i + 2];

        // si el parametro es el primero, es el archivo
        if (i == 1) 
        { 
            file = arg;
            if (StrCmp(file, "--s") == 0)
            {
                ModuCandyShell();
                return KernelStatusSuccess;
            }
        }
    }

    // archivo
    FatFile ModuCandyFile = Services->File->OpenFile(file);
    ObjectAny ModuCandyFileContent;
    Entero ModuCandyFileSize;
    KernelStatus ModuCandyOpen = (KernelStatus)gSys->File->GetFile(ModuCandyFile, &ModuCandyFileContent, &ModuCandyFileSize);

    // si no se pudo abrir
    if (ModuCandyOpen) return ModuCandyOpen;

    // el archivo
    ModuLibCpp::String* FileContent = new ModuLibCpp::String((const char*)ModuCandyFileContent);

    ModuLibCpp::String Code = ParseCode(*FileContent);

    int len = 0;

    uint8_t* Buffer = CodifiqueProgram(Code, &len);

    typedef int (*EntryPoint)();
    EntryPoint fn = (EntryPoint)Buffer;
    int Retval = fn();
    ModuLibCpp::Display::Print("\nReturnValue: ");
    ModuLibCpp::Display::PrintInt(Retval);
    ModuLibCpp::Display::Print("\n");

    delete FileContent;

    // liberar contenido
    gMS->FreePool(ModuCandyFileContent);
    // cerrar archivo
    gSys->File->CloseFile(ModuCandyFile);

    return KernelStatusSuccess;
}