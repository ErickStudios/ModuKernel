#include "../exint.h"

extern void IdtExPageFault();

void IdtExPageFaultHnd(regs_t* r) {
    // mensaje
    GlobalServices->Display->printg("***Exception*** PageFault\n");

    // para matar el proceso
    KillProcess(r);
}