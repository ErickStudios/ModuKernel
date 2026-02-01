#include "../library/lib.hpp"

extern "C" KernelStatus ErickMain(KernelServices* Services) {
    InitializeLibrary(Services);

    ModuLibCpp::String s1("Hola");
    ModuLibCpp::String s2(" Erick");

    ModuLibCpp::String saludo = s1 + s2; // usa operator+
    if (saludo == s1) {
        ModuLibCpp::Display::Print("Son iguales\n");
    } else {
        ModuLibCpp::Display::Print(saludo.InternalString);
    }

    return KernelStatusSuccess;
}