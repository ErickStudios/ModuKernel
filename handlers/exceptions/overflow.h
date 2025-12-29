#include "../exint.h"

extern void IdtExOverflow();

void IdtExOverflowHnd(regs_t* r) {
    // mensaje
    GlobalServices->Display->printg("***Exception*** Overflow\n");

    // para matar el proceso
    KillProcess(r);
}