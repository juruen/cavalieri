#ifndef PUBSUB_TEST_CASE
#define PUBSUB_TEST_CASE

#include <pub_sub/pub_sub.h>

TEST(pubsub_test_case, test)
{
  pub_sub pubsub;

  pubsub.add_publisher("topic1");
  pubsub.add_publisher("topic2");

  std::vector<Event> vec1;
  std::vector<Event> vec2;

  pubsub.subscribe("topic1", [&](const Event & e){ vec1.push_back(e); });
  pubsub.subscribe("topic2", [&](const Event & e){ vec2.push_back(e); });

  Event e;

  e.set_host("topic1 1");
  pubsub.publish("topic1", e);

  e.set_host("topic2 1");
  pubsub.publish("topic2", e);

  ASSERT_EQ(1, vec1.size());
  ASSERT_EQ(1, vec2.size());

  ASSERT_EQ("topic1 1", vec1[0].host());
  ASSERT_EQ("topic2 1", vec2[0].host());

}

#endif
