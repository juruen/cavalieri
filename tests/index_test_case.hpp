#ifndef INDEX_TEST_CASE_H
#define INDEX_TEST_CASE_H

#include <index/real_index.h>
#include <instrumentation/instrumentation.h>
#include <scheduler/mock_scheduler.h>
#include <core/core.h>
#include <glog/logging.h>

TEST(index_test_case, test)
{
  g_core->sched().clear();

  std::vector<Event> s;

  pub_sub pubsub;

  auto no_thread = [](std::function<void()> fn) { fn(); };

  config conf;
  instrumentation instr(conf);

  real_index index(pubsub, [&](const Event & pe) {  s.push_back(pe); },
                   60, g_core->sched(), instr, no_thread);

  Event e;
  e.set_host("foo");
  e.set_service("bar");
  e.set_time(1);
  e.set_ttl(120);

  index.add_event(e);

  auto queue = std::make_shared<tbb::concurrent_bounded_queue<Event>>();
  auto all_events_fn = pubsub.subscribe("index", queue);

  auto all_events = all_events_fn();
  ASSERT_EQ(1, all_events.size());
  ASSERT_EQ("foo", all_events[0]->host());
  ASSERT_EQ("bar", all_events[0]->service());

  e.set_host("baz");
  e.set_time(100);
  e.set_ttl(120);
  index.add_event(e);

  Event c;
  ASSERT_TRUE(queue->try_pop(c));
  ASSERT_EQ("baz", c.host());
  ASSERT_EQ("bar", c.service());

  g_core->sched().set_time(180);
  ASSERT_EQ(1, s.size());
  ASSERT_EQ("foo", s[0].host());
  ASSERT_EQ("bar", s[0].service());
  ASSERT_EQ("expired", s[0].state());
  s.clear();

}

#endif
