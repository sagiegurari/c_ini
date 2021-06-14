#ifndef __INI_H__
#define __INI_H__

#include <stdbool.h>
#include <stddef.h>

struct IniKeyValue
{
  char *key;
  char *value;
};

struct IniSection
{
  char               *name;
  struct IniKeyValue **key_value_pairs;
  size_t             count;
};

struct Ini
{
  struct IniSection **sections;
  size_t            count;
};

struct IniSpecification
{
  char comment_character;
  bool allow_comment_after_value;
};

/**
 * Returns a new specification that can be altered.
 * It returns a default specification that is used by other functions
 * if no specification is provided to them.
 */
struct IniSpecification *ini_new_specification(void);

/**
 * Parses the give string and returns the Ini structure.
 * In case of any error, this function will return null.
 * The specification paramter is optional and if not provided, the
 * default specification will be used.
 * The Ini structure must be fully released once done.
 */
struct Ini *ini_parse_string(char * /* ini_text */, struct IniSpecification *);

/**
 * Writes the Ini struct as ini text and returns it.
 * This function will not validate the Ini structure is valid, for example
 * if multiple sections with same name exist.
 */
char *ini_to_string(struct Ini *);

/**
 * Releases the Ini struct and all internal memory used.
 * All internal strings will also be released, therefore no const strings
 * or freed strings must reside in the structure.
 */
void ini_release_ini_structure(struct Ini *);

#endif

