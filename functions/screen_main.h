/* escritura y fuente */
char* text_attr;

/* columna seleccionada */
int* row_selected = 0;
/* linea seleccionada */
int* line_selected = 0;

/* limpia la pantalla */
void InternalClearScreen();
/* imprime algo requiriendo una linea */
void InternalPrintg(char *message, unsigned int line);
/* imprime algo normalmente */
void InternalPrintgNonLine(char *message);
/* ajustar posicion del cursor */
void InternalCursorPos(int x, int y);
/* para ajustar */
void InternalSetActualDisplayService(DisplayServices* Serv);
/* para ajustar atributos */
void InternalSetAttriubtes(char bg, char fg);