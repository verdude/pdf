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
 * non-decoded byte stream.
 */
typedef struct {
  unsigned char* bytes;
  size_t len;
} stream_t;

/**
 * dictionary entry. key -> val pointers. key is always a name, val is any object.
 */
typedef struct {
  object_t* key;
  object_t* val;
} d_entry_t;

/**
 * list object. has number `len` of elements.
 * there is `memsize` memory allocated for the entries.
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
  stream_t* stream;
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
 * Gets an indirect.
 * c: previous character. Should be either 'R' or 'o'.
 */
indirect_t* get_indirect(state_t* state, int c);

/**
 * Create object_t pointing to name that starts at the current position.
 * Type determines whether the name is in a dictionary or not.
 * If we are reading from a dictionary, we anticipate errors like:
 * /Key/Value
 * as being two names instead of 1.
 */
object_t* get_name(state_t* state, int fail_on_error);

/**
 * Get a string representation of the object type.
 */
char* get_type_name(object_t* o);

/**
 * Create object_t pointing to string that starts at the current position.
 */
object_t* get_string(state_t* state);

/**
 * Returns the expected first char for the specified string format.
 */
int get_first_char(enum encoding enc);

/**
 * Create object_t pointing to hex string that starts at the current position.
 */
object_t* get_hex_string(state_t* state);
object_t* get_dictionary(state_t* state, int fail_on_error);
object_t* get_list(state_t* state, enum el_t el_type);
object_t* get_term(state_t* state, enum term type);

/**
 * Gets the value for the key in the dictionary object.
 */
object_t* get_val(list_t* obj, char* key);

/**
 * ["9", "0", "R"] is an indirect reference
 * object_t for references will not contain the referenced object in val.
 *
 * ["9", "0", "obj\n"] is an indirect object.
 * object_t for indirect objects will contain the referenced object in val.
 *
 * Anything else will be a number.
 */
object_t* parse_num(state_t* state);

/**
 * Creates a number object_t.
 */
object_t* create_num_obj(FILE*, long, long);

/**
 * Reads and parses a number at the current position.
 * fs points to the first char after the number.
 */
long get_num(state_t* state, int base, int fail_on_error);

/**
 * Get a long from an object_t of type Num
 */
long get_num_val(object_t* o);

/**
 * Tries to read a stream.
 * Stream is invalid if there is not a valid newline sequence
 * after len bytes followed by 'endstream'.
 */
stream_t* try_read_stream(state_t* state, long len);

/**
 * Get object_t* with val pointing to string_t.
 */
object_t* get_string_type_obj(state_t* state, enum encoding enc);

/**
 * Returns the result of calling strncmp on the object's string value
 * and the given string for the given length.
 */
int string_equals(object_t* o, char* s, int n);

/**
 * Reads a dictionary entry from fs.
 */
d_entry_t* get_entry(state_t* state);

/**
 * Load the name into a char string.
 */
char* name_str(state_t* state, object_t* name);

/**
 * Get the value for the provided entry if it exists.
 * Returns NULL otherwise.
 */
object_t* get_entry_value(object_t* dict, char* key);

/**
 * Print a dictionary.
 */
void print_dictionary(list_t* d);
void print_list(list_t* l);
void print_object(object_t* o);
void print_string(string_t* s, char open, char close);
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
