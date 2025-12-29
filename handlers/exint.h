#ifndef _ExIntDotH_
#define _ExIntDotH_
extern void* ExceptionHandlePtr;

#include "regs.h"

#define StrOpenCsM "

#define RegisterPrint(reg)                      \
    GlobalServices->Display->printg(#reg "=");    \
    GlobalServices->Display->printg(reg_##reg);   \
    GlobalServices->Display->printg("  ");      
#endif