#ifndef next_h
#define next_h

#define FAIL 1
#define IGNORE 0

#include "pdf.h"
#include "object.h"
#include "xref.h"
#include "trailer.h"

/**
 * Returns the next character.
 * If eof_fail is non-zero, will exit on EOF; otherwise, EOF is returned.
 */
int get_char(pdf_t* pdf, int eof_fail);

/**
 * ungetc wrapper.
 */
void unget_char(pdf_t* pdf, int c, int fail_on_error);

/**
 * unget wrapper for list of unsigned char cast to int.
 * TODO: remove?
 */
void unget_chars(pdf_t* pdf, unsigned char* c, int len);

/**
 * Finds the next symbol.
 * Returns NULL unless an object is successfully read.
 */
object_t* next_sym(pdf_t* pdf);

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
char* consume_chars(pdf_t* pdf, int (*fn)(int), int len);
void consume_chars_stack(pdf_t* pdf, int (*fn)(int), char* chars, int len);

/**
 * Moves the fs position to the first byte after
 * the given string.
 * Returns 0 if there is no match at the current position
 * and resets to the starting position.
 */
int skip_string(pdf_t* pdf, char* s, long pos);

/**
 * ftell wrapper.
 * Exits on failure.
 */
long get_pos(pdf_t* pdf);

/**
 * fseek wrapper.
 */
int seek(pdf_t* pdf, long offset, int whence);

/**
 * Checks for a match at the current position.
 * returns 1 if a match is found, returns 0 otherwise.
 * check_for_match_seek_back returns to the current position
 * after checking for a match if one was not found.
 */
size_t check_for_match(pdf_t* pdf, char* s);
size_t check_for_match_seek_back(pdf_t* pdf, char* s);

/**
 * Finds a sequence searching backwards through the stream.
 * sequence is the length of len.
 * Max 15 char sequence for now.
 * fs will point to the char after the sequence.
 * returns 0 if not found and 1 if found.
 * O(n) where n is the length of the pdf in bytes.
 */
int find_backwards(pdf_t* pdf, char* sequence, int len);

/**
 * Does what you think it does.
 */
void scexit(pdf_t* pdf, int code);

/**
 * Skips whitespace.
 */
void consume_whitespace(pdf_t* pdf);

int is_not_space(int c);

/**
 * Reads n bytes.
 */
unsigned char* fs_read(pdf_t* pdf, size_t n);

/**
 * strtol wrapper
 */
long estrtol(char* s, char** endptr, int base);

#endif // next_h
