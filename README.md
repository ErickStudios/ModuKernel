ModuKernel
===============================

basado en: https://github.com/debashisbarman/Simple-Kernel-in-C-and-Assembly

Bienvenidos a ModuKernel! un proyecto basado en otro pero este sirve para enseñar a la comunidad española como hacer un kernel modular

# Caracteristicas

Este kernel Contiene
* Archivos de ejemplo
* Una tabla de servicios del sistema modular y estructurada
* Una shell basica

# Contribuciones

Puede hacer lo siguiente para contribuir
* Puede crear drivers para el sistema
* Puede hacer programas para el sistema
* Puede crear scripts utiles

# Ejemplos

Un ejemplo como base para estas contribuciones puede ser
```c
// incluir libreria
#include "../library/lib.h"

// el punto de entrada estandart
KernelStatus ErickMain(KernelServices* Services)
{
    Services->Display->printg("hello world\n");
    return KernelStatusSuccess;
}
```