#ifndef LibcStringDotH
#define LibcStringDotH
// obtiene la longitud de s
int StrLen(char* s)
{
    // variables
    int ch, len; ch = 0; len = 0;
    // obtener longitud
    while (s[ch++] != 0) len++;
    // retornarla
    return len;
}
// compara s1 con s2
int StrCmp(char* s1, char* s2)
{
    // variables
    int Len1, Len2, Diferences, ch; Diferences = 0;
    // obtener longitud
    Len1 = StrLen(s1); Len2 = StrLen(s2);
    // si no son iguales
    if (Len1 != Len2) return 1;
    // comparar
    for (ch = 0; ch < Len1; ch++) Diferences += (s1[ch] != s2[ch]);
    // retornarlo
    return Diferences;
}
// para comparalo s1 con s2 hasta cierta longitud
int StrnCmp(char* s1, char* s2, int len)
{
    int Diferences = 0;
    for (int ch = 0; ch < len; ch++)
    {
        // si alguna cadena termina antes
        if (s1[ch] == 0 || s2[ch] == 0)
        {
            if (s1[ch] != s2[ch]) Diferences++;
            break; // ya no hay más caracteres que comparar
        }

        // comparar carácter por carácter
        if (s1[ch] != s2[ch]) Diferences++;
    }
    return Diferences; // 0 si iguales, >0 si hay diferencias
}
void UIntToString(unsigned int value, char* buffer) {
    char temp[16]; // espacio temporal para dígitos
    int i = 0;

    // caso especial: valor = 0
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    // extraer dígitos en orden inverso
    while (value > 0) {
        temp[i++] = '0' + (value % 10);
        value /= 10;
    }

    // invertir el orden en buffer final
    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
}
int HexStringToInt(const char* Value) {
    // si el valor es nulo entonces 0
    if (Value[0] == 0) return 0;

    // detectar prefijo 0x
    int start = 0;
    if (Value[0] == '0' && (Value[1] == 'x' || Value[1] == 'X')) {
        start = 2;
    }

    int Len = StrLen(Value);
    int Valor = 0;
    int Cif = 1;

    // recorrer de derecha a izquierda
    for (int i = Len - 1; i >= start; i--) {
        char c = Value[i];
        int digit;

        if (c >= '0' && c <= '9') {
            digit = c - '0';
        } else if (c >= 'A' && c <= 'F') {
            digit = 10 + (c - 'A');
        } else if (c >= 'a' && c <= 'f') {
            digit = 10 + (c - 'a');
        } else {
            continue; // ignorar caracteres no hex
        }

        Valor += digit * Cif;
        Cif *= 16;
    }

    return Valor;
}
// a string
void IntToString(int value, char *buffer) {
    char temp[12];
    int i = 0;
    int isNegative = 0;

    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    if (value < 0) {
        isNegative = 1;
        value = -value;
    }

    // convertir número en orden inverso
    while (value > 0) {
        temp[i++] = (value % 10) + '0';
        value /= 10;
    }

    if (isNegative) {
        temp[i++] = '-';
    }

    // revertir en buffer final
    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }

    buffer[j] = '\0';
}
// a int
int StringToInt(char* Value)
{
    // si el valor es nulo entonces 0
    if (Value[0] == 0) return 0;

    // detectar signo negativo
    int Negative = (Value[0] == '-');

    // longitud de la cadena
    int Len = StrLen(Value);

    int Valor = 0;
    int Cif = 1;

    // recorrer de derecha a izquierda
    for (int i = Len - 1; i >= (Negative ? 1 : 0); i--)
    {
        char c = Value[i];
        if (c < '0' || c > '9') continue; // ignorar caracteres no numéricos
        Valor += (c - '0') * Cif;
        Cif *= 10;
    }

    if (Negative) Valor = -Valor;
    return Valor;
}
char CharToUpCase(char lower) {
    if (lower >= 'a' && lower <= 'z') {
        return lower - 32; // diferencia entre 'a' y 'A' en ASCII
    }
    return lower; // si no es minúscula, lo devuelve igual
}
// para separar
int StrSplit(char* str, char** buffer, char splitter)
{
    int count = 0;
    char* token = str;

    while (*str)
    {
        if (*str == splitter)
        {
            *str = '\0';             // cortar aquí
            buffer[count++] = token; // guardar inicio del token
            token = str + 1;         // siguiente token
        }
        str++;
    }

    // último token
    if (*token != '\0')
        buffer[count++] = token;

    return count;
}
char* StrUpr(char* str)
{
    char* p = str;
    while (*p) {
        *p = (char)CharToUpCase((unsigned char)*p);
        p++;
    }
    return str;
}

// devuelve un puntero al primer carácter encontrado, o NULL si no existe
char* StrChr(const char* str, int ch) {
    while (*str) {
        if (*str == (char)ch) {
            return (char*)str; // encontrado
        }
        str++;
    }
    return 0; // no encontrado
}
void IntToHexString(unsigned int value, char* buffer) {
    const char* hexDigits = "0123456789ABCDEF";
    char temp[16]; // espacio temporal para dígitos
    int i = 0;

    // caso especial: valor = 0
    if (value == 0) {
        buffer[0] = '0';
        buffer[1] = '\0';
        return;
    }

    // extraer dígitos en orden inverso
    while (value > 0) {
        temp[i++] = hexDigits[value % 16];
        value /= 16;
    }

    // invertir el orden en buffer final
    int j = 0;
    while (i > 0) {
        buffer[j++] = temp[--i];
    }
    buffer[j] = '\0';
}

char* StrTrim(char* str) {
    char* stra = str;

    while (*stra) {
        if (*stra == ' ' || *stra == '\t')
            stra++;
        else
            return stra;
    }
    return stra; // si toda la cadena eran espacios, devuelve el '\0'
}
#endif