ModuKernel
===============================

basado en: https://github.com/debashisbarman/Simple-Kernel-in-C-and-Assembly

Bienvenidos a ModuKernel! un proyecto basado en otro pero este sirve para enseñar a la comunidad española como hacer un kernel modular

# Caracteristicas

Este kernel Contiene
* Archivos de ejemplo
* Una tabla de servicios del sistema modular y estructurada
* Una shell basica

## Compatibilidad con unidades

por ahora estas son las unidades de almacenamiento que ofrece y estan programadas para funcionar o disque funcionar y poder ser montadas y leerlas

| Disco | Estado  | Probado      |
| ------------- | ------------- | ------------- |
| HardDisk | ✅ | Es el formato original |
| FloppyDisk | ⚠️ | Probado, funciona a medias, no soporta FSLST.IFS |
| CD-ROM | ❌ | No garantizado |

## Compatibilidad con drivers

por ahora estos son los drivers probados y que funcionan a medias

| Driver | Estado | Probado      |
| ------------- | ------------- | ------------- |
| Teclado | ✅ | Soporte primitivo, estable |
| Mouse | ⚠️ | Soporte adicional, solo con funciones externas al kernel, inestable |
| VGA (Modo texto) | ✅ | Soporte primitivo, estable |
| VGA (Modo grafico) | ⚠️ | Inestable y buggeado, cada byte del display ocupa 4 pixeles |

## Compatibilidad con sistemas de archivos

por ahora solo se permite FAT12 pero puede extenderlo y volverlo mas complejo añadiendo el archivo `FSLST.IFS` a la unidad para que admita rutas y carpetas virtuales

| Sistema de archivos | Estado | Probado      |
| ------------- | ------------- | ------------- |
| FAT12 | ✅ | Soporte primitivo, estable |

# Detalles pequeños del sistema

Estas son las diferencias al estandart
* La tecla **Shift** no funciona, para los caracteres especiales se requiere activar el **Caps Lock**, que es lo mismo que el shift, solo que requieres desactivarlo para volver a usar su contraparte
* Como plus, la tecla **Caps Lock** tambien cumple su funcion de volver mayusculas las teclas de las letras
* El teclado no se adapta, siempre usa el **layout ingles**
* Los simbolos y el keymap aunque usan el ingles no es el mismo, usa uno parecido pero por limitaciones require que unas teclas sean diferentes
* **No hay soporte para paginacion**
* Muchas funciones como el **modo grafico** estan un poco buggeadas asi que solo utilizalas cuando las necesties
* El modo grafico esta tan bugeado que **no se puede volver al modo texto** por algun bug debido a esto, el sistema en los servicios no incluye una funcion para volver al modo texto

Estos inconvenientes son por errores y bugs que son mas del **BIOS** que del OS, aunque se intente replicar por ejemplo del int 10h no funciona bien, y esto hace que funciones como el modo grafico esten buggeadas

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