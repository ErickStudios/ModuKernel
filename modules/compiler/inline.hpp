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
    /// @brief el offset
    int offset;
    /// @brief constructor
    AssemblyContext(int initialCapacity = 64)
        : Labels(initialCapacity), offset(0)
    {
    }
};
enum Reg8 {
    AL = 0,
    CL = 1,
    DL = 2,
    BL = 3,
    AH = 4,
    CH = 5,
    DH = 6,
    BH = 7,
    NONE8 = -1
};

Reg8 CodifiqueRegister8(ModuLibCpp::String& regStr) {
    // copiarlo
    ModuLibCpp::String reg_upr{regStr.InternalString};
    StrUpr(reg_upr.InternalString);
    if (reg_upr == "AL") return AL;
    if (reg_upr == "CL") return CL;
    if (reg_upr == "DL") return DL;
    if (reg_upr == "BL") return BL;
    if (reg_upr == "AH") return AH;
    if (reg_upr == "CH") return CH;
    if (reg_upr == "DH") return DH;
    if (reg_upr == "BH") return BH;
    return NONE8;
}
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

        if (Instr == (char*)"MOV" || Instr == (char*)"SETVALUE") {
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
                    if (dst.StartsWith('[') && dst.EndsWith(']')) {
                        dst.RemoveFirstChar();
                        dst.RemoveLastChar();
                        Reg32 dstMemReg = CodifiqueRegister(dst);
                        Reg32 srcReg = CodifiqueRegister(src);

                        uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
                        code[0] = 0x89; // mov r/m32, r32
                        code[1] = (0b00 << 6) | ((uint8_t)srcReg << 3) | (uint8_t)dstMemReg;
                        *len = 2;
                        return code;
                    }
                    if (src.StartsWith('[') && src.EndsWith(']')) {
                        src.RemoveFirstChar();
                        src.RemoveLastChar();
                        Reg32 srcMemReg = CodifiqueRegister(src);

                        uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
                        code[0] = 0x8B; // mov r32, r/m32
                        code[1] = (0b00 << 6) | ((uint8_t)dstReg << 3) | (uint8_t)srcMemReg;
                        *len = 2;
                        return code;
                    }

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
        else if (Instr == (char*)"MOVB" || Instr == (char*)"SETVALUEASBYTE") {
            char* ops[2];
            int opCount = StrSplit(dorames[1], ops, ',');
            if (opCount == 2) {
                ModuLibCpp::String dst{ops[0]};
                ModuLibCpp::String src{ops[1]};
                dst.Trim(); src.Trim();

                if (dst.StartsWith('[') && dst.EndsWith(']')) {
                    dst.RemoveFirstChar();
                    dst.RemoveLastChar();
                    Reg32 dstMemReg = CodifiqueRegister(dst);
                    Reg8 srcReg = CodifiqueRegister8(src); // AL, BL, CL, DL, etc.

                    uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
                    code[0] = 0x88; // MOV r/m8, r8
                    code[1] = (0b00 << 6) | ((uint8_t)srcReg << 3) | (uint8_t)dstMemReg;
                    *len = 2;
                    return code;
                }
                else if (src.StartsWith('[') && src.EndsWith(']')) {
                    src.RemoveFirstChar();
                    src.RemoveLastChar();
                    Reg32 srcMemReg = CodifiqueRegister(src);
                    Reg8 dstReg = CodifiqueRegister8(dst); // AL, BL, CL, DL, etc.

                    uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
                    code[0] = 0x8A; // MOV r8, r/m8
                    code[1] = (0b00 << 6) | ((uint8_t)dstReg << 3) | (uint8_t)srcMemReg;
                    *len = 2;
                    return code;
                }
            }
        }
        else if (Instr == (char*)"ADD" || Instr == (char*)"ADDNUMBER") {
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
        else if (Instr == (char*)"SUB" || Instr == (char*)"SUBNUMBER") {
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
        else if (Instr == (char*)"JMP" || Instr == (char*)"CALLWITHOUTSAVE") {
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
        else if (Instr == (char*)"CALL" || Instr == (char*)"CALLPROCEDURE") {
            ModuLibCpp::String target{dorames[1]};
            target.Trim();

            for (AssemblyLabel* Label : context->Labels) {

                if (Label->Name == target) {
                    int instrSize = 5;
                    int32_t rel = Label->Offset - (*code_len + instrSize);

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
            label->Offset = *code_len;
            context->Labels.push(label);
        }
        else if (Instr == (char*)"PUSH" || Instr == (char*)"SAVE") {
            ModuLibCpp::String regStr{dorames[1]};
            regStr.Trim();
            Reg32 reg = CodifiqueRegister(regStr);

            uint8_t* code = (uint8_t*)gMS->AllocatePool(1);
            code[0] = 0x50 + (uint8_t)reg; // PUSH r32
            *len = 1;
            return code;
        }
        else if (Instr == (char*)"POP" || Instr == (char*)"LOAD") {
            ModuLibCpp::String regStr{dorames[1]};
            regStr.Trim();
            Reg32 reg = CodifiqueRegister(regStr);

            uint8_t* code = (uint8_t*)gMS->AllocatePool(1);
            code[0] = 0x58 + (uint8_t)reg; // POP r32
            *len = 1;
            return code;
        }
        else if (Instr == (char*)"INC" || Instr == (char*)"INCREMENT") {
            ModuLibCpp::String regStr{dorames[1]};
            regStr.Trim();
            Reg32 reg = CodifiqueRegister(regStr);

            uint8_t* code = (uint8_t*)gMS->AllocatePool(1);
            code[0] = 0x40 + (uint8_t)reg; // INC r32
            *len = 1;
            return code;
        }
        else if (Instr == (char*)"DEC" || Instr == (char*)"DECREMENT") {
            ModuLibCpp::String regStr{dorames[1]};
            regStr.Trim();
            Reg32 reg = CodifiqueRegister(regStr);

            uint8_t* code = (uint8_t*)gMS->AllocatePool(1);
            code[0] = 0x48 + (uint8_t)reg; // DEC r32
            *len = 1;
            return code;
        }
        else if (Instr == (char*)"RET" || Instr == (char*)"RETURNTOSOURCE") {
            uint8_t* code = (uint8_t*)gMS->AllocatePool(1);
            code[0] = 0xC3; // RET
            *len = 1;
            return code;
        }
        else if (Instr == (char*)"CMP" || Instr == (char*)"COMPARE") {
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
        else if (Instr == (char*)"JE" || Instr == (char*)"JUMPIFEQUAL") {
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
        else if (Instr == (char*)"JNE" || Instr == (char*)"JUMPIFNOTEQUAL") {
            ModuLibCpp::String target{dorames[1]};
            target.Trim();

            for (AssemblyLabel* Label : context->Labels) {
                if (Label->Name == target) {
                    int instrSize = 6; // 0F 85 + rel32
                    int rel = Label->Offset - (*code_len + instrSize);

                    uint8_t* code = (uint8_t*)gMS->AllocatePool(instrSize);
                    code[0] = 0x0F;
                    code[1] = 0x85; // JNE near
                    *(int32_t*)&code[2] = rel;
                    *len = instrSize;
                    return code;
                }
            }
            *len = 6;
        }
        else if (Instr == (char*)"JL" || Instr == (char*)"JUMPIFLESS") {
            ModuLibCpp::String target{dorames[1]};
            target.Trim();

            for (AssemblyLabel* Label : context->Labels) {
                if (Label->Name == target) {
                    int instrSize = 6; // 0F 8C + rel32
                    int rel = Label->Offset - (*code_len + instrSize);

                    uint8_t* code = (uint8_t*)gMS->AllocatePool(instrSize);
                    code[0] = 0x0F;
                    code[1] = 0x8C; // JL near
                    *(int32_t*)&code[2] = rel;
                    *len = instrSize;
                    return code;
                }
            }

            *len = 6;
        }
        else if (Instr == (char*)"JLE" || Instr == (char*)"JUMPIFLESSEQUAL") {
            ModuLibCpp::String target{dorames[1]};
            target.Trim();

            for (AssemblyLabel* Label : context->Labels) {
                if (Label->Name == target) {
                    int instrSize = 6; // 0F 8E + rel32
                    int rel = Label->Offset - (*code_len + instrSize);

                    uint8_t* code = (uint8_t*)gMS->AllocatePool(instrSize);
                    code[0] = 0x0F;
                    code[1] = 0x8E; // JLE near
                    *(int32_t*)&code[2] = rel;
                    *len = instrSize;
                    return code;
                }
            }
            *len = 6;
        }
        else if (Instr == (char*)"JG" || Instr == (char*)"JUMPIFGREATER") {
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
        else if (Instr == (char*)"JGE" || Instr == (char*)"JUMPIFGREATEREQUAL") {
            ModuLibCpp::String target{dorames[1]};
            target.Trim();

            for (AssemblyLabel* Label : context->Labels) {
                if (Label->Name == target) {
                    int instrSize = 6; // 0F 8D + rel32
                    int rel = Label->Offset - (*code_len + instrSize);

                    uint8_t* code = (uint8_t*)gMS->AllocatePool(instrSize);
                    code[0] = 0x0F;
                    code[1] = 0x8D; // JGE near
                    *(int32_t*)&code[2] = rel;
                    *len = instrSize;
                    return code;
                }
            }
            *len = 6;
        }
        else if (Instr == (char*)"JB" || Instr == (char*)"JUMPIFBELOW") {
            ModuLibCpp::String target{dorames[1]};
            target.Trim();

            for (AssemblyLabel* Label : context->Labels) {
                if (Label->Name == target) {
                    int instrSize = 6; // 0F 82 + rel32
                    int rel = Label->Offset - (*code_len + instrSize);

                    uint8_t* code = (uint8_t*)gMS->AllocatePool(instrSize);
                    code[0] = 0x0F;
                    code[1] = 0x82; // JB near
                    *(int32_t*)&code[2] = rel;
                    *len = instrSize;
                    return code;
                }
            }
            *len = 6;
        }
        else if (Instr == (char*)"JBE" || Instr == (char*)"JUMPIFBELOWEQUAL") {
            ModuLibCpp::String target{dorames[1]};
            target.Trim();

            for (AssemblyLabel* Label : context->Labels) {
                if (Label->Name == target) {
                    int instrSize = 6; // 0F 86 + rel32
                    int rel = Label->Offset - (*code_len + instrSize);

                    uint8_t* code = (uint8_t*)gMS->AllocatePool(instrSize);
                    code[0] = 0x0F;
                    code[1] = 0x86; // JBE near
                    *(int32_t*)&code[2] = rel;
                    *len = instrSize;
                    return code;
                }
            }
            *len = 6;
        }
        else if (Instr == (char*)"MUL" || Instr == (char*)"MULTIPLIQUE") {
            ModuLibCpp::String src{dorames[1]};
            src.Trim();
            Reg32 srcReg = CodifiqueRegister(src);

            uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
            code[0] = 0xF7; // grupo de operaciones
            code[1] = (0b11 << 6) | (4 << 3) | (uint8_t)srcReg; // /4 = MUL
            *len = 2;
            return code;
        }
        else if (Instr == (char*)"IMUL" || Instr == (char*)"SIGNEDMULTIPLIQUE") {
            ModuLibCpp::String src{dorames[1]};
            src.Trim();
            Reg32 srcReg = CodifiqueRegister(src);

            uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
            code[0] = 0xF7; // grupo de operaciones
            code[1] = (0b11 << 6) | (5 << 3) | (uint8_t)srcReg; // /5 = IMUL r/m32
            *len = 2;
            return code;
        }
        else if (Instr == (char*)"JC" || Instr == (char*)"JUMPIFCARRY") {
            ModuLibCpp::String target{dorames[1]};
            target.Trim();

            for (AssemblyLabel* Label : context->Labels) {
                if (Label->Name == target) {
                    int instrSize = 6; // 0F 82 + rel32
                    int rel = Label->Offset - (*code_len + instrSize);

                    uint8_t* code = (uint8_t*)gMS->AllocatePool(instrSize);
                    code[0] = 0x0F;
                    code[1] = 0x82; // JC near (igual que JB)
                    *(int32_t*)&code[2] = rel;
                    *len = instrSize;
                    return code;
                }
            }
            *len = 6;
        }
        else if (Instr == (char*)"JNC" || Instr == (char*)"JUMPIFNOTCARRY") {
            ModuLibCpp::String target{dorames[1]};
            target.Trim();

            for (AssemblyLabel* Label : context->Labels) {
                if (Label->Name == target) {
                    int instrSize = 6; // 0F 83 + rel32
                    int rel = Label->Offset - (*code_len + instrSize);

                    uint8_t* code = (uint8_t*)gMS->AllocatePool(instrSize);
                    code[0] = 0x0F;
                    code[1] = 0x83; // JNC near
                    *(int32_t*)&code[2] = rel;
                    *len = instrSize;
                    return code;
                }
            }
            *len = 6;
        }
        else if (Instr == (char*)"DIV" || Instr == (char*)"DIVIDE") {
            ModuLibCpp::String src{dorames[1]};
            src.Trim();
            Reg32 srcReg = CodifiqueRegister(src);

            uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
            code[0] = 0xF7;
            code[1] = (0b11 << 6) | (6 << 3) | (uint8_t)srcReg; // /6 = DIV
            *len = 2;
            return code;
        }
        else if (Instr == (char*)"SIMPLEMULTIPLICATION") {
            char* ops[2];
            int opCount = StrSplit(dorames[1], ops, ',');
            if (opCount == 2) {
                ModuLibCpp::String dst{ops[0]};
                ModuLibCpp::String src{ops[1]};
                dst.Trim(); src.Trim();

                Reg32 dstReg = CodifiqueRegister(dst);
                Reg32 srcReg = CodifiqueRegister(src);

                uint8_t* code = (uint8_t*)gMS->AllocatePool(3);
                code[0] = 0x0F;
                code[1] = 0xAF; // IMUL r32, r/m32
                code[2] = (0b11 << 6) | ((uint8_t)dstReg << 3) | (uint8_t)srcReg;
                *len = 3;
                return code;
            }
        }
        else if (Instr == (char*)"AND") {
            char* ops[2];
            int opCount = StrSplit(dorames[1], ops, ',');
            if (opCount == 2) {
                ModuLibCpp::String dst{ops[0]};
                ModuLibCpp::String src{ops[1]};
                dst.Trim(); src.Trim();

                Reg32 dstReg = CodifiqueRegister(dst);
                Reg32 srcReg = CodifiqueRegister(src);

                uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
                code[0] = 0x21; // AND r/m32, r32
                code[1] = (0b11 << 6) | ((uint8_t)srcReg << 3) | (uint8_t)dstReg;
                *len = 2;
                return code;
            }
        }
        else if (Instr == (char*)"OR") {
            char* ops[2];
            int opCount = StrSplit(dorames[1], ops, ',');
            if (opCount == 2) {
                ModuLibCpp::String dst{ops[0]};
                ModuLibCpp::String src{ops[1]};
                dst.Trim(); src.Trim();

                Reg32 dstReg = CodifiqueRegister(dst);
                Reg32 srcReg = CodifiqueRegister(src);

                uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
                code[0] = 0x09; // OR r/m32, r32
                code[1] = (0b11 << 6) | ((uint8_t)srcReg << 3) | (uint8_t)dstReg;
                *len = 2;
                return code;
            }
        }
        else if (Instr == (char*)"XOR" || Instr == (char*)"EXCLUSIVEOR") {
            char* ops[2];
            int opCount = StrSplit(dorames[1], ops, ',');
            if (opCount == 2) {
                ModuLibCpp::String dst{ops[0]};
                ModuLibCpp::String src{ops[1]};
                dst.Trim(); src.Trim();

                Reg32 dstReg = CodifiqueRegister(dst);
                Reg32 srcReg = CodifiqueRegister(src);

                uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
                code[0] = 0x31; // XOR r/m32, r32
                code[1] = (0b11 << 6) | ((uint8_t)srcReg << 3) | (uint8_t)dstReg;
                *len = 2;
                return code;
            }
        }
        else if (Instr == (char*)"CDQ") {
            uint8_t* code = (uint8_t*)gMS->AllocatePool(1);
            code[0] = 0x99; // CDQ
            *len = 1;
            return code;
        }
        else if (Instr == (char*)"IDIV") {
            ModuLibCpp::String src{dorames[1]};
            src.Trim();
            Reg32 srcReg = CodifiqueRegister(src);

            uint8_t* code = (uint8_t*)gMS->AllocatePool(2);
            code[0] = 0xF7;
            code[1] = (0b11 << 6) | (7 << 3) | (uint8_t)srcReg; // /7 = IDIV
            *len = 2;
            return code;
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

    ModuLibCpp::Array<char*> programLines{};
    int programLinesCount = ModuLibCpp::StrSplitArray(Coppy.InternalString, programLines, '\n');

    // calcular tamaño total
    int totalSize = 0;
    ModuLibCpp::Array<uint8_t*> compiledLines;
    ModuLibCpp::Array<int> compiledLens;

    int code_len1 = 0;

    for (int i = 0; i < programLinesCount; i++) {
        ModuLibCpp::String line{programLines[i]};
        int len2 = 0;

        CodifiqueInstruction(line, &len2, &Context, &code_len1);

        code_len1 += len2;
    }

    for (int i = 0; i < programLines.size(); i++) {
        ModuLibCpp::String line{programLines[i]};
        int len2 = 0;

        uint8_t* lineCompiled = CodifiqueInstruction(line, &len2, &Context, &totalSize);

        compiledLines.push(lineCompiled);
        compiledLens.push(len2);
        totalSize += len2;
    }

    // reservar buffer final
    uint8_t* ProgramCompiled = (uint8_t*)gMS->AllocatePool(totalSize);

    // copiar cada instrucción compilada
    int offset = 0;
    for (int i = 0; i < compiledLines.size(); i++) {
        if (compiledLines[i] != nullptr && compiledLens[i] > 0) {
            gMS->CoppyMemory(ProgramCompiled + offset, compiledLines[i], compiledLens[i]);
            offset += compiledLens[i];
            gMS->FreePool(compiledLines[i]);
        }
    }

    *len = offset;
    return ProgramCompiled;
}
