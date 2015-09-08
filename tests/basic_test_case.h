#ifndef BASIC_TEST_CASE
#define BASIC_TEST_CASE

#include <vector>
#include "util/util.h"

TEST(basic_test_case, test)
{
  std::vector<unsigned char> str;

  str.push_back('H');
  str.push_back('E');
  str.push_back('L');
  str.push_back('L');
  str.push_back('O');

  EXPECT_EQ("SEVMTE8=", base64Encode(str));
}

#endif
