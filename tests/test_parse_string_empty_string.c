#include "ini.h"
#include "test.h"


void test_impl()
{
  struct Ini *ini = ini_parse_string("", NULL);

  assert_num_equal(ini->count, 0);

  ini_release_ini_structure(ini);
}


int main()
{
  test_run(test_impl);
}

