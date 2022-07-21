#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "next.h"
#include "object.h"

static unsigned char get_hex_char(FILE* fs) {
  int c1 = get_char(fs, FAIL);
  int c2 = get_char(fs, FAIL);
  char hex_str[] = { (unsigned char) c1, (unsigned char) c2 };

  // Validate hex_str by converting to long
  long abnormal = strtol(hex_str, NULL, 16);
  if (abnormal == LONG_MAX) {
    fprintf(stderr,
        "Invalid hex character [0x%c%c] at: %li\n",
        c1, c2, get_pos(fs));
    perror("get_name_char_len");
    return 0;
  }

  printf("hex: %c%c\n", c1, c2);
  return (unsigned char) abnormal;
}

/**
 * Adds a char to ptr+len, reallocating ptr if len >= memsize.
 * returns NULL in case of failure.
 */
static void* add_byte(unsigned char c, void* ptr, int* memsize, int len) {
  int new_size = *memsize;
  unsigned char* new_ptr = (unsigned char*) ptr;
  printf("adding char: %c to ptr: %p size %i len %i\n", c, ptr, *memsize, len);
  while (new_size <= len + 1) {
    new_size += *memsize;
  }

  if (new_size < 0) {
    fprintf(stderr, "Overflow computing new size: ptr:"
        " %p memsize: %i strlen: %i\n", ptr, *memsize, len);
    return NULL;
  }

  if (new_size > *memsize) {
    new_ptr = realloc(ptr, new_size);
    if (new_ptr == NULL) {
      perror("realloc in add_byte");
      return NULL;
    }
    memset(new_ptr + len, 0, new_size - len);
  }

  *memsize = new_size;
  new_ptr[len] = c;
  return new_ptr;
}

/**
 * 1 for a regular char,
 * 3 for a hex char, encoded as: #0A in the pdf.
 * 0 for invalid chars. May be the start of another symbol.
 *
 * returns positive int if a char was read successfully.
 * Int signifies the number of bytes that it takes to represent
 * the character in the pdf.
 * returns 0 when done reading.
 */
static int add_name_char(FILE* fs, int c, name_t* name) {
  if (c > 0x21 && c < 0x7e) {
    name->str = add_byte(c, name->str, &name->memsize, name->len);
    if (name->str == NULL) {
      cexit(fs, 1);
    }
    name->len++;
    return 1;
  }

  if (c == '#') {
    unsigned char h_char = get_hex_char(fs);
    name->str = add_byte(h_char, name->str, &name->memsize, name->len);
    if (name->str == NULL) {
      cexit(fs, 1);
    }
    name->len++;
    return 3;
  } else {
    fprintf(stderr, "Invalid char in name: %#4x\n", c);
    fprintf(stderr, "File Offset: %li\n", get_pos(fs));
    return 0;
  }
}

object_t* get_name(FILE* fs, int fail_on_error) {
  consume_whitespace(fs);
  int c = get_char(fs, FAIL);
  if (c != '/' && fail_on_error) {
    printf("Invalid first char for Name object: [%c]. "
        "Must begin with forward slash.\n", c);
    cexit(fs, 1);
  }

  object_t* name = allocate(sizeof(object_t));
  name->offset = get_pos(fs);
  name->len = 0;
  name->type = Name;
  name->val = allocate(sizeof(name_t));

  name_t* name_val = name->val;
  name_val->memsize = 1;
  name_val->str = allocate(name_val->memsize);

  while ((c = get_char(fs, FAIL))) {
    int char_len = add_name_char(fs, c, name_val);

    if (!char_len) {
      // finished reading name
      unget_char(fs, c, FAIL);
      break;
    } else {
      name->len += char_len;
    }
  }

  return name;
}

// TODO: remove this.
char* name_str(FILE* fs, object_t* name) {
  char* ns = allocate(name->len + 1);
  seek(fs, name->offset, SEEK_SET, FAIL);
  size_t read = fread(ns, 1, name->len, fs);
  if (read != name->len) {
    fprintf(stderr,
        "Failed to read %i from fs. Read %li instead.\n",
        name->len, read);
  }
  return ns;
}

object_t* get_dictionary(FILE* fs, int fail_on_error) {
  size_t pos = get_pos(fs);
  object_t* first_key = allocate(sizeof(object_t));
  first_key->type = Name;
  first_key->offset = pos;
  first_key->len = 0;
  first_key->val = get_name(fs, fail_on_error);

  d_entry_t* entry = allocate(sizeof(d_entry_t));
  entry->key = first_key;
  entry->val = NULL;

  dict_t* dict = allocate(sizeof(dict_t));
  dict->entries = allocate(sizeof(d_entry_t*));
  dict->entries[0] = entry;
  dict->len = 1;

  object_t* obj = allocate(sizeof(object_t));
  obj->type = Dict;
  // points to the first char after dictionary sym '<<'
  obj->offset = pos;
  obj->len = 0;
  obj->val = (void*) dict;

  return obj;
}

void print_dictionary(dict_t* d) {
  for (int i = 0; i < d->len; ++i) {
    printf("KEY: (%s) VAL: %p\n", (char*) ((name_t*) d->entries[i]->key->val)->str, d->entries[i]->val);
  }
}

