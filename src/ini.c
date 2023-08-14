#include "ini.h"
#include "stringbuffer.h"
#include "stringfn.h"
#include "vector.h"
#include <stdlib.h>
#include <string.h>

static struct IniKeyValue *_ini_split_key_value(struct StringBuffer *, char *);
static char *_ini_parse_value(struct StringBuffer *, char *, struct IniSpecification *);
static bool _ini_section_to_string(struct StringBuffer *, struct IniSection *);
static void _ini_release_ini_section(struct IniSection *);

struct IniSpecification *ini_new_specification(void)
{
  struct IniSpecification *specification = malloc(sizeof(struct IniSpecification));

  specification->comment_character         = ';';
  specification->allow_comment_after_value = true;

  return(specification);
}

struct Ini *ini_parse_string(char *ini_text, struct IniSpecification *specification)
{
  if (ini_text == NULL)
  {
    return(NULL);
  }

  bool release_specification = false;
  if (specification == NULL)
  {
    specification         = ini_new_specification();
    release_specification = true;
  }

  struct StringBuffer    *buffer          = stringbuffer_new();
  struct Vector          *sections        = vector_new();
  struct Vector          *key_value_pairs = vector_new();
  struct StringFNStrings lines            = stringfn_split_lines_and_trim(ini_text);
  bool                   valid            = true;
  char                   *section_name    = NULL;
  for (int index = 0; index < lines.count; index++)
  {
    char   *line       = lines.strings[index];
    size_t line_length = strlen(line);

    if (!line_length || line[0] == specification->comment_character)
    {
      continue;
    }

    if (line[0] == '[')
    {
      if (line[line_length - 1] == ']')
      {
        if (section_name != NULL || vector_size(key_value_pairs))
        {
          struct IniSection *section = malloc(sizeof(struct IniSection));
          section->name = section_name;
          if (section->name == NULL)
          {
            section->name = strdup("");
          }
          section->count           = vector_size(key_value_pairs);
          section->key_value_pairs = (struct IniKeyValue **)vector_to_array(key_value_pairs);
          vector_clear(key_value_pairs);
          vector_push(sections, section);
        }

        section_name = stringfn_substring(line, 1, line_length - 2);
      }
      else
      {
        valid = false;
        break;
      }
    }
    else
    {
      struct IniKeyValue *key_value_pair = _ini_split_key_value(buffer, line);
      if (key_value_pair != NULL)
      {
        char *key   = stringfn_trim(key_value_pair->key);
        char *value = stringfn_trim(key_value_pair->value);
        free(key_value_pair->key);
        if (key_value_pair->value != NULL)
        {
          free(key_value_pair->value);
        }

        key   = _ini_parse_value(buffer, key, specification);
        value = _ini_parse_value(buffer, value, specification);

        key_value_pair->key   = key;
        key_value_pair->value = value;
        vector_push(key_value_pairs, key_value_pair);
      }
      else
      {
        valid = false;
        break;
      }
    }
  }

  stringfn_release_strings_struct(lines);

  if (section_name != NULL || vector_size(key_value_pairs))
  {
    struct IniSection *section = malloc(sizeof(struct IniSection));
    section->name = section_name;
    if (section->name == NULL)
    {
      section->name = strdup("");
    }
    section->count           = vector_size(key_value_pairs);
    section->key_value_pairs = (struct IniKeyValue **)vector_to_array(key_value_pairs);
    vector_clear(key_value_pairs);
    vector_push(sections, section);
  }

  struct Ini *ini = malloc(sizeof(struct Ini));
  ini->count    = vector_size(sections);
  ini->sections = (struct IniSection **)vector_to_array(sections);

  stringbuffer_release(buffer);
  vector_release(key_value_pairs);
  vector_release(sections);

  if (release_specification)
  {
    free(specification);
  }

  if (!valid)
  {
    ini_release_ini_structure(ini);
    return(NULL);
  }

  return(ini);
} /* ini_parse_string */


char *ini_to_string(struct Ini *ini)
{
  if (ini == NULL)
  {
    return(NULL);
  }

  if (ini->sections == NULL || !ini->count)
  {
    return(strdup(""));
  }

  struct StringBuffer *buffer = stringbuffer_new();

  for (size_t index = 0; index < ini->count; index++)
  {
    if (!_ini_section_to_string(buffer, ini->sections[index]))
    {
      stringbuffer_release(buffer);
      return(NULL);
    }
  }

  char *ini_text = stringbuffer_to_string(buffer);
  stringbuffer_release(buffer);

  return(ini_text);
}


void ini_release_ini_structure(struct Ini *ini)
{
  if (ini == NULL)
  {
    return;
  }

  if (ini->sections != NULL)
  {
    for (size_t index = 0; index < ini->count; index++)
    {
      _ini_release_ini_section(ini->sections[index]);
    }

    free(ini->sections);
  }

  free(ini);
}

static struct IniKeyValue *_ini_split_key_value(struct StringBuffer *buffer, char *line)
{
  stringbuffer_clear(buffer);

  size_t length = strlen(line);
  if (!length)
  {
    return(NULL);
  }

  bool in_escape = false;
  char *key      = NULL;
  char *value    = NULL;
  for (size_t index = 0; index < length; index++)
  {
    char character = line[index];

    if (in_escape)
    {
      if (character == '=')
      {
        stringbuffer_append(buffer, '=');
      }
      else
      {
        stringbuffer_append(buffer, '\\');
        stringbuffer_append(buffer, character);
      }

      in_escape = false;
    }
    else if (character == '\\')
    {
      in_escape = true;
    }
    else if (character == '=')
    {
      key   = stringbuffer_to_string(buffer);
      value = stringfn_substring(line, (int)(index + 1), 0);
      break;
    }
    else
    {
      stringbuffer_append(buffer, character);
    }
  }

  if (key == NULL)
  {
    return(NULL);
  }

  struct IniKeyValue *key_value_pair = malloc(sizeof(struct IniKeyValue));
  key_value_pair->key   = key;
  key_value_pair->value = value;

  return(key_value_pair);
} /* _ini_split_key_value */


static char *_ini_parse_value(struct StringBuffer *buffer, char *value, struct IniSpecification *specification)
{
  size_t value_length = strlen(value);

  if (!value_length)
  {
    return(value);
  }

  stringbuffer_clear(buffer);

  bool in_escape = false;
  for (size_t index = 0; index < value_length; index++)
  {
    char character = value[index];

    if (in_escape)
    {
      if (character == 'n')
      {
        stringbuffer_append(buffer, '\n');
      }
      else if (character == specification->comment_character)
      {
        stringbuffer_append(buffer, specification->comment_character);
      }
      else
      {
        stringbuffer_append(buffer, '\\');
        stringbuffer_append(buffer, character);
      }

      in_escape = false;
    }
    else if (character == '\\')
    {
      in_escape = true;
    }
    else if (character == specification->comment_character && specification->allow_comment_after_value)
    {
      break;
    }
    else
    {
      stringbuffer_append(buffer, character);
    }
  }

  if (in_escape)
  {
    stringbuffer_append(buffer, '\\');
  }

  free(value);
  value = stringbuffer_to_string(buffer);

  return(value);
} /* _ini_parse_value */


static void _ini_value_to_string(struct StringBuffer *buffer, char *value, bool is_key)
{
  if (value == NULL)
  {
    return;
  }

  size_t length = strlen(value);

  for (size_t index = 0; index < length; index++)
  {
    if (value[index] == '\n')
    {
      stringbuffer_append_string(buffer, "\\n");
    }
    else if (is_key && value[index] == '=')
    {
      stringbuffer_append_string(buffer, "\\=");
    }
    else
    {
      stringbuffer_append(buffer, value[index]);
    }
  }
}


static bool _ini_key_value_to_string(struct StringBuffer *buffer, struct IniKeyValue *key_value)
{
  if (key_value == NULL)
  {
    return(true);
  }

  if (key_value->key == NULL || !strlen(key_value->key))
  {
    return(false);
  }

  _ini_value_to_string(buffer, key_value->key, true);
  stringbuffer_append(buffer, '=');
  _ini_value_to_string(buffer, key_value->value, false);
  stringbuffer_append(buffer, '\n');

  return(true);
}


static bool _ini_section_to_string(struct StringBuffer *buffer, struct IniSection *section)
{
  if (section == NULL)
  {
    return(true);
  }

  if (section->name != NULL && strlen(section->name))
  {
    stringbuffer_append(buffer, '[');
    stringbuffer_append_string(buffer, section->name);
    stringbuffer_append_string(buffer, "]\n");
  }

  if (section->key_value_pairs != NULL)
  {
    for (size_t index = 0; index < section->count; index++)
    {
      if (!_ini_key_value_to_string(buffer, section->key_value_pairs[index]))
      {
        return(false);
      }
    }
  }

  stringbuffer_append(buffer, '\n');

  return(true);
}


static void _ini_release_ini_key_value(struct IniKeyValue *key_value)
{
  if (key_value == NULL)
  {
    return;
  }

  if (key_value->key != NULL)
  {
    free(key_value->key);
  }
  if (key_value->value != NULL)
  {
    free(key_value->value);
  }

  free(key_value);
}


static void _ini_release_ini_section(struct IniSection *section)
{
  if (section == NULL)
  {
    return;
  }

  if (section->name != NULL)
  {
    free(section->name);
  }

  if (section->key_value_pairs != NULL)
  {
    for (size_t index = 0; index < section->count; index++)
    {
      _ini_release_ini_key_value(section->key_value_pairs[index]);
    }

    free(section->key_value_pairs);
  }

  free(section);
}

