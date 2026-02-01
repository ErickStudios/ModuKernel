// incluye el encabezado
#include "../library/lib.h"

class Greeter {
public:
    void sayHello(DisplayServices* ds) {
        ds->printg("Hola desde una clase C++!\n");
    }
};

extern "C" KernelStatus ErickMain(KernelServices* Services) {
    Greeter g;
    g.sayHello(Services->Display);
    return KernelStatusSuccess;
}