#ifndef object_h
#define object_h

/**
 * boo -> int
 * num -> long
 * str -> char
 * hstr -> char
 * name -> name_t
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
 * Dictionry object. Has entries and number (len) of entries.
 */
typedef struct {
  d_entry_t** entries;
  int len;
} dict_t;

/**
 * Name object.
 * len: length of the name in bytes as represented in single chars.
 * memsize: allocated size of val object in bytes.
 */
typedef struct {
  unsigned char* str;
  int memsize;
  int len;
} name_t;

/**
 * Create object_t pointing to name that starts at the current position.
 */
object_t* get_name(FILE* fs, int fail_on_error);

object_t* get_string(FILE* fs, int fail_on_error);

object_t* get_hex_string(FILE* fs, int fail_on_error);

object_t* get_dictionary(FILE* fs, int fail_on_error);

/**
 * Load the name into a char string.
 */
char* name_str(FILE* fs, object_t* name);

void print_dictionary(dict_t* d);

#endif // object_h
