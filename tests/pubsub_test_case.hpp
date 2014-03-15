#ifndef PUBSUB_TEST_CASE
#define PUBSUB_TEST_CASE

#include <pub_sub/pub_sub.h>

TEST(pubsub_test_case, test)
{
  pub_sub pubsub;

  auto queue1 = std::make_shared<tbb::concurrent_bounded_queue<Event>>();
  auto queue2 = std::make_shared<tbb::concurrent_bounded_queue<Event>>();

  pubsub.add_publisher("topic1",
      []()->std::vector<Event>{ Event e; e.set_host("topic1 1"); return {e};});

  pubsub.add_publisher("topic2",
      []()->std::vector<Event>{ Event e; e.set_host("topic2 1"); return {e};});

  auto all_1_fn = pubsub.subscribe("topic1", queue1);
  auto all_2_fn = pubsub.subscribe("topic2", queue2);

  auto evs1 = all_1_fn();
  auto evs2 = all_2_fn();

  ASSERT_EQ(1, evs1.size());
  ASSERT_EQ(1, evs2.size());

  ASSERT_EQ("topic1 1", evs1[0].host());
  ASSERT_EQ("topic2 1", evs2[0].host());

  Event e;
  e.set_host("topic1 2");

  pubsub.publish("topic1", e);

  e.set_host("topic2 2");

  pubsub.publish("topic2", e);

  ASSERT_EQ(1, queue1->size());
  ASSERT_EQ(1, queue2->size());

  ASSERT_TRUE(queue1->try_pop(e));
  ASSERT_EQ("topic1 2", e.host());

  ASSERT_TRUE(queue2->try_pop(e));
  ASSERT_EQ("topic2 2", e.host());

}

#endif
