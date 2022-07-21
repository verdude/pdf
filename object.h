#ifndef object_h
#define object_h

/**
 * boo -> int
 * num -> long
 * str -> char
 * hstr -> char
 * name -> char
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

// TODO: update types
// perhaps array of pairs
typedef struct {
  int key;
  int val;
} dict_t;

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
 * Create object_t pointing to name that starts at the current position.
 */
object_t* get_name(FILE* fs, int fail_on_error);

object_t* get_string(FILE*, int);

object_t* get_hex_string(FILE*, int);

/**
 * Load the name into a char string.
 */
char* name_str(FILE* fs, object_t* name);

#endif // object_h
