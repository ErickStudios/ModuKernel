bits 32

%macro MakeInstruction 2
    %2 %1, eax
    %2 %1, 0x12345678
%endmacro

%macro OperateRegisters 1
    MakeInstruction %1, mov
    MakeInstruction %1, add
    MakeInstruction %1, sub
    MakeInstruction %1, or
    MakeInstruction %1, and
    MakeInstruction %1, xor
%endmacro

%macro OperateMulDiv 1
    mul %1
    div %1
%endmacro

; Genera combinaciones válidas
OperateRegisters eax
OperateRegisters ebx
OperateRegisters ecx
OperateRegisters edx
OperateRegisters esi
OperateRegisters edi
OperateRegisters ebp
OperateRegisters esp

OperateMulDiv eax
OperateMulDiv ebx
OperateMulDiv ecx
OperateMulDiv edx
OperateMulDiv esi
OperateMulDiv edi
OperateMulDiv ebp
OperateMulDiv esp

call xd
jmp xd2

xd:
    ret

xd2:
    nop