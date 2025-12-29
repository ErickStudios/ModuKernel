#ifndef _RegsDotHHandlers_
#define _RegsDotHHandlers_
typedef struct regs {
    uint32_t gs, fs, es, ds;
    uint32_t edi, esi, ebp, esp, ebx, edx, ecx, eax;
    uint32_t eip, cs, eflags;
} regs_t;

#endif