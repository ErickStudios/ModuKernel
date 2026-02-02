#include "../../library/lib.hpp"
/// @brief registros generales de i386
enum class Reg32 : uint8_t { EAX,ECX,EDX,EBX,ESP,EBP,ESI,EDI };
/// @brief las instrucciones disponibles
enum class Instructions : uint8_t { MovRegister = 0x89};
/// @brief codifica el registro
/// @param reg el registro
/// @return el registro
Reg32 CodifiqueRegister(ModuLibCpp::String& reg);
/// @brief codifica una instruccion
/// @param instruction la instruccion
/// @return la instruccion
uint8_t* CodifiqueInstruction(ModuLibCpp::String& instruction, int* len);