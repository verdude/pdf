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
  boo,
  num,
  str,
  hstr,
  name,
  arr,
  dict,
  null,
  stream,
  ind
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
  int type;
  long offset;
  int len;
  void* val;
} object_t;

object_t* get_name(FILE*, int);

object_t* get_string(FILE*, int);

object_t* get_hex_string(FILE*, int);

#endif // object_h
