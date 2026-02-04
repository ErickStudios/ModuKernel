#include "../../library/lib.hpp"
/// el contexto
struct AssemblyContext;
/// @brief registros generales de i386
enum class Reg32 : uint8_t { EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI };
/// @brief las instrucciones disponibles
enum class Instructions : uint8_t { MovRegister = 0x89};
/// @brief codifica el registro
/// @param reg el registro
/// @return el registro
Reg32 CodifiqueRegister(ModuLibCpp::String& reg);
/// @brief codifica una instruccion
/// @param instruction la instruccion
/// @return la instruccion
uint8_t* CodifiqueInstruction(ModuLibCpp::String& instruction, int* len, AssemblyContext* context);
/// @brief el label
struct AssemblyLabel {
    ModuLibCpp::String Name;
    int Offset;
    // Constructor por defecto
    AssemblyLabel() : Name(""), Offset(0) {}

    // Constructor con parámetros
    AssemblyLabel(const char* name, int offset)
        : Name(name), Offset(offset) {}
};
/// @brief el contexto
struct AssemblyContext {
    /// @brief labels
    ModuLibCpp::Array<AssemblyLabel*> Labels;
    /// @brief constructor
    AssemblyContext(int initialCapacity = 64)
        : Labels(initialCapacity)
    {
    }
};
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
uint8_t* CodifiqueInstruction(ModuLibCpp::String& instruction, int* len, AssemblyContext* context, int* code_len)
{
    // copiarlo
    ModuLibCpp::String InstructionInternal{instruction.InternalString};

    InstructionInternal.Trim();

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
        else if (Instr == (char*)"JMP") {
            ModuLibCpp::String target{dorames[1]};
            target.Trim();

            for (AssemblyLabel* Label : context->Labels) {
                if (Label->Name == target) {
                    int instrSize = 5; // JMP rel32 ocupa 5 bytes
                    int rel = Label->Offset - (*code_len + instrSize);

                    uint8_t* code = (uint8_t*)gMS->AllocatePool(instrSize);
                    code[0] = 0xE9; // JMP rel32
                    *(int32_t*)&code[1] = rel;
                    *len = instrSize;
                    return code;
                }
            }

            *len = 5;
        }
        else if (Instr == (char*)"CALL") {
            ModuLibCpp::String target{dorames[1]};
            target.Trim();

            for (AssemblyLabel* Label : context->Labels) {

                if (Label->Name == target) {
                    int instrSize = 5;
                    int rel = Label->Offset - (*code_len + instrSize);

                    uint8_t* code = (uint8_t*)gMS->AllocatePool(instrSize);
                    code[0] = 0xE8; // CALL rel32
                    *(int32_t*)&code[1] = rel;
                    *len = instrSize;
                    return code;
                }
            }

            *len = 5;
        }
        else if (Instr.EndsWith(':')) {
            Instr.RemoveLastChar();
            AssemblyLabel* label = new AssemblyLabel();
            label->Name.SetString(Instr.InternalString);
            label->Offset = *len;
            context->Labels.push(label);

        }
        else if (Instr == (char*)"PUSH") {
            ModuLibCpp::String regStr{dorames[1]};
            regStr.Trim();
            Reg32 reg = CodifiqueRegister(regStr);

            uint8_t* code = (uint8_t*)gMS->AllocatePool(1);
            code[0] = 0x50 + (uint8_t)reg; // PUSH r32
            *len = 1;
            return code;
        }
        else if (Instr == (char*)"POP") {
            ModuLibCpp::String regStr{dorames[1]};
            regStr.Trim();
            Reg32 reg = CodifiqueRegister(regStr);

            uint8_t* code = (uint8_t*)gMS->AllocatePool(1);
            code[0] = 0x58 + (uint8_t)reg; // POP r32
            *len = 1;
            return code;
        }
        else if (Instr == (char*)"INC") {
            ModuLibCpp::String regStr{dorames[1]};
            regStr.Trim();
            Reg32 reg = CodifiqueRegister(regStr);

            uint8_t* code = (uint8_t*)gMS->AllocatePool(1);
            code[0] = 0x40 + (uint8_t)reg; // INC r32
            *len = 1;
            return code;
        }
        else if (Instr == (char*)"DEC") {
            ModuLibCpp::String regStr{dorames[1]};
            regStr.Trim();
            Reg32 reg = CodifiqueRegister(regStr);

            uint8_t* code = (uint8_t*)gMS->AllocatePool(1);
            code[0] = 0x48 + (uint8_t)reg; // DEC r32
            *len = 1;
            return code;
        }
        else if (Instr == (char*)"RET") {
            uint8_t* code = (uint8_t*)gMS->AllocatePool(1);
            code[0] = 0xC3; // RET
            *len = 1;
            return code;
        }
        else if (Instr == (char*)"CMP") {
            char* ops[2];
            int opCount = StrSplit(dorames[1], ops, ',');
            if (opCount == 2) {
                ModuLibCpp::String dst{ops[0]};
                ModuLibCpp::String src{ops[1]};
                dst.Trim(); src.Trim();

                Reg32 dstReg = CodifiqueRegister(dst);
                Reg32 srcReg = CodifiqueRegister(src);

                uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
                code[0] = 0x39; // CMP r/m32, r32
                code[1] = (0b11 << 6) | ((uint8_t)srcReg << 3) | (uint8_t)dstReg;
                *len = 2;
                return code;
            }
        }
        else if (Instr == (char*)"JE") {
            ModuLibCpp::String target{dorames[1]};
            target.Trim();

            ModuLibCpp::Display::Print(target.InternalString);

            for (AssemblyLabel* Label : context->Labels) {
                if (Label->Name == target) {
                    int instrSize = 6; // 0F 84 + rel32
                    int rel = Label->Offset - (*code_len + instrSize);

                    uint8_t* code = (uint8_t*)gMS->AllocatePool(instrSize);
                    code[0] = 0x0F;
                    code[1] = 0x84; // JE near
                    *(int32_t*)&code[2] = rel;
                    *len = instrSize;
                    return code;
                }
            }
        
            *len = 6;
        }
        else if (Instr == (char*)"JG") {
            ModuLibCpp::String target{dorames[1]};
            target.Trim();

            for (AssemblyLabel* Label : context->Labels) {
                if (Label->Name == target) {
                    int instrSize = 6; // 0F 8F + rel32
                    int rel = Label->Offset - (*code_len + instrSize);

                    uint8_t* code = (uint8_t*)gMS->AllocatePool(instrSize);
                    code[0] = 0x0F;
                    code[1] = 0x8F; // JG near
                    *(int32_t*)&code[2] = rel;
                    *len = instrSize;
                    return code;
                }
            }

            *len = 6;
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
    AssemblyContext Context{};

    char* programLines[256];
    int programLinesCount = StrSplit(Coppy.InternalString, programLines, '\n');

    // calcular tamaño total
    int totalSize = 0;
    uint8_t* compiledLines[256];
    int compiledLens[256];

    int code_len1 = 0;

    for (int i = 0; i < programLinesCount; i++) {
        ModuLibCpp::String line{programLines[i]};
        int len2 = 0;

        CodifiqueInstruction(line, &len2, &Context, &code_len1);

        code_len1 += len2;
    }

    for (int i = 0; i < programLinesCount; i++)
    {
        ModuLibCpp::String line{programLines[i]};
        int len2 = 0;

        uint8_t* lineCompiled = CodifiqueInstruction(line, &len2, &Context, &totalSize);

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
