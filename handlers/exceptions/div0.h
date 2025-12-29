#include "../exint.h"

extern void IdtDivideBy0();

void IdtDivideBy0Hnd(regs_t* r) {
    // mensaje
    GlobalServices->Display->printg("***Exception*** Divide by 0\n");

    // para matar el proceso
    KillProcess(r);
}