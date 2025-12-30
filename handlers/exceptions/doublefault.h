#include "../exint.h"

extern void IdtExDoubleFault();

void IdtExDoubleFaultHnd(regs_t* r) {
    // mensaje
    GlobalServices->Display->printg("***Exception*** Double Fault\n");

    // para matar el proceso
    KillProcess(r);
}