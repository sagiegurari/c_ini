#include "ini.h"
#include "test.h"


void test_impl()
{
  struct Ini *ini = ini_parse_string("[section\na=1", NULL);

  assert_true(ini == NULL);
}


int main()
{
  test_run(test_impl);
}

