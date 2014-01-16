#ifndef FOLDS_TEST_CASE
#define FOLDS_TEST_CASE

#include <folds.h>

stream_t asink(std::vector<Event> & v) {
  return [&](e_t e) { v.push_back(e); };
}

TEST(folds_update_test_case, test)
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
}

#endif
