#ifndef next_h
#define next_h

#define FAIL 1
#define IGNORE 0

#include "object.h"
#include "xref.h"
#include "trailer.h"

/**
 * Returns the next character.
 * If eof_fail is non-zero, will exit on EOF; otherwise, EOF is returned.
 */
int get_char(state_t* state, int eof_fail);

/**
 * ungetc wrapper.
 */
void unget_char(state_t* state, int c, int fail_on_error);

/**
 * unget wrapper for list of unsigned char cast to int.
 * TODO: remove?
 */
void unget_chars(state_t* state, unsigned char* c, int len);

/**
 * Finds the next symbol.
 * Returns NULL unless an object is successfully read.
 */
object_t* next_sym(state_t* state);

/**
 * Returns pointer to 0 initialized mem (by calloc) of size len bytes.
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
char* consume_chars(state_t* state, int (*fn)(int), int len);
void consume_chars_stack(state_t* state, int (*fn)(int), char* chars, int len);

/**
 * Moves the fs position to the first byte after
 * the given string.
 * Returns 0 if there is no match at the current position
 * and resets to the starting position.
 */
int skip_string(state_t* state, char* s, long pos);

/**
 * ftell wrapper.
 * Exits on failure.
 */
long get_pos(state_t* state);

/**
 * fseek wrapper.
 */
int seek(state_t* state, long offset, int whence);

/**
 * Checks for a match at the current position.
 * returns 1 if a match is found, returns 0 otherwise.
 * check_for_match_seek_back returns to the current position
 * after checking for a match if one was not found.
 */
size_t check_for_match(state_t* state, char* s);
size_t check_for_match_seek_back(state_t* state, char* s);

/**
 * Finds a sequence searching backwards through the stream.
 * sequence is the length of len.
 * Max 15 char sequence for now.
 * fs will point to the char after the sequence.
 * returns 0 if not found and 1 if found.
 * O(n) where n is the length of the pdf in bytes.
 */
int find_backwards(state_t* state, char* sequence, int len);

/**
 * Does what you think it does.
 */
void cexit(state_t* state, int code);

/**
 * Skips whitespace.
 */
void consume_whitespace(state_t* state);

int is_not_space(int c);

/**
 * Reads n bytes.
 */
unsigned char* fs_read(state_t* state, size_t n);

/**
 * strtol wrapper
 */
long estrtol(char* s, char** endptr, int base);

/**
 * The PDF state.
 */
typedef struct {
  state_t* state;
  xref_t* xref;
  trailer_t* trailer;
} state_t;

#endif // next_h
