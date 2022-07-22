#ifndef object_h
#define object_h

#include <stdio.h>

/**
 * boo -> int
 * num -> long
 * str -> string_t
 * hstr -> string_t
 * name -> string_t
 * arr -> object_t*
 * dict -> dict_t*
 * null -> NULL
 * stream -> tbd
 * ind -> tbd
 */
enum o_type {
  Boo,
  Num,
  Str,
  Hstr,
  Name,
  Arr,
  Dict,
  Null,
  Stream,
  Ind
};

/**
 * type: Object type (o_type)
 * offset: Byte offset from beginning of file stream
 * len: length of object in the stream in bytes
 * val: pointer to the parsed value of the object. See o_type (above) for pointer types.
 */
typedef struct {
  enum o_type type;
  long offset;
  int len;
  void* val;
} object_t;

/**
 * Dictionary entry. key -> val pointers. Key is always a Name, val is any object.
 */
typedef struct {
  object_t* key;
  object_t* val;
} d_entry_t;

/**
 * Dictionary object. Has entries and number (len) of entries.
 * There is memsize memory allocated for the entries.
 */
typedef struct {
  d_entry_t** entries;
  int len;
  int memsize;
} dict_t;

/**
 * stringish object.
 *
 * used by:
 *  - string
 *  - hex string
 *  - name
 *
 * len: length of the text in bytes as represented in single chars.
 * memsize: allocated size of str object in bytes.
 */
typedef struct {
  char* str;
  int memsize;
  int len;
} string_t;

/**
 * Create object_t pointing to name that starts at the current position.
 */
object_t* get_name(FILE* fs, int fail_on_error);

/**
 * Create object_t pointing to string that starts at the current position.
 */
object_t* get_string(FILE* fs, int fail_on_error);

/**
 * Create object_t pointing to hex string that starts at the current position.
 */
object_t* get_hex_string(FILE* fs, int fail_on_error);

/**
 * Get object_t* with val pointing to string_t.
 */
object_t* get_string_type_obj(FILE* fs, unsigned char first_char, int fail_on_error);

object_t* get_dictionary(FILE* fs, int fail_on_error);

object_t* get_number(FILE* fs, int fail_on_error);

/**
 * Load the name into a char string.
 */
char* name_str(FILE* fs, object_t* name);

/**
 * Print a dictionary.
 */
void print_dictionary(dict_t* d);

/**
 * Add a byte to the string
 */
int add_byte(unsigned char c, string_t* st);

/**
 * Free an object_t*
 */
int free_object_t(object_t* o);

#endif // object_h
