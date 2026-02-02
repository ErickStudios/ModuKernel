/** funcion de matcheo de grupos de regex */
#include "../../library/lib.h"

/// @brief el resultado del regex
struct RegexResult {
    bool matched;
    const char* groups[8];   
    int groupLens[8];
    int groupCount;
};
/// @brief el regex matcheador
/// @param pattern el pattern
/// @param text el texto
/// @return true si matchean y false si no
bool regex_match_groups(const char* pattern, const char* text, RegexResult* result) {
    result->matched = false;
    result->groupCount = 0;

    const char* groupStart = nullptr;

    while (*pattern && *text) {
        if (*pattern == '(') {
            groupStart = text; // inicio del grupo
            pattern++;
            continue;
        }
        if (*pattern == ')') {
            if (groupStart) {
                result->groups[result->groupCount] = groupStart;
                result->groupLens[result->groupCount] = text - groupStart;
                result->groupCount++;
                groupStart = nullptr;
            }
            pattern++;
            continue;
        }

        if (*(pattern+1) == '*') {
            // caso con '*'
            if (regex_match_groups(pattern+2, text, result)) return true;
            if ((*text != '\0') && (*pattern == '.' || *pattern == *text)) {
                return regex_match_groups(pattern, text+1, result);
            }
            return false;
        } else {
            // caso normal
            if ((*text != '\0') && (*pattern == '.' || *pattern == *text)) {
                pattern++;
                text++;
            } else {
                return false;
            }
        }
    }

    if (*pattern == '\0' && *text == '\0') {
        result->matched = true;
    }
    return result->matched;
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

    RegexResult* Result = (RegexResult*)Services->Memory->AllocatePool(sizeof(RegexResult));

    bool Match = regex_match_groups(pattern, stringToComp, Result);

    Services->Packages->Launch(_KernelPackageType::KPackageFromDriver, (uint32_t)Result);

    // retornar
    return (Match ? KernelStatusSuccess : KernelStatusDisaster);
}