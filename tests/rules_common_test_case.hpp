#ifndef RULES_COMMON_TEST_CASE
#define RULES_COMMON_TEST_CASE

#include <rules/common.h>

stream_t bsink(std::vector<Event> & v) {
  return [&](e_t e) { v.push_back(e); };
}


TEST(critical_above_test_case, test)
{
  std::vector<Event> v;

  auto cabove = critical_above(5, {bsink(v)});

  Event e;

  e.set_metric_d(1);
  call_rescue(e, {cabove});
  ASSERT_EQ("ok", v[0].state());

  e.set_metric_d(6);
  call_rescue(e, {cabove});
  ASSERT_EQ("critical", v[1].state());
}

TEST(critical_under_test_case, test)
{
  std::vector<Event> v;

  auto cunder = critical_under(5, {bsink(v)});

  Event e;

  e.set_metric_d(1);
  call_rescue(e, {cunder});
  ASSERT_EQ("critical", v[0].state());

  e.set_metric_d(6);
  call_rescue(e, {cunder});
  ASSERT_EQ("ok", v[1].state());
}

TEST(trigger_detrigger_above_test_case, test)
{
  std::vector<Event> v;

  auto td_above = trigger_detrigger_above(5, 5, 3, {bsink(v)});

  Event e;

  e.set_metric_d(1);
  e.set_time(1);
  call_rescue(e, {td_above});
  ASSERT_EQ(0, v.size());

  e.set_time(6);
  call_rescue(e, {td_above});
  ASSERT_EQ(2, v.size());
  ASSERT_EQ("ok", v[0].state());
  ASSERT_EQ("ok", v[1].state());
  v.clear();

  e.set_metric_d(6);
  e.set_time(10);
  call_rescue(e, {td_above});
  ASSERT_EQ(0, v.size());

  e.set_time(12);
  call_rescue(e, {td_above});
  ASSERT_EQ(0, v.size());

  e.set_time(15);
  call_rescue(e, {td_above});
  ASSERT_EQ(3, v.size());
  ASSERT_EQ("critical", v[0].state());
  ASSERT_EQ("critical", v[1].state());
  ASSERT_EQ("critical", v[2].state());
}

TEST(trigger_detrigger_under_test_case, test)
{
  std::vector<Event> v;

  auto td_under = trigger_detrigger_under(5, 5, 3, {bsink(v)});

  Event e;

  e.set_metric_d(1);
  e.set_time(1);
  call_rescue(e, {td_under});
  ASSERT_EQ(0, v.size());

  e.set_time(6);
  call_rescue(e, {td_under});
  ASSERT_EQ(2, v.size());
  ASSERT_EQ("critical", v[0].state());
  ASSERT_EQ("critical", v[1].state());
  v.clear();

  e.set_metric_d(6);
  e.set_time(10);
  call_rescue(e, {td_under});
  ASSERT_EQ(0, v.size());

  e.set_time(12);
  call_rescue(e, {td_under});
  ASSERT_EQ(0, v.size());

  e.set_time(15);
  call_rescue(e, {td_under});
  ASSERT_EQ(3, v.size());
  ASSERT_EQ("ok", v[0].state());
  ASSERT_EQ("ok", v[1].state());
  ASSERT_EQ("ok", v[2].state());
}




#endif
