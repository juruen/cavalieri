#ifndef FOLDS_TEST_CASE
#define FOLDS_TEST_CASE

#include <folds/folds.h>

TEST(sum_test_case, test)
{
  std::vector<Event> v;

  std::vector<Event> events(5);
  for (auto & e: events) {
    e.set_metric_d(1);
  }

  ASSERT_EQ(5, sum(events).metric_d());
}

TEST(product_test_case, test)
{
  std::vector<Event> v;

  std::vector<Event> events(5);
  for (auto & e: events) {
    e.set_metric_d(2);
  }

  ASSERT_EQ(1<<5, product(events).metric_d());
}

TEST(difference_test_case, test)
{
  std::vector<Event> v;

  std::vector<Event> events(3);
  for (auto & e: events) {
    e.set_metric_d(1);
  }

  ASSERT_EQ(-1, difference(events).metric_d());
}

TEST(mean_test_case, test)
{
  std::vector<Event> v;

  std::vector<Event> events(3);
  for (size_t i = 1; i < 4; i++) {
    events[i - 1].set_metric_d(i);
  }

  ASSERT_EQ(2, mean(events).metric_d());
}

TEST(maximum_test_case, test)
{
  std::vector<Event> v;

  std::vector<Event> events(3);
  for (size_t i = 1; i < 4; i++) {
    events[i - 1].set_metric_d(i);
  }

  ASSERT_EQ(3, maximum(events).metric_d());
}

TEST(minimum_test_case, test)
{
  std::vector<Event> v;

  std::vector<Event> events(3);
  for (size_t i = 1; i < 4; i++) {
    events[i - 1].set_metric_d(i);
  }

  ASSERT_EQ(1, minimum(events).metric_d());
}

#endif
