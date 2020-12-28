#include "ini.h"
#include "test.h"
#include <stdlib.h>


void test_impl()
{
  struct IniSpecification *specification = ini_new_specification();

  specification->comment_character = '#';
  struct Ini *ini = ini_parse_string("[section]\na=1\n#b=2", specification);
  free(specification);

  assert_num_equal(ini->count, 1);

  struct IniSection *section = ini->sections[0];
  assert_string_equal(section->name, "section");
  assert_num_equal(section->count, 1);
  assert_string_equal(section->key_value_pairs[0]->key, "a");
  assert_string_equal(section->key_value_pairs[0]->value, "1");

  ini_release_ini_structure(ini);
}


int main()
{
  test_run(test_impl);
}

