char* AllocateStringArray(char* text)
{
    size_t len = StrLen(text) + 1; // incluye el '\0'

    char* retval = GlobalServices->Memory->AllocatePool(len);
    if (!retval) {
        return 0; // manejo de error si no hay memoria
    }

    GlobalServices->Memory->CoppyMemory(retval, text, len);

    return retval;
}