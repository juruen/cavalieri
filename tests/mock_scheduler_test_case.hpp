#ifndef MOCK_SCHEDULER_TEST_CASE
#define MOCK_SCHEDULER_TEST_CASE

#include <scheduler/mock_scheduler.h>

TEST(mock_scheduler_unix_time_test_case, test)
{
  mock_scheduler sched;

  ASSERT_EQ(0, sched.unix_time());

  sched.process_event_time(1);
  ASSERT_EQ(1, sched.unix_time());

  sched.process_event_time(5);
  ASSERT_EQ(5, sched.unix_time());

  // Make sure we don't go back in time
  sched.process_event_time(4);
  ASSERT_EQ(5, sched.unix_time());

}

TEST(mock_scheduler_add_periodic_task_test_case, test)
{
  mock_scheduler sched;

  int calls1 = 0;
  auto t1 = [&]() { calls1++; };
  sched.add_periodic_task(t1, 5);

  sched.process_event_time(1);
  sched.process_event_time(2);
  ASSERT_EQ(0, calls1);

  sched.process_event_time(4);
  sched.process_event_time(5);
  ASSERT_EQ(1, calls1);

  sched.process_event_time(6);
  ASSERT_EQ(1, calls1);

  sched.process_event_time(15);
  ASSERT_EQ(3, calls1);

  int calls2 = 0;
  auto t2 = [&]() { calls2++; };
  sched.add_periodic_task(t2, 2);

  sched.process_event_time(15);
  ASSERT_EQ(3, calls1);
  ASSERT_EQ(0, calls2);

  sched.process_event_time(17);
  ASSERT_EQ(3, calls1);
  ASSERT_EQ(1, calls2);

  sched.process_event_time(20);
  ASSERT_EQ(4, calls1);
  ASSERT_EQ(2, calls2);

  sched.process_event_time(100);
  ASSERT_EQ(20, calls1);
  ASSERT_EQ(42, calls2);

  int calls3 = 0;
  auto t3 = [&]() { calls3++; };
  sched.add_periodic_task(t3, 5);
  sched.clear();
  ASSERT_EQ(0, calls3);
  ASSERT_EQ(0, sched.unix_time());
}

TEST(mock_scheduler_add_once_task_test_case, test)
{
  mock_scheduler sched;

  int calls1 = 0;
  auto t1 = [&]() { calls1++; };
  sched.add_once_task(t1, 5);

  sched.process_event_time(1);
  sched.process_event_time(2);
  ASSERT_EQ(0, calls1);

  sched.process_event_time(4);
  sched.process_event_time(5);
  ASSERT_EQ(1, calls1);

  sched.process_event_time(10);
  sched.process_event_time(11);
  ASSERT_EQ(1, calls1);
}


#endif
