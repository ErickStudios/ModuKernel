/* shell, escrita para encajar como modulo para entrar en init despues de haber
entrado */

#include "../../library/lib.h"

/* funcion principal */
KernelStatus ErickMain(KernelServices* Services)
{
    // prompt
    char* Prompt = 0;
    // impresion de prompt
    char* PromptPrintCmd = "prp";
    
    /* loop de la shell*/
    while (1)
    {
        // ejecutar
        Services->Misc->Run(Services, PromptPrintCmd, 0);

        // lee la linea
        Prompt = Services->InputOutput->ReadLine();

        Services->Display->printg("\n");

        // salir
        if (StrCmp(Prompt, "exit") == 0) return KernelStatusSuccess;
        // si no, ejecutar
        else Services->Misc->Run(Services, Prompt, 0);

        // liberarlo
        Services->Memory->FreePool(Prompt);
    }
}