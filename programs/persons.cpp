// libreria
#include "../library/lib.hpp"

/// @brief genero de la persona
enum class Genero { 
    /// @brief el genero masculino
    Masculino, 
    /// @brief el genero femenino
    Femenino, 
    /// @brief genero no binario
    Otro
};

// Definición de una clase Persona
class Persona {
public:
    /// @brief nombre de la persona
    char* nombre;
    /// @brief edad de la persona
    int edad;
    /// @brief genero de la persona
    Genero genero;

    /// @brief genera una persona
    /// @param n nombre
    /// @param e edad
    /// @param g genero
    Persona(const char* n, int e, Genero g) : nombre((char*)n), edad(e), genero(g) {}

    /// @brief identidad de genero
    /// @return la descripcion
    char* GetIdentity() {
        // prefijo de la persona
        const char* prefix;

        // litte
        if (edad < 13) prefix = "little ";
        // si es menor de 18
        else if (edad < 18) prefix = "";
        // si es mayor que 18
        else prefix = "";

        // el genero
        switch (genero) {
            // masculino
            case Genero::Masculino: return (edad < 18) ? (char*)(prefix == "little " ? "little boy" : "boy") : (char*)"men";
            // femenino
            case Genero::Femenino: return (edad < 18) ? (char*)(prefix == "little " ? "little girl" : "girl") : (char*)"women";
            // otro
            default: return (char*)"They";
        }
    }

    /// @brief muestra la informacion de la persona
    void ShowInfo()
    {
        // nombre
        ModuLibCpp::Display::Print("Name: "); ModuLibCpp::Display::Print(nombre); ModuLibCpp::Display::Print("\n");
        // edad
        ModuLibCpp::Display::Print("Age: "); ModuLibCpp::Display::PrintInt(edad); ModuLibCpp::Display::Print("\n");
        // identidad
        ModuLibCpp::Display::Print("Identity: "); ModuLibCpp::Display::Print(GetIdentity()); ModuLibCpp::Display::Print("\n");
    }

    /// @brief alias de ShowInfo para versiones anteriores
    void presentar() { ShowInfo(); }
};

/// @brief la entrada del programa
/// @param Services los servicios
/// @return el estado
extern "C" KernelStatus ErickMain(KernelServices* Services) {
    // inicializar libreria
    InitializeLibrary(Services);
    
    // la persona
    Persona* p = new Persona("Erick", 25, Genero::Masculino);

    // presentarla
    p->presentar();

    // eliminar
    delete p;

    // retornar estado
    return KernelStatusSuccess;
}