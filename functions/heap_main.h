/* el inicio del heap */
char _heap_start;
/* el fin del heap */
char _heap_end;
/* puntero al heap */
static char* heap_ptr = &_heap_start;
/* para ver cuanto queda */
unsigned int InternalGetFreeHeapSpace();