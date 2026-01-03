![ModuKernel](./assets/Logo.svg)

| Que / Arquitectura   | i386                           | Comentario                               |
| -------------------- | ------------------------------ | ---------------------------------------- |
| Kernel               | ![bd](./assets/badgeds/ok.svg) | El kernel se puede compilar              |
| Drivers              | ![bd](./assets/badgeds/ok.svg) | Los drivers funcionane excelente         |
| Programas            | ![bd](./assets/badgeds/ok.svg) | Los programas funcionan como se esperaba |

Resumen : ![bd](./assets/badgeds/ok.svg), el kernel tiene bugs en cosas muy especificas pero funciona

Contenidos
----------------------------------------

* [Caracteristicas](#caracteristicas)
    - [Compatibilidad con unidades](#compatibilidad-con-unidades)
    - [Compatibilidad con drivers](#compatibilidad-con-drivers)
    - [Compatibilidad con sistemas de archivos](#compatibilidad-con-sistemas-de-archivos)
* [Detalles pequeños del sistema](#detalles-pequeños-del-sistema)
* [Contribuciones](#contribuciones)
    - [Ejemplos](#ejemplos)

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
| HardDisk | ![bd](./assets/badgeds/ok.svg) | Es el formato original |
| FloppyDisk | ![bd](./assets/badgeds/bugs.svg) | Probado, funciona a medias, no soporta FSLST.IFS |
| CD-ROM | ![bd](./assets/badgeds/bugs.svg)  | No garantizado |

## Compatibilidad con drivers

por ahora estos son los drivers probados y que funcionan a medias

| Driver | Estado | Probado      |
| ------------- | ------------- | ------------- |
| Teclado | ![bd](./assets/badgeds/ok.svg) | Soporte primitivo, estable |
| Pci | ![bd](./assets/badgeds/ok.svg) | Soporte plus, usa un driver `/dev/pci` |
| Mouse | ![bd](./assets/badgeds/bugs.svg) | Soporte adicional, solo con funciones externas al kernel, inestable |
| VGA (Modo texto) | ![bd](./assets/badgeds/ok.svg) | Soporte primitivo, estable |
| VGA (Modo grafico) | ![bd](./assets/badgeds/ok.svg) | Soporte como plus |
| Sonido | ![bd](./assets/badgeds/ok.svg) | Soporte primitivo, usa el buzzer para generar sonidos |

## Compatibilidad con sistemas de archivos

por ahora solo se permite FAT12 pero puede extenderlo y volverlo mas complejo añadiendo el archivo `FSLST.IFS` a la unidad para que admita rutas y carpetas virtuales

| Sistema de archivos | Estado | Probado      |
| ------------- | ------------- | ------------- |
| FAT12 | ![bd](./assets/badgeds/ok.svg) | Soporte primitivo, estable |

# Detalles pequeños del sistema

Estas son las diferencias al estandart
* La tecla **Shift** no funciona, para los caracteres especiales se requiere activar el **Caps Lock**, que es lo mismo que el shift, solo que requieres desactivarlo para volver a usar su contraparte
* Como plus, la tecla **Caps Lock** tambien cumple su funcion de volver mayusculas las teclas de las letras
* El teclado no se adapta, siempre usa el **layout ingles**
* Los simbolos y el keymap aunque usan el ingles no es el mismo, usa uno parecido pero por limitaciones require que unas teclas sean diferentes
* **No hay soporte para paginacion**
* El modo grafico esta tan bugeado que **no se puede volver al modo texto** por algun bug debido a esto, el sistema en los servicios no incluye una funcion para volver al modo texto

Estos inconvenientes son por errores y bugs que son mas del **BIOS** que del OS, aunque se intente replicar por ejemplo del int 10h no funciona bien, y esto hace que funciones como el modo grafico esten buggeadas

# Contribuciones

Puede hacer lo siguiente para contribuir
* Puede crear drivers para el sistema
* Puede hacer programas para el sistema
* Puede crear scripts utiles

## Ejemplos

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