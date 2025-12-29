static uint8_t inb(uint16_t port) {
    uint8_t ret;
    __asm__ __volatile__("inb %1, %0" : "=a"(ret) : "dN"(port));
    return ret;
}

static void outb(uint16_t port, uint8_t val) {
    __asm__ __volatile__("outb %0, %1" : : "a"(val), "dN"(port));
}

void InternalSendCharToSerialFy(char ch) {
    // esperar transmisor listo
    while ((inb(0x3F8 + 5) & 0x20) == 0);

    // enviar carácter
    outb(0x3F8, (uint8_t)ch);
}

void InternalDebug(char* str)
{
	char* String = str;

	while (*String) {
		if (*String == '\n')
			InternalSendCharToSerialFy('\r');
		InternalSendCharToSerialFy(*(String++));
	}
}

void SystemInternalMessage(char* msg)
{
	InternalDebug("\x1B[94m[  \x1B[92mSysInternal  \x1B[94m]     \x1B[0m");
	InternalDebug(msg);
	InternalDebug("\n");
}