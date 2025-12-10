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
#endif