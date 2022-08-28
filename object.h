#ifndef object_h
#define object_h

#include <stdio.h>

/**
 * boo -> int
 * num -> long
 * str -> string_t
 * hstr -> string_t
 * name -> string_t
 * arr -> list_t*
 * dict -> list_t*
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
 * Corresponds to a static string value:
 * NullTerm -> null
 * TrueTerm -> true
 * FalseTerm -> false
 */
enum term {
  NullTerm,
  TrueTerm,
  FalseTerm
};

/**
 * List element Type
 */
enum el_t {
  DictionaryEntry,
  Object
};

/**
 * String encoding/format
 */
enum encoding {
  HexString,
  LiteralString,
  NameString
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
 * List object. Has number `len` of elements.
 * There is `memsize` memory allocated for the entries.
 */
typedef struct {
  void** el;
  int len;
  int memsize;
  enum el_t el_type;
} list_t;

/**
 * Indirect object
 */
typedef struct {
  long obj_num;
  long gen_num;
  object_t* obj;
} indirect_t;

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
  enum encoding enc;
} string_t;

/**
 * Function pointer types to read a dictionary entry or object.
 * read_element for casting.
 */
typedef d_entry_t* (*read_dict_entry)(FILE*);
typedef object_t* (*read_object)(FILE*);
typedef void* (*read_element)(FILE*);

/**
 * Create object_t pointing to name that starts at the current position.
 * Type determines whether the name is in a dictionary or not.
 * If we are reading from a dictionary, we anticipate errors like:
 * /Key/Value
 * as being two names instead of 1.
 */
object_t* get_name(FILE* fs, int fail_on_error, enum el_t type);

/**
 * Create object_t pointing to string that starts at the current position.
 */
object_t* get_string(FILE* fs, enum encoding enc);

/**
 * Returns the expected first char for the specified string format.
 */
int get_first_char(enum encoding enc);

/**
 * Create object_t pointing to hex string that starts at the current position.
 */
object_t* get_hex_string(FILE* fs);
object_t* get_dictionary(FILE* fs, int fail_on_error);
object_t* get_list(FILE* fs, enum el_t el_type);
object_t* get_term(FILE* fs, enum term type);

/**
 * ["9", "0", "R"] is an indirect reference
 * object_t for references will not contain the referenced object in val.
 *
 * ["9", "0", "obj\n"] is an indirect object.
 * object_t for indirect objects will contain the referenced object in val.
 *
 * Anything else will be a number.
 */
object_t* parse_num(FILE* fs);

/**
 * Reads and parses a number at the current position.
 * fs points to the first char after the number.
 */
long get_num(FILE* fs, int base);

/**
 * Get object_t* with val pointing to string_t.
 */
object_t* get_string_type_obj(FILE* fs, enum encoding enc);

/**
 * Reads a dictionary entry from fs.
 */
d_entry_t* get_entry(FILE* fs);

/**
 * Load the name into a char string.
 */
char* name_str(FILE* fs, object_t* name);

/**
 * Print a dictionary.
 */
void print_dictionary(list_t* d);
void print_list(list_t* l);
void print_object(object_t* o);
void print_string(string_t* s);
void print_d_entry(d_entry_t* e);
void print_indirect(indirect_t* i);

/**
 * Add a byte to the string
 */
int add_byte(unsigned char c, string_t* st);

/**
 * Free an object_t*
 */
void free_object_t(object_t* o);

#endif // object_h
