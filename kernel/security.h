#include <stdint.h>
// anillo actual
extern uint32_t InternalRingLevel;

/** tipo de proteccion */
typedef enum _ProtectionType {
    /** es libre para que cualquiera lo use */
    ProtectionTypeFreeToUse,
    /** solo el kernel puede modificar eso */
    ProtectionTypeOnlyKernel
} ProtectionType;

/** zona protegida */
typedef struct _ProtectedZone {
    /** tipo de proteccion de la zona */
    ProtectionType TypeOfProtection;
    /** direccion donde empieza la zona */
    uint32_t StartOfProtectedZone;
    /** tamaño de la zona que se protege */
    uint32_t SizeOfProtectedZone;
} ProtectedZone;

/** zonas protegidas del kernel */
ProtectedZone* ProtectedZones;
/* Numero de zonas protegidas */
int ProtectedZonesCount;

/** verifica si una dirección está en una zona protegida */
uint8_t IsAddressProtected(uint32_t addr) {
    for (int i = 0; i < ProtectedZonesCount; i++) {
        uint32_t start = ProtectedZones[i].StartOfProtectedZone;
        uint32_t end   = start + ProtectedZones[i].SizeOfProtectedZone;

        if (addr >= start && addr < end) {
            if (ProtectedZones[i].TypeOfProtection == ProtectionTypeOnlyKernel) {
                // si estamos en el entorno C
                if (InternalRingLevel < 2) return 1; // violación: dirección protegida
            }
        }
    }
    return 0; // libre
}

/** prototipo que añade zonas protegidas */
void AddProtectedZone(ProtectionType Type, uint32_t Start, uint32_t Size);