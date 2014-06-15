#ifndef RULES_COMMON_TEST_CASE
#define RULES_COMMON_TEST_CASE

#include <rules/common.h>

streams_t bsink(std::vector<Event> & v) {
  return create_stream([&](forward_fn_t, e_t e) { v.push_back(e); });
}

TEST(critical_above_test_case, test)
{
  std::vector<Event> v;

  auto cabove = critical_above(5) >> bsink(v);

  Event e;

  e.set_metric_d(1);
  push_event(cabove, e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ("ok", v[0].state());

  e.set_metric_d(6);
  push_event(cabove, e);
  ASSERT_EQ(2, v.size());
  ASSERT_EQ("critical", v[1].state());
}

TEST(critical_under_test_case, test)
{
  std::vector<Event> v;

  auto cunder = critical_under(5) >>  bsink(v);

  Event e;

  e.set_metric_d(1);
  push_event(cunder, e);
  ASSERT_EQ("critical", v[0].state());

  e.set_metric_d(6);
  push_event(cunder, e);
  ASSERT_EQ("ok", v[1].state());
}

TEST(stable_metric_above_test_case, test)
{
  std::vector<Event> v;

  auto td_above = stable_metric(5, above_pred(5), under_pred(3)) >> bsink(v);

  Event e;

  e.set_metric_d(1);
  e.set_time(1);
  push_event(td_above, e);
  ASSERT_EQ(0, v.size());

  e.set_time(6);
  push_event(td_above, e);
  ASSERT_EQ(2, v.size());
  ASSERT_EQ("ok", v[0].state());
  ASSERT_EQ("ok", v[1].state());
  v.clear();

  e.set_metric_d(6);
  e.set_time(10);
  push_event(td_above, e);
  ASSERT_EQ(0, v.size());

  e.set_time(12);
  push_event(td_above, e);
  ASSERT_EQ(0, v.size());

  e.set_time(15);
  push_event(td_above, e);
  ASSERT_EQ(3, v.size());
  ASSERT_EQ("critical", v[0].state());
  ASSERT_EQ("critical", v[1].state());
  ASSERT_EQ("critical", v[2].state());
}

TEST(stable_metric_under_test_case, test)
{
  std::vector<Event> v;

  auto td_under = stable_metric(5, under_pred(5), above_pred(3)) >> bsink(v);

  Event e;

  e.set_metric_d(1);
  e.set_time(1);
  push_event(td_under, e);
  ASSERT_EQ(0, v.size());

  e.set_time(6);
  push_event(td_under, e);
  ASSERT_EQ(2, v.size());
  ASSERT_EQ("critical", v[0].state());
  ASSERT_EQ("critical", v[1].state());
  v.clear();

  e.set_metric_d(6);
  e.set_time(10);
  push_event(td_under, e);
  ASSERT_EQ(0, v.size());

  e.set_time(12);
  push_event(td_under, e);
  ASSERT_EQ(0, v.size());

  e.set_time(15);
  push_event(td_under, e);
  ASSERT_EQ(3, v.size());
  ASSERT_EQ("ok", v[0].state());
  ASSERT_EQ("ok", v[1].state());
  ASSERT_EQ("ok", v[2].state());
}

TEST(agg_sum_trigger_above_test_case,test)
{
  std::vector<Event> v;

  auto agg =  agg_stable_metric(5, sum, above_eq_pred(5), under_eq_pred(3))
              >> sink(v);

  g_core->sched().clear();

  Event e1,e2;
  e1.set_host("a");
  e1.set_service("foo");
  e1.set_metric_d(1);
  e2.set_host("b");
  e2.set_service("bar");
  e2.set_metric_d(1);

  e1.set_time(1);
  e2.set_time(1);
  push_event(agg,  e1);
  push_event(agg,  e2);
  ASSERT_EQ(0, v.size());

  e1.set_time(6);
  e2.set_time(6);
  push_event(agg,  e1);
  push_event(agg,  e2);

  ASSERT_EQ(4, v.size());
  v.clear();

  e1.set_time(7);
  push_event(agg,  e1);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ("ok", v[0].state());
  ASSERT_EQ(2, v[0].metric_d());
  v.clear();

  e2.set_time(10);
  e2.set_metric_d(5);
  push_event(agg,  e2);
  ASSERT_EQ(0, v.size());

  e1.set_time(14);
  push_event(agg,  e1);
  ASSERT_EQ(0, v.size());

  e2.set_time(16);
  push_event(agg,  e2);
  ASSERT_EQ(3, v.size());
  ASSERT_EQ("critical", v[0].state());
  ASSERT_EQ(6, v[0].metric_d());
}


TEST(max_critical_hosts_test_case,test)
{
  std::vector<Event> v;

  auto max = max_critical_hosts(3) >> changed_state("ok") >> sink(v);

  std::vector<Event> events(5);

  for (size_t i = 0; i < events.size(); i++) {
    events[i].set_host(std::to_string(i));
    events[i].set_state("ok");
    events[i].set_ttl(300);
    push_event(max, events[i]);
  }

  ASSERT_EQ(0, v.size());

  events[0].set_state("critical");
  push_event(max, events[0]);
  ASSERT_EQ(0, v.size());

  events[1].set_state("critical");
  events[2].set_state("critical");
  push_event(max, events[1]);
  push_event(max, events[2]);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ("critical", v[0].state());

  v.clear();

  events[0].set_state("ok");
  events[1].set_state("ok");
  events[2].set_state("oj");
  push_event(max, events[0]);
  push_event(max, events[1]);
  push_event(max, events[2]);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ("ok", v[0].state());

}

#endif
