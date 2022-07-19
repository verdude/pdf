#ifndef next_h
#define next_h

#define FAIL 1
#define IGNORE 0

/**
 * Returns the next character.
 * If eof_fail is non-zero, will exit on EOF; otherwise, EOF is returned.
 */
int get_char(FILE* fs, int eof_fail);

/**
 * ungetc wrapper.
 */
void unget_char(FILE* fs, int c, int fail_on_error);

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

/**
 * Adds chars to a heap c-string which is allocated and initialized with a
 * max lenght of len. fn(char) must return a non-zero value for the char
 * to be added to the c-string.
 * Returns char* pointer.
 * There will be at least one 0 at the end of the string.
 */
unsigned char* consume_chars(FILE* fs, int (*fn)(), int len);

/**
 * ftell wrapper.
 * Exits on failure.
 */
long get_pos(FILE* fs);

/**
 * fseek wrapper. Exit on fail if specified.
 */
int seek(FILE* fs, long offset, int whence, int fail_on_error);

/**
 * Finds a sequence searching backwards through the stream.
 * sequence is the length of len.
 * Max 10 char sequence for now.
 * Updates the fs to point at the start of the sequence and
 * returns 0 if not found and 1 if found.
 */
int find_backwards(FILE* fs, unsigned char* sequence, int len);

/**
 * Does what you think it does.
 */
void cexit(FILE* fs, int code);

#endif // next_h
