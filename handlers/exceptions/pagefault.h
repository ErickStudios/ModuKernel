#include "../exint.h"

extern void IdtExPageFault();

void IdtExPageFaultHnd(regs_t* r) {
    uint32_t fault_addr;
    asm volatile("mov %%cr2, %0" : "=r"(fault_addr));

    // mensaje
    GlobalServices->Display->printg("***Exception*** PageFault on \n");

    // para matar el proceso
    KillProcess(r);
}