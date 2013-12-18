#include "gtest/gtest.h"

#include "basic_test_case.hpp"

int main(int argc, char **argv)
{
  ::testing::InitGoogleTest(&argc, argv);

  return RUN_ALL_TESTS();
}
