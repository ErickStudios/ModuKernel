/// libreria de C++ v0.1 para ModuKernel (library/lib.hpp)

/// pragma once
#pragma once
// incluir la libreria de c
#include "../library/lib.h"
// incluir cstddef
#include <cstddef>

// el tamaño
using std::size_t;

/// @brief el operator new
/// @param size tamaño
/// @return un void
void* operator new(size_t size) {
    // el pool
    void* p = gMS->AllocatePool(size);
    // si no hay retornar nullptr
    if (!p) return nullptr;

    // retornar
    return p;
}

/// @brief ek operator delete
/// @param p parametro
void operator delete(void* p) noexcept {
    // liberar memoria
    if (p) gMS->FreePool(p);
}

/// @brief el operator delete con tamaño (C++14+)
/// @param p el pool
/// @param size el tamaño
void operator delete(void* p, size_t) noexcept {
    // liberar memoria con tamaño
    if (p) gMS->FreePool(p);
}

/// @brief el operator new[] (para arreglos)
void* operator new[](std::size_t size) {
    // nuevo
    void* p = gMS->AllocatePool(size);
    // retornarlo
    return p ? p : nullptr;
}

/// @brief el operator delete[] (para arreglos)
void operator delete[](void* p) noexcept {
    // liberar memoria de array
    if (p) gMS->FreePool(p);
}


/// @brief el namespace de la libreria cpp
namespace ModuLibCpp
{
    /// @brief version de la libreria
    const double LibraryVersion = 0.1;
    /// @brief clase de i/o avanzado
    class InputOutput {
    public:
        /// @brief input a un puerto
        /// @param port el puerto
        static uint16_t InputPort(uint16_t port) { return inw(port);}
        /// @brief outpud a un puerto
        /// @param port el puerto
        /// @param value el valor a enviar
        static void OutpudPort(uint16_t port, uint16_t value) { outw(port, value); }
    };
    /// @brief clase de un string
    class String {
    public:
        /// @brief el string interno
        char* InternalString;
        /// @brief inicializa el string
        /// @param String el string
        String(const char* str) {
            size_t len = StrLen(str) + 1; // +1 para el terminador '\0'
            InternalString = (char*) gMS->AllocatePool(len);
            if (InternalString) {
                gMS->CoppyMemory(InternalString, str, len);
            }
        }
        /// @brief Destructor para liberar memoria
        ~String() { if (InternalString) gMS->FreePool(InternalString); }
        /// @brief asigna un string
        /// @param other el otro string
        /// @return algo
        String& operator=(const String& other) {
            // si no es igual al otro
            if (this != &other) {
                // si hay un string liberarlo
                if (InternalString) gMS->FreePool(InternalString);

                // longitud
                size_t len = StrLen(other.InternalString) + 1;

                // es igual a string
                InternalString = (char*) gMS->AllocatePool(len);
                // copiar memoria
                if (InternalString) gMS->CoppyMemory(InternalString, other.InternalString, len);
            }
            // retornarlo
            return *this;
        }
        /// @brief compara dos strings
        /// @param other el otro string
        /// @return true si es verdadero y false si no coinciden
        bool operator==(const String& other) const {
            // comparar strings
            return StrCmp(InternalString, other.InternalString) == 0;
        }
        /// @brief añade el string con otro
        /// @param other el otro
        /// @return un string
        String operator+(const String& other) const {
            // longitud
            size_t len = StrLen(InternalString) + StrLen(other.InternalString) + 1;
            // nuevo string
            char* newStr = (char*) gMS->AllocatePool(len);

            // si existe
            if (newStr) {
                // copiar memoria
                gMS->CoppyMemory(newStr, InternalString, StrLen(InternalString));
                // copiar memoria 2
                gMS->CoppyMemory(newStr + StrLen(InternalString), other.InternalString, StrLen(other.InternalString) + 1);
            }
            // el string
            return String(newStr);
        }
    };
    /// @brief clase del display
    class Display {
    public:
        /// @brief imprime un texto en la consola
        /// @param msg el texto
        static void Print(char* msg) { gDS->printg(msg); }
        /// @brief imprime un entero en la consola
        /// @param value el entero
        static void PrintInt(int value) {
            // el buffer del valor
            char buffer[32];
            // convertirlo a string
            IntToString(value, buffer);
            // imprimirlo
            gDS->printg(buffer);
        }
    };    
    /// @brief una caja
    /// @tparam T el tipo
    template <typename T>
    class Box {
    private:
        /// @brief contenido de la caja
        T* Content;
    public:
        /// @brief crea la caja
        Box() : Content(nullptr) {}
        /// @brief permite copiar el contenido de una caja a otra
        /// @param other la otra caja
        Box(Box&& other) noexcept : Content(other.Content) { other.Content = nullptr; }
        /// @brief elimina la caja
        ~Box() { delete Content; }
        /// @brief pone contenido en una caja
        /// @param value el valor
        void Put(const T& value) { delete Content; Content = new T(value);}
        /// @brief abre la caja como contenido simple,
        /// @brief por favor cuide la caja manualmente verificando si su contenido esta presente
        /// @return el contenido
        T Open() const { return *Content;}
        /// @brief funcion que ayuda a que Open no haga un desastre
        /// @return si no es nullptr
        bool HasValue() const { return Content != nullptr; }
        /// @brief abre la caja como un array
        /// @return el contenido
        T* OpenArray() const { return Content;}
        /// @brief asignacion de la caja
        /// @param other la otra caja
        /// @return la caja
        Box& operator=(Box&& other) noexcept { if (this != &other) { delete Content; Content = other.Content; other.Content = nullptr;} return *this; }
    };
    /// @brief un par
    /// @tparam A el tipo del a
    /// @tparam B el tipo del b
    template <typename A, typename B>
    struct Pair {
        /// @brief el primero
        A first;
        /// @brief el segundo
        B second;
    };
    /// @brief el tipo de tercero
    /// @tparam A el tipo del a
    /// @tparam B el tipo del b
    /// @tparam C el tipo del c
    template <typename A, typename B, typename C>
    struct ThridyPair {
        /// @brief el primero
        A first;
        /// @brief el segundo
        B second;
        /// @brief el tercero
        C third;
    };
    /// @brief alias para los de pythn
    using str = String;
    /// @brief alias para el string
    using string = str;
}
