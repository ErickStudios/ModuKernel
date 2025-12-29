// comando de ayuda
#include "../library/lib.h"

// esto
KernelStatus ErickMain(KernelServices* Serv)
{
    InitializeLibrary(Serv);

    // obtener argumentos
    GetArgs(Serv, count);
    // ejecucion
    GetMagic(Serv, execType);

    // comando
    char* Comando;

    // argumentos
    for (int i = 1; i < count; i++) {
        // parametro
        char* arg = (char*)Serv->Misc->Paramaters[i + 2];
        // si es el comando
        if (i == 1) Comando = arg;
        else {
            Serv->Display->printg("flag '");
            Serv->Display->printg(arg);
            Serv->Display->printg("' no correcta\n");
            return KernelStatusDisaster;
        }
    }

    //
    // compararlo
    //

    if (StrCmp(Comando, "echo") == 0)
    {
        Serv->Display->printg("((echo Text:(char*)):(void))\n");
        Serv->Display->printg("Sirve para imprimir algo en la consola con un salto de linea al final\n");
    }
    else if (StrCmp(Comando, "cls") == 0)
    {
        Serv->Display->printg("((cls):(void))\n");
        Serv->Display->printg("limpia la pantalla\n");
    }
    else if (StrCmp(Comando, "cd") == 0)
    {
        Serv->Display->printg("((cd Dir:(char*)):(void))\n");
        Serv->Display->printg("Sirve para cambiar de directorio\n");
    }
    else if (StrCmp(Comando, "ls") == 0)
    {
        Serv->Display->printg("((ls):(void))\n");
        Serv->Display->printg("lista los archivos en el directorio actual\n");
    }
    else if (StrCmp(Comando, "drvload") == 0)
    {
        Serv->Display->printg("((drvload File:(char*)):(void))\n");
        Serv->Display->printg("carga un archivo como driver\n");
    }
    else if (StrCmp(Comando, "reset") == 0)
    {
        Serv->Display->printg("((reset):(void))\n");
        Serv->Display->printg("reinicia la maquina\n");
    }
    else if (StrCmp(Comando, "reset") == 0)
    {
        Serv->Display->printg("((shutdown):(void))\n");
        Serv->Display->printg("apaga la maquina\n");
    }
    else if (StrCmp(Comando, "modush") == 0)
    {
        Serv->Display->printg("((modush File:(char*)):(void))\n");
        Serv->Display->printg("ejecuta un script de ModuShell\n");
    }
    else if (StrCmp(Comando, "cat") == 0)
    {
        Serv->Display->printg("((cat File:(char*)):(void))\n");
        Serv->Display->printg("imprime el contenido de un archivo\n");
    }
    else if (StrCmp(Comando, "time") == 0)
    {
        Serv->Display->printg("((time):(void))\n");
        Serv->Display->printg("muestra la hora\n");
    }
    else
    {
        Serv->Display->printg("modush: comando '");
        Serv->Display->printg(Comando);
        Serv->Display->printg("' no documentado o no encontrado\n");
    }

    return KernelStatusSuccess;
}