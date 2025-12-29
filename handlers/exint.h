#ifndef _ExIntDotH_
#define _ExIntDotH_
extern void* ExceptionHandlePtr;

typedef struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t eip, cs, eflags;
} regs_t;

#define StrOpenCsM "

#define RegisterPrint(reg)                      \
    GlobalServices->Display->printg(#reg "=");    \
    GlobalServices->Display->printg(reg_##reg);   \
    GlobalServices->Display->printg("  ");      
#endif