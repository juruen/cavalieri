#ifndef FOLDS_TEST_CASE
#define FOLDS_TEST_CASE

#include <folds.h>

TEST(sum_test_case, test)
{
  std::vector<Event> v;

  std::vector<Event> events(5);
  for (auto & e: events) {
    e.set_metric_d(1);
  }

  ASSERT_EQ(fold_result_t(5), sum(events));
}

TEST(product_test_case, test)
{
  std::vector<Event> v;

  std::vector<Event> events(5);
  for (auto & e: events) {
    e.set_metric_d(2);
  }

  ASSERT_EQ(fold_result_t(1<<5), product(events));
}

TEST(difference_test_case, test)
{
  std::vector<Event> v;

  std::vector<Event> events(3);
  for (auto & e: events) {
    e.set_metric_d(1);
  }

  ASSERT_EQ(fold_result_t(-1), difference(events));
}

TEST(mean_test_case, test)
{
  std::vector<Event> v;

  std::vector<Event> events(3);
  for (size_t i = 1; i < 4; i++) {
    events[i - 1].set_metric_d(i);
  }

  ASSERT_EQ(fold_result_t(2), mean(events));
}

TEST(maximum_test_case, test)
{
  std::vector<Event> v;

  std::vector<Event> events(3);
  for (size_t i = 1; i < 4; i++) {
    events[i - 1].set_metric_d(i);
  }

  ASSERT_EQ(fold_result_t(3), maximum(events));
}

TEST(minimum_test_case, test)
{
  std::vector<Event> v;

  std::vector<Event> events(3);
  for (size_t i = 1; i < 4; i++) {
    events[i - 1].set_metric_d(i);
  }

  ASSERT_EQ(fold_result_t(1), minimum(events));
}

#endif
