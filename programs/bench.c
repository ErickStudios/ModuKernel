// benchmark
#include "../library/lib.h"

KernelStatus ErickMain(KernelServices* Services)
{
    // un MiniPositivo (unsigned char)
    MiniPositivo Bench = 1;
    // el inicio
    GrandePositivo start = Services->Misc->GetTicks();
    // operacion
    for (int i = 0; i < 1000000; i++) Bench += 1;
    // el final
    GrandePositivo end = Services->Misc->GetTicks();
    // ciclos
    Services->Display->printg("Ciclos: ");

    // convertir
    char intar[13]; IntToString((end - start), intar);

    // imprimir ciclos
    Services->Display->printg(intar);
    Services->Display->printg("\n");

    return KernelStatusSuccess;
}