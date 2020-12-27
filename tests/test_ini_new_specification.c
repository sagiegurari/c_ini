#include "ini.h"
#include "test.h"
#include <stdlib.h>


void test_impl()
{
  struct IniSpecification *specification = ini_new_specification();

  assert_true(specification->comment_character == ';');
  assert_true(specification->allow_comment_after_value);

  free(specification);
}


int main()
{
  test_run(test_impl);
}

