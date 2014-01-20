#ifndef FOLDS_TEST_CASE
#define FOLDS_TEST_CASE

#include <folds.h>

stream_t asink(std::vector<Event> & v) {
  return [&](e_t e) { v.push_back(e); };
}

TEST(folds_test_case, test)
{
  std::vector<Event> v;

  auto sum_fn = [](double x, double y) { return (x + y); };
  auto fold_stream = fold(sum_fn, {asink(v)});

  std::vector<Event> events(5);
  for (auto & e: events) {
    e.set_metric_d(1);
  }

  fold_stream(events);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(5, v[0].metric_d());
}

TEST(sum_test_case, test)
{
  std::vector<Event> v;

  auto sum_stream = sum({asink(v)});

  std::vector<Event> events(5);
  for (auto & e: events) {
    e.set_metric_d(1);
  }

  sum_stream(events);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(5, v[0].metric_d());
}

TEST(product_test_case, test)
{
  std::vector<Event> v;

  auto product_stream = product({asink(v)});

  std::vector<Event> events(5);
  for (auto & e: events) {
    e.set_metric_d(2);
  }

  product_stream(events);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(1<<5, v[0].metric_d());
}

TEST(difference_test_case, test)
{
  std::vector<Event> v;

  auto difference_stream = difference({asink(v)});

  std::vector<Event> events(3);
  for (auto & e: events) {
    e.set_metric_d(1);
  }

  difference_stream(events);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(-1, v[0].metric_d());
}

TEST(mean_test_case, test)
{
  std::vector<Event> v;

  auto mean_stream = mean({asink(v)});

  std::vector<Event> events(3);
  for (size_t i = 1; i < 4; i++) {
    events[i - 1].set_metric_d(i);
  }

  mean_stream(events);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(2, v[0].metric_d());
}

TEST(maximum_test_case, test)
{
  std::vector<Event> v;

  auto maximum_stream = maximum({asink(v)});

  std::vector<Event> events(3);
  for (size_t i = 1; i < 4; i++) {
    events[i - 1].set_metric_d(i);
  }

  maximum_stream(events);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(3, v[0].metric_d());
}

TEST(minimum_test_case, test)
{
  std::vector<Event> v;

  auto minimum_stream = minimum({asink(v)});

  std::vector<Event> events(3);
  for (size_t i = 1; i < 4; i++) {
    events[i - 1].set_metric_d(i);
  }

  minimum_stream(events);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(1, v[0].metric_d());
}

#endif
