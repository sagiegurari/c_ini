#include "ini.h"
#include "test.h"
#include <stdlib.h>


void test_impl()
{
  struct Ini *ini = ini_parse_string(";test\n\
      \
      topk1=topv1\n\
      topk2=topv2\n\
      ; comment\n\
      \n\
      topk3=topv3;comment\n\
      \n\
      [section1]\n\
      a1\\n2=1\n\
      a2\\=3=2\n\
      [section2]\n\
      b1=1\\n2\\n3\n\
      \n\
      \n\
      b2=2\n\
      [section3]\n\
      [section4.subsection]\n\
      c1=1=2", NULL);

  assert_num_equal(ini->count, 5);

  struct IniSection *section = ini->sections[0];
  assert_string_equal(section->name, "");
  assert_num_equal(section->count, 3);
  assert_string_equal(section->key_value_pairs[0]->key, "topk1");
  assert_string_equal(section->key_value_pairs[0]->value, "topv1");
  assert_string_equal(section->key_value_pairs[1]->key, "topk2");
  assert_string_equal(section->key_value_pairs[1]->value, "topv2");
  assert_string_equal(section->key_value_pairs[2]->key, "topk3");
  assert_string_equal(section->key_value_pairs[2]->value, "topv3");

  section = ini->sections[1];
  assert_string_equal(section->name, "section1");
  assert_num_equal(section->count, 2);
  assert_string_equal(section->key_value_pairs[0]->key, "a1\n2");
  assert_string_equal(section->key_value_pairs[0]->value, "1");
  assert_string_equal(section->key_value_pairs[1]->key, "a2=3");
  assert_string_equal(section->key_value_pairs[1]->value, "2");

  section = ini->sections[2];
  assert_string_equal(section->name, "section2");
  assert_num_equal(section->count, 2);
  assert_string_equal(section->key_value_pairs[0]->key, "b1");
  assert_string_equal(section->key_value_pairs[0]->value, "1\n2\n3");
  assert_string_equal(section->key_value_pairs[1]->key, "b2");
  assert_string_equal(section->key_value_pairs[1]->value, "2");

  section = ini->sections[3];
  assert_string_equal(section->name, "section3");
  assert_num_equal(section->count, 0);

  section = ini->sections[4];
  assert_string_equal(section->name, "section4.subsection");
  assert_num_equal(section->count, 1);
  assert_string_equal(section->key_value_pairs[0]->key, "c1");
  assert_string_equal(section->key_value_pairs[0]->value, "1=2");

  char *ini_text   = ini_to_string(ini);
  char *normalized = "topk1=topv1\n\
topk2=topv2\n\
topk3=topv3\n\
\n\
[section1]\n\
a1\\n2=1\n\
a2\\=3=2\n\
\n\
[section2]\n\
b1=1\\n2\\n3\n\
b2=2\n\
\n\
[section3]\n\
\n\
[section4.subsection]\n\
c1=1=2\n\
\n";
  assert_string_equal(ini_text, normalized);
  free(ini_text);
  ini_release_ini_structure(ini);
} /* test_impl */


int main()
{
  test_run(test_impl);
}

