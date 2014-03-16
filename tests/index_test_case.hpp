#ifndef INDEX_TEST_CASE_H
#define INDEX_TEST_CASE_H

#include <index/real_index.h>
#include <scheduler/mock_scheduler.h>
#include <glog/logging.h>

extern mock_scheduler mock_sched;

TEST(index_test_case, test)
{
  mock_sched.clear();

  std::vector<Event> s;

  pub_sub pubsub;

  auto no_thread = [](std::function<void()> fn) { fn(); };
  real_index index(pubsub, [&](const Event & pe) {  s.push_back(pe); },
                   60, no_thread);

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
  ASSERT_EQ("foo", all_events[0].host());
  ASSERT_EQ("bar", all_events[0].service());

  e.set_host("baz");
e.set_time(100);
  e.set_ttl(120);
  index.add_event(e);

  Event c;
  ASSERT_TRUE(queue->try_pop(c));
  ASSERT_EQ("baz", c.host());
  ASSERT_EQ("bar", c.service());

  mock_sched.process_event_time(180);
  ASSERT_EQ(1, s.size());
  ASSERT_EQ("foo", s[0].host());
  ASSERT_EQ("bar", s[0].service());
  ASSERT_EQ("expired", s[0].state());
  s.clear();

}

#endif
