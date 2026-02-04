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
    return p;
}

/// @brief el operator delete[] (para arreglos)
void operator delete[](void* p) noexcept {
    // liberar memoria de array
    if (p) gMS->FreePool(p);
}

/// @brief el operator delete[] con tamaño (C++14+)
void operator delete[](void* p, std::size_t) noexcept {
    if (p) gMS->FreePool(p);
}

/// @brief el namespace de la libreria cpp
namespace ModuLibCpp
{
    /// @brief version de la libreria
    const double LibraryVersion = 0.1;
    /// @brief el array
    /// @tparam T el tipo
    template<typename T>
    class Array {
        T* data;
        int capacity;
        int count;

        void resize(int newCapacity) {
            T* newData = new T[newCapacity]; // solo copia punteros
            for (int i = 0; i < count; i++) {
                newData[i] = data[i];
            }
            delete[] data;
            data = newData;
            capacity = newCapacity;
        }

    public:
        Array(int initialCapacity = 4) {
            capacity = initialCapacity;
            count = 0;
            data = new T[capacity]; // T = AssemblyLabel*
        }

        ~Array() {
            delete[] data; // libera solo el arreglo de punteros
        }

        void push(T value) {
            if (count >= capacity) resize(capacity * 2);
            data[count++] = value;
        }

        int size() const { return count; }

        T& operator[](int index) { return data[index]; }

        T* begin() { return data; }
        T* end() { return data + count; }
    };    
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
        /// @brief compara dos strings
        /// @param other el otro string
        /// @return true si es verdadero y false si no coinciden
        bool operator==(char* other) const {
            // comparar strings
            return StrCmp(InternalString, other) == 0;
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
        /// @brief operador << como alias de AddChar
        /// @param c el carácter a añadir
        /// @return referencia al propio String
        String& operator<<(char c) {
            AddChar(c);
            return *this;
        }
        /// @brief operador << como alias de juntar string
        /// @param c el string a añadir
        /// @return referencia al propio String
        String& operator<<(String& other) {
            int len = StrLen(other.InternalString);

            for (size_t i = 0; i < len; i++) AddChar(other.InternalString[i]);
            
            return *this;
        }
        /// @brief operador << como alias de juntar string
        /// @param c el string a añadir
        /// @return referencia al propio String
        String& operator<<(const String& other) {
            int len = StrLen(other.InternalString);

            for (size_t i = 0; i < len; i++) AddChar(other.InternalString[i]);
            
            return *this;
        }
        /// @brief devuelve puntero al inicio
        char* begin() { return InternalString; }
        /// @brief devuelve puntero al final (justo antes del nullchar)
        char* end() { return InternalString + StrLen(InternalString); }
        /// @brief versión const para foreach en const String
        const char* begin() const { return InternalString; }
        const char* end() const { return InternalString + StrLen(InternalString); }
        /// @brief añade un carácter al final del string
        /// @param c el carácter a añadir
        void AddChar(char c) {
            // longitud actual
            size_t oldLen = StrLen(InternalString);

            // nuevo tamaño (+1 para el nuevo char, +1 para el terminador)
            size_t newLen = oldLen + 2;

            // reservar nuevo pool
            char* newStr = (char*) gMS->AllocatePool(newLen);

            if (newStr) {
                // copiar el contenido anterior
                gMS->CoppyMemory(newStr, InternalString, oldLen);

                // añadir el nuevo carácter
                newStr[oldLen] = c;

                // añadir el terminador
                newStr[oldLen + 1] = '\0';

                // liberar el viejo string
                if (InternalString) gMS->FreePool(InternalString);

                // asignar el nuevo
                InternalString = newStr;
            }
        }
        /// @brief limpia el string
        void ClearString()
        {
            gMS->FreePool((void*)InternalString);
            InternalString = (char*)gMS->AllocatePool(1);
            InternalString[0] = '\0';
        }
        /// @brief asigna un nuevo contenido al string
        /// @param str el nuevo contenido (char*)
        void SetString(const char* str) {
            // liberar el contenido actual
            if (InternalString) {
                gMS->FreePool(InternalString);
            }

            // calcular longitud del nuevo string (+1 para '\0')
            size_t len = StrLen(str) + 1;

            // reservar nuevo pool
            InternalString = (char*) gMS->AllocatePool(len);

            // copiar memoria
            if (InternalString) {
                gMS->CoppyMemory(InternalString, str, len);
            }
        }
        /// @brief devuelve true si empieza con un carácter
        bool StartsWith(char c) const {
            return InternalString[0] == c;
        }
        /// @brief devuelve true si termina con un carácter
        bool EndsWith(char c) const {
            size_t len = StrLen(InternalString);
            if (len == 0) return false;
            return InternalString[len - 1] == c;
        }
        /// @brief elimina el primer carácter
        void RemoveFirstChar() {
            size_t len = StrLen(InternalString);
            if (len == 0) return;
            // mover todo una posición hacia atrás
            for (size_t i = 1; i <= len; i++) {
                InternalString[i - 1] = InternalString[i];
            }
        }
        /// @brief elimina el último carácter
        void RemoveLastChar() {
            size_t len = StrLen(InternalString);
            if (len == 0) return;
            InternalString[len - 1] = '\0';
        }
        /// @brief elimina espacios al inicio y al final
        void Trim() {
            // quitar espacios al inicio
            while (InternalString[0] == ' ' || InternalString[0] == '\n' || InternalString[0] == '\t') {
                RemoveFirstChar();
            }
            // quitar espacios al final
            while (EndsWith(' ') || EndsWith('\n') || EndsWith('\t')) {
                RemoveLastChar();
            }
        }
        /// @brief verifica si todos los caracteres son dígitos
        bool IsDigitString() const {
            size_t len = StrLen(InternalString);
            if (len == 0) return false;
            for (size_t i = 0; i < len; i++) {
                if (InternalString[i] < '0' || InternalString[i] > '9') return false;
            }
            return true;
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
