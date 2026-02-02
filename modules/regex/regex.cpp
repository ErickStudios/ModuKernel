#include "../../library/lib.h"

/// @brief el regex matcheador
/// @param pattern el pattern
/// @param text el texto
/// @return true si matchean y false si no
bool regex_match(const char* pattern, const char* text) {
    // si el patrón está vacío, el texto también debe estar vacío
    if (*pattern == '\0') return *text == '\0';

    // caso: siguiente char es '*'
    if (*(pattern+1) == '*') {
        // opción 1: ignorar el "char*" y avanzar en el patrón
        if (regex_match(pattern+2, text)) return true;
        // opción 2: consumir un char del texto si coincide
        if ((*text != '\0') && (*pattern == '.' || *pattern == *text)) {
            return regex_match(pattern, text+1);
        }
        return false;
    } else {
        // caso normal: un solo char
        if ((*text != '\0') && (*pattern == '.' || *pattern == *text)) {
            return regex_match(pattern+1, text+1);
        }
        return false;
    }
}

/// @brief punto de entrada del driver
/// @param Services los servicios
/// @return algo
extern "C" KernelStatus ErickMain(KernelServices* Services)
{
    // el pattern
    KernelPackage Pattern = Services->Packages->Catch();
    // el string a comparar
    KernelPackage StringToComp = Services->Packages->Catch();

    const char* pattern = (const char*)Pattern.Data;
    const char* stringToComp = (const char*)StringToComp.Data;

    // retornar
    return (regex_match(pattern, stringToComp) ? KernelStatusSuccess : KernelStatusDisaster);
}