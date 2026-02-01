// libreria
#include "../library/lib.hpp"

/// @brief la entrada del programa
/// @param Services los servicios
/// @return el estado
extern "C" KernelStatus ErickMain(KernelServices* Services)
{
    // inicializar libreria
    InitializeLibrary(Services);

    // crear una caja
    ModuLibCpp::Box<int> MyBox;
    // poner 42 en la caja
    MyBox.Put(42);

    // imprimir el contenido
    ModuLibCpp::Display::PrintInt(MyBox.Open());

    return KernelStatusSuccess;
}