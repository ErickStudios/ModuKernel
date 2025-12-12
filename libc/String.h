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

#endif