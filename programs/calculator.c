#include "../library/lib.h"

typedef struct {
    Entero nums[2];
} Operacion;

Entero Resultado = 0;

Entero VariableA = 0, VariableB = 0, VariableC = 0, VariableD = 0;
Entero VariableE = 0, VariableF = 0, VariableX = 0, VariableY = 0, VariableM = 0;

Operacion PedirOperacion(KernelServices* Servicios)
{
    Operacion Op;
    Servicios->Display->printg("1: ");
    Op.nums[0] = Expresion(Servicios->InputOutput->ReadLine());
    Servicios->Display->printg("\n");

    Servicios->Display->printg("2: ");
    Op.nums[1] = Expresion(Servicios->InputOutput->ReadLine());
    Servicios->Display->printg("\n");

    return Op;
}

Entero Expresion(char* exp)
{
    if (StrCmp(exp, "ans") == 0) return Resultado;
    else if (CharToUpCase(exp[0]) == 'A') return VariableA;
    else if (CharToUpCase(exp[0]) == 'B') return VariableB;
    else if (CharToUpCase(exp[0]) == 'C') return VariableC;
    else if (CharToUpCase(exp[0]) == 'D') return VariableD;
    else if (CharToUpCase(exp[0]) == 'E') return VariableE;
    else if (CharToUpCase(exp[0]) == 'F') return VariableF;
    else if (CharToUpCase(exp[0]) == 'X') return VariableX;
    else if (CharToUpCase(exp[0]) == 'Y') return VariableY;
    else if (CharToUpCase(exp[0]) == 'M') return VariableM;
    else return StringToInt(exp);
}

KernelStatus ErickMain(KernelServices* Services)
{
    for (;;) {
        Services->Display->printg("Calc: ");
        Corto* Action = Services->InputOutput->ReadLine();

        Services->Display->printg("\n");

        if (StrCmp(Action, "sumar") == 0) {
            Operacion Op = PedirOperacion(Services);
            Resultado = Op.nums[0] + Op.nums[1];
        }
        else if (StrCmp(Action, "restar") == 0) {
            Operacion Op = PedirOperacion(Services);
            Resultado = Op.nums[0] - Op.nums[1];
        }
        else if (StrCmp(Action, "multiplicar") == 0) {
            Operacion Op = PedirOperacion(Services);
            Resultado = Op.nums[0] * Op.nums[1];
        }
        else if (StrCmp(Action, "dividir") == 0) {
            Operacion Op = PedirOperacion(Services);
            Resultado = Op.nums[0] / Op.nums[1];
        }
        else if (StrCmp(Action, "salir") == 0) {
            Services->Memory->FreePool(Action);
            return KernelStatusSuccess;
        }
        else if (StrnCmp(Action, "set ", 4) == 0) {
            switch (CharToUpCase(Action[4])) {
                case 'A': VariableA = Resultado; break;
            }
        }
        else if (Expresion(Action) != 0)
        {
            Resultado = Expresion(Action);
        }

        char Buff[13];
        IntToString(Resultado, Buff);
        Services->Display->printg(Buff);
        Services->Display->printg("\n");

        Services->Memory->FreePool(Action);
    }

    return KernelStatusDisaster;
}