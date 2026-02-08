#include "exint.h"

extern void* InCaseOfViolationOfSecurity;

void KillProcess(regs_t* r)
{
    char reg_gs[9]; char reg_fs[9]; char reg_es[9]; char reg_ds[9];                             
    char reg_edi[9]; char reg_esi[9]; char reg_ebp[9]; char reg_esp[9]; char reg_ebx[9];        
    char reg_edx[9]; char reg_ecx[9]; char reg_eax[9];                                          
    char reg_eip[9]; char reg_cs[9]; char reg_eflags[9];                                        
    IntToHexString(r->gs, reg_gs); IntToHexString(r->fs, reg_fs); IntToHexString(r->es, reg_es);
    IntToHexString(r->ds, reg_ds);                                                              
    IntToHexString(r->edi, reg_edi); IntToHexString(r->esi, reg_esi);                           
    IntToHexString(r->ebp, reg_ebp); IntToHexString(r->esp, reg_esp);                          
    IntToHexString(r->ebx, reg_ebx); IntToHexString(r->edx, reg_edx);                           
    IntToHexString(r->ecx, reg_ecx); IntToHexString(r->eax, reg_eax);                           
    IntToHexString(r->eip, reg_eip); IntToHexString(r->cs, reg_cs);                             
    IntToHexString(r->eflags, reg_eflags);                                                      
    RegisterPrint(gs); RegisterPrint(fs); RegisterPrint(es); RegisterPrint(ds);            
    GlobalServices->Display->printg("\n");                             
    RegisterPrint(edi); RegisterPrint(esi); RegisterPrint(ebp); RegisterPrint(esp);             
    RegisterPrint(ebx); RegisterPrint(edx); RegisterPrint(ecx); RegisterPrint(eax);       
    GlobalServices->Display->printg("\n");                              
    RegisterPrint(eip); RegisterPrint(cs); RegisterPrint(eflags);        
    GlobalServices->Display->printg("\n");
    if (r->eip == 0) {
        r->eip = (uint32_t)InCaseOfViolationOfSecurity;
    }
    else if (ExceptionHandlePtr) r->eip = (uint32_t)ExceptionHandlePtr;
}

#include "exceptions/div0.h"
#include "exceptions/overflow.h"
#include "exceptions/pagefault.h"
#include "exceptions/doublefault.h"