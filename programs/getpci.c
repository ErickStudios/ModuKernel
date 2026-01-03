/* programa para obtener informacion de un pci */

#include "../library/lib.h"

KernelStatus ErickMain(KernelServices* Services)
{
    // archivo de driver
    FatFile PciDriver = Services->File->OpenFile("/dev/pci");

    // el driver
    ObjectAny PciDriverBin;
    // el tamaño
    Entero PciDriverBinSize;

    // obtenerlo
    KernelStatus OpenFilePci = Services->File->GetFile(PciDriver, &PciDriverBin, &PciDriverBinSize);

    // si hay error entonces retornarlo
    if (OpenFilePci) return OpenFilePci;

    // la configuracion
    uint32_t ValuePciConfig;

    // variables de pci
    uint8_t PciBus = (uint8_t)(HexStringToInt((char*)Services->Misc->Paramaters[1 + 2]));
    uint8_t Device = (uint8_t)(HexStringToInt((char*)Services->Misc->Paramaters[2 + 2]));
    uint8_t Function = (uint8_t)(HexStringToInt((char*)Services->Misc->Paramaters[3 + 2]));
    uint8_t Offset = (uint8_t)(HexStringToInt((char*)Services->Misc->Paramaters[4 + 2]));
    
    // parametros
    Services->Packages->Launch(KPackageToDriver, 0);                // accion: obtener
    Services->Packages->Launch(KPackageToDriver, &ValuePciConfig);  // donde lo retorna
    Services->Packages->Launch(KPackageToDriver, PciBus);           // el bus
    Services->Packages->Launch(KPackageToDriver, Device);           // el dispositivo
    Services->Packages->Launch(KPackageToDriver, Function);         // la funcion
    Services->Packages->Launch(KPackageToDriver, Offset);           // el offset

    // ejecutar el driver
    Services->Misc->RunBinary(PciDriverBin, PciDriverBinSize, Services);

    // obtenerlo
    char StringPciInfoGetted[9];                            // la variable
    IntToHexString(ValuePciConfig, StringPciInfoGetted);    // obtenerlo

    // imprimirlo
    Services->Display->printg(StringPciInfoGetted);

    return KernelStatusSuccess;
}