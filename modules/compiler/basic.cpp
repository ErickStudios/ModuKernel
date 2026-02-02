// la libreria
#include "../../library/lib.hpp"
// el inline
#include "inline.hpp"
/// @brief codifica el registro
/// @param reg el registro
/// @return el registro
Reg32 CodifiqueRegister(ModuLibCpp::String& reg)
{
    // copiarlo
    ModuLibCpp::String reg_upr{reg.InternalString};
    // volverlo mayusculas
    StrUpr(reg_upr.InternalString);

    if (reg_upr == (char*)"EAX") return Reg32::EAX;
    else if (reg_upr == (char*)"ECX") return Reg32::ECX;
    else if (reg_upr == (char*)"EDX") return Reg32::EDX;
    else if (reg_upr == (char*)"EBX") return Reg32::EBX;
    else if (reg_upr == (char*)"ESP") return Reg32::ESP;
    else if (reg_upr == (char*)"ESI") return Reg32::ESI;
    else if (reg_upr == (char*)"EDI") return Reg32::EDI;
    else return Reg32::EAX;
}
/// @brief codifica una instruccion
/// @param instruction la instruccion
/// @return la instruccion
uint8_t* CodifiqueInstruction(ModuLibCpp::String& instruction, int* len)
{
    // copiarlo
    ModuLibCpp::String InstructionInternal{instruction.InternalString};

    char* dorames[3];
    int Instrs = StrSplit(InstructionInternal.InternalString, dorames, ' ');

    if (Instrs != 0)
    {
        ModuLibCpp::String Instr{dorames[0]};

        // volver mayusculas
        StrUpr(Instr.InternalString);

        if (Instr == (char*)"MOV") 
        {
            char* ops[2];
            int opCount = StrSplit(dorames[1], ops, ',');

            if (opCount == 2)
            {
                ModuLibCpp::String dst{ops[0]};
                ModuLibCpp::String src{ops[1]};
                dst.Trim(); src.Trim();

                Reg32 dstReg = CodifiqueRegister(dst);

                if (StrIsNumber(src.InternalString)) {
                    // caso: reg ← imm32
                    uint32_t imm = StringToInt(src.InternalString);
                    uint8_t* code = (uint8_t*)gMS->AllocatePool(5);
                    code[0] = 0xB8 + (uint8_t)dstReg;   // mov reg, imm32
                    *(uint32_t*)&code[1] = imm;         // inmediato en little endian
                    *len = 5;
                    return code;
                } else {
                    // caso: reg ← reg
                    Reg32 srcReg = CodifiqueRegister(src);
                    uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
                    code[0] = 0x89; // mov r/m32, r32
                    code[1] = (0b11 << 6) | ((uint8_t)srcReg << 3) | (uint8_t)dstReg;
                    *len = 2;
                    return code;
                }
            }


        }
        else if (Instr == (char*)"ADD") {
            char* ops[2];
            int opCount = StrSplit(dorames[1], ops, ',');
            if (opCount == 2) {
                ModuLibCpp::String dst{ops[0]};
                ModuLibCpp::String src{ops[1]};
                dst.Trim(); src.Trim();

                Reg32 dstReg = CodifiqueRegister(dst);
                Reg32 srcReg = CodifiqueRegister(src);

                uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
                code[0] = 0x01; // ADD r/m32, r32
                code[1] = (0b11 << 6) | ((uint8_t)srcReg << 3) | (uint8_t)dstReg;
                *len = 2;
                return code;
            }
        }
        else if (Instr == (char*)"SUB") {
            char* ops[2];
            int opCount = StrSplit(dorames[1], ops, ',');
            if (opCount == 2) {
                ModuLibCpp::String dst{ops[0]};
                ModuLibCpp::String src{ops[1]};
                dst.Trim(); src.Trim();

                Reg32 dstReg = CodifiqueRegister(dst);
                Reg32 srcReg = CodifiqueRegister(src);

                uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
                code[0] = 0x29; // SUB r/m32, r32
                code[1] = (0b11 << 6) | ((uint8_t)srcReg << 3) | (uint8_t)dstReg;
                *len = 2;
                return code;
            }
        }
    }

    return nullptr;
}
/// @brief programa codificado
/// @param ProgramAsm programa
/// @return retornar
uint8_t* CodifiqueProgram(ModuLibCpp::String& ProgramAsm, int* len)
{
    ModuLibCpp::String Coppy{ProgramAsm.InternalString};

    char* programLines[256];
    int programLinesCount = StrSplit(Coppy.InternalString, programLines, '\n');

    // calcular tamaño total
    int totalSize = 0;
    uint8_t* compiledLines[256];
    int compiledLens[256];

    for (int i = 0; i < programLinesCount; i++)
    {
        ModuLibCpp::String line{programLines[i]};
        int len2 = 0;

        uint8_t* lineCompiled = CodifiqueInstruction(line, &len2);

        compiledLines[i] = lineCompiled;
        compiledLens[i] = len2;
        totalSize += len2;
    }

    // reservar buffer final
    uint8_t* ProgramCompiled = (uint8_t*)gMS->AllocatePool(totalSize);

    // copiar cada instrucción compilada
    int offset = 0;
    for (int i = 0; i < programLinesCount; i++)
    {
        if (compiledLines[i] != nullptr && compiledLens[i] > 0)
        {
            gMS->CoppyMemory(ProgramCompiled + offset, compiledLines[i], compiledLens[i]);
            offset += compiledLens[i];
            gMS->FreePool(compiledLines[i]); // liberar cada instrucción temporal
        }
    }

    *len = offset;
    return ProgramCompiled;
}
/// punto de entrada
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
        if (i == 1) file = arg;
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