#ifndef next_h
#define next_h

/**
 * Returns the next character.
 * If eof_fail is non-zero, will exit on EOF; otherwise, EOF is returned.
 */
int get_char(FILE* fs, int eof_fail);

int unget_char(FILE* fs, int c, int fail_on_error);

/**
 * Finds the next symbol.
 */
char* next_sym(FILE* fs);

/**
 * Gets the next non space block of chars.
 */
char* get_word(FILE* fs);

/**
 * Returns pointer to initialized mem of size len bytes.
 * Exits on failure.
 */
void* allocate(int len);

char* consume_chars(FILE* fs, int (*fn)(), int len);

#endif // next_h
