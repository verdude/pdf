#include <limits.h>
#include <stdio.h>
#include <stdlib.h>

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
static int add_byte(unsigned char c, name_t* name) {
  int new_size = name->memsize;
  name->str = (unsigned char*) name->str;
  while (new_size < name->len + 1) {
    new_size += name->memsize;
  }

  if (new_size < 0) {
    fprintf(stderr, "Overflow computing new size: ptr:"
        " %p memsize: %i strlen: %i\n", name->str, name->memsize, name->len);
    return 0;
  }

  if (new_size > name->memsize) {
    name->str = realloc(name->str, new_size);
    if (name->str == NULL) {
      perror("realloc in add_byte");
      return 0;
    }
  }

  name->memsize = new_size;
  name->str[name->len++] = c;
  name->str[name->len] = 0;
  return 1;
}

/**
 * 1 for a regular char.
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
    int success = add_byte(c, name);
    if (!success) {
      cexit(fs, 1);
    }
    return 1;
  }

  if (c == '#') {
    unsigned char h_char = get_hex_char(fs);
    int success = add_byte(h_char, name);
    if (!success) {
      cexit(fs, 1);
    }
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

  object_t* name_obj = allocate(sizeof(object_t));
  name_obj->offset = get_pos(fs);
  name_obj->len = 0;
  name_obj->type = Name;
  name_obj->val = allocate(sizeof(name_t));

  name_t* name_val = name_obj->val;
  name_val->memsize = 1;
  name_val->str = allocate(name_val->memsize);

  while ((c = get_char(fs, FAIL))) {
    int char_len = add_name_char(fs, c, name_val);

    if (!char_len) {
      // finished reading name
      unget_char(fs, c, FAIL);
      break;
    } else {
      name_obj->len += char_len;
    }
  }

  return name_obj;
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
