#include "ini.h"
#include <stdio.h>
#include <stdlib.h>


int main()
{
  struct Ini *ini = ini_parse_string("; some comment\n\
      key=value\n\
      another_key=another value\n\
      ; more comments\n\
      \n\
      [section1]\n\
      a1=1\n\
      a2=2\n\
      [section2.subsection]\n\
      c1=1", NULL);

  printf("Parse ini, total sections: %zu\n", ini->count);

  struct IniSection *section = ini->sections[0];
  printf("Total key/values in first section: %zu\n", section->count);

  struct IniKeyValue *key_value_pair = section->key_value_pairs[0];
  printf("First key: %s value: %s\n", key_value_pair->key, key_value_pair->value);
  key_value_pair = section->key_value_pairs[1];
  printf("Second key: %s value: %s\n", key_value_pair->key, key_value_pair->value);

  char *ini_text = ini_to_string(ini);
  printf("Converted back to string:\n%s\n", ini_text);
  free(ini_text);

  ini_release_ini_structure(ini);

  return(0);
}
