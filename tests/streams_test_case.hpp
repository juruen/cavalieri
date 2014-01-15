#ifndef STREAMS_TEST_CASE
#define STREAMS_TEST_CASE

#include <thread>
#include <streams.h>
#include <scheduler.h>

extern mock_scheduler mock_sched;

namespace {
const size_t thread_num = 4;
const size_t iterations = 10000;
}

std::function<void(e_t e)> sink(std::vector<Event> & v) {
  return [&](e_t e) { v.push_back(e); };
}

TEST(call_rescue_streams_test_case, test)
{
  std::vector<Event> v1, v2, v3;

  Event e;
  call_rescue(e, {sink(v1)});
  ASSERT_EQ(1, v1.size());

  v1.clear();
  call_rescue(e, {sink(v1), sink(v2), sink(v3)});
  ASSERT_EQ(1, v1.size());
  ASSERT_EQ(1, v2.size());
  ASSERT_EQ(1, v3.size());
}

TEST(with_test_case, test)
{
  std::vector<Event> v;

  with_changes_t changes = {
    {"host", "host"},
    {"service", "service"},
    {"description", "description"},
    {"state", "state"},
    {"metric", 1},
    {"ttl", 2}
  };

  Event e;
  call_rescue(e, {with(changes, {sink(v)})});

  ASSERT_EQ(1, v.size());
  EXPECT_EQ("host", v[0].host());
  EXPECT_EQ("service", v[0].service());
  EXPECT_EQ("description", v[0].description());
  EXPECT_EQ("state", v[0].state());
  EXPECT_EQ(1, v[0].metric_sint64());
  EXPECT_EQ(2, v[0].ttl());

  v.clear();

  changes = {{"metric", 1.0}};
  call_rescue(e, {with(changes, {sink(v)})});
  ASSERT_EQ(1, v.size());
  EXPECT_EQ(1.0, v[0].metric_d());
  ASSERT_FALSE(v[0].has_metric_sint64());
  ASSERT_FALSE(v[0].has_metric_f());


  v.clear();
  changes = {{"attribute", "foo"}};
  call_rescue(e, {with(changes, {sink(v)})});
  ASSERT_EQ(1, v.size());
  EXPECT_EQ(1, v[0].attributes_size());
  EXPECT_EQ("attribute", v[0].attributes(0).key());
  EXPECT_EQ("foo", v[0].attributes(0).value());
}

TEST(with_ifempty_test_case, test)
{
  std::vector<Event> v;

  with_changes_t changes = {
    {"host", "host"},
    {"service", "service"},
  };

  Event e;
  e.set_host("localhost");
  call_rescue(e, {with_ifempty(changes, {sink(v)})});

  ASSERT_EQ(1, v.size());
  EXPECT_EQ("localhost", v[0].host());
  EXPECT_EQ("service", v[0].service());

  v.clear();

  ASSERT_FALSE(e.has_metric_d());
  ASSERT_FALSE(e.has_metric_f());
  ASSERT_FALSE(e.has_metric_sint64());

  changes = {{"metric", 1.0}};
  call_rescue(e, {with_ifempty(changes, {sink(v)})});
  ASSERT_TRUE(v[0].has_metric_d());
  ASSERT_EQ(1, v.size());
  EXPECT_EQ(1.0, v[0].metric_d());
  ASSERT_FALSE(v[0].has_metric_sint64());
  ASSERT_FALSE(v[0].has_metric_f());

  v.clear();

  changes = {{"metric", 2.0}};
  e.set_metric_d(1.0);
  call_rescue(e, {with_ifempty(changes, {sink(v)})});
  ASSERT_EQ(1, v.size());
  EXPECT_EQ(1.0, v[0].metric_d());
  ASSERT_FALSE(v[0].has_metric_sint64());
  ASSERT_FALSE(v[0].has_metric_f());

  v.clear();

  changes = {{"metric", 2}};
  e.set_metric_d(1.0);
  call_rescue(e, {with_ifempty(changes, {sink(v)})});
  ASSERT_EQ(1, v.size());
  EXPECT_EQ(1.0, v[0].metric_d());
  ASSERT_FALSE(v[0].has_metric_sint64());
  ASSERT_FALSE(v[0].has_metric_f());
}

TEST(split_test_case, test)
{
  std::vector<Event> v1, v2, v3;

  split_clauses_t clauses =
                            {

                              {PRED(e.host() == "host1"), sink(v1)},

                              {PRED(metric_to_double(e) > 3.3), sink(v2)}

                            };

  Event e;
  call_rescue(e, {split(clauses)});
  ASSERT_EQ(0, v1.size());
  ASSERT_EQ(0, v2.size());

  e.set_host("host2");
  call_rescue(e, {split(clauses)});
  ASSERT_EQ(0, v1.size());
  ASSERT_EQ(0, v2.size());

  e.set_host("host1");
  call_rescue(e, {split(clauses)});
  ASSERT_EQ(1, v1.size());
  ASSERT_EQ(0, v2.size());

  v1.clear();

  e.set_host("host1");
  e.set_metric_d(3.4);
  call_rescue(e, {split(clauses)});
  ASSERT_EQ(1, v1.size());
  ASSERT_EQ(0, v2.size());

  v1.clear();

  e.set_host("host2");
  e.set_metric_d(3.4);
  call_rescue(e, {split(clauses)});
  ASSERT_EQ(0, v1.size());
  ASSERT_EQ(1, v2.size());

  v2.clear();

  e.set_host("host3");
  e.set_metric_d(1.0);
  call_rescue(e, {split(clauses, sink(v3))});
  ASSERT_EQ(0, v1.size());
  ASSERT_EQ(0, v2.size());
  ASSERT_EQ(1, v3.size());
}

TEST(where_test_case, test)
{
  std::vector<Event> v1, v2;

  Event e;
  predicate_t predicate = PRED(e.host() == "foo");

  call_rescue(e, {where(predicate, {sink(v1)})});
  ASSERT_EQ(0, v1.size());

  e.set_host("foo");
  call_rescue(e, {where(predicate, {sink(v1)})});
  ASSERT_EQ(1, v1.size());

  e.set_host("bar");
  call_rescue(e, {where(predicate, {sink(v1)}, {sink(v2)})});
  ASSERT_EQ(1, v1.size());
  ASSERT_EQ(1, v2.size());
}

TEST(by_test_case, test)
{
  std::vector<std::vector<Event>> v;
  int i = 0;
  auto by_sink = [&]()
  {
    v.resize(++i);
    return [=,&v](e_t e) { v[i-1].push_back(e);};
  };

  by_keys_t by_keys = {"host", "service"};

  auto by_stream = by(by_keys, {by_sink});

  Event e1, e2, e3;
  e1.set_host("host1"); e1.set_service("service1");
  e2.set_host("host2"); e2.set_service("service2");
  e3.set_host("host3"); e3.set_service("service3");

  call_rescue(e1, {by_stream});
  call_rescue(e2, {by_stream});
  call_rescue(e3, {by_stream});

  ASSERT_EQ(3, v.size());
  ASSERT_EQ(1, v[0].size());
  ASSERT_EQ(1, v[1].size());
  ASSERT_EQ(1, v[2].size());

  call_rescue(e1, {by_stream});
  call_rescue(e2, {by_stream});
  call_rescue(e3, {by_stream});

  ASSERT_EQ(3, v.size());
  ASSERT_EQ(2, v[0].size());
  ASSERT_EQ(2, v[1].size());
  ASSERT_EQ(2, v[2].size());
}

TEST(rate_test_case, test)
{
  std::vector<Event> v;
  mock_sched.clear();

  Event e1, e2, e3;

  auto rate_stream = rate(5, {sink(v)});

  // Check that we send a 0-valued metric if no event is received
  call_rescue(e1, {rate_stream});
  mock_sched.process_event_time(5);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(0, v[0].metric_d());


  // Check {e1.metric_d=10.0, e2.metric_d=20.0, e3.metric_d=30.0}
  // gives us ((10.0 + 20.0 + 30.0) / 5) = 12
  e1.set_metric_d(10.0);
  e2.set_metric_d(20.0);
  e3.set_metric_d(30.0);
  call_rescue(e1, {rate_stream});
  call_rescue(e2, {rate_stream});
  call_rescue(e3, {rate_stream});
  mock_sched.process_event_time(10);
  ASSERT_EQ(2, v.size());
  ASSERT_EQ(12, v[1].metric_d());

  e1.clear_metric_d();
  e2.clear_metric_d();
  e3.clear_metric_d();

  // Same as above but mixing different metric types
  e1.set_metric_d(10.0);
  e2.set_metric_f(20.0);
  e3.set_metric_sint64(30);
  call_rescue(e1, {rate_stream});
  call_rescue(e2, {rate_stream});
  call_rescue(e3, {rate_stream});
  mock_sched.process_event_time(15);
  ASSERT_EQ(3, v.size());
  ASSERT_EQ(12, v[1].metric_d());

  mock_sched.clear();
}

TEST(rate_thread_test_case, test)
{
  std::vector<Event> v;
  mock_sched.clear();

  auto rate_stream = rate(5, {sink(v)});

  auto run_fn = [&]() {
    for (size_t i = 0; i < iterations; i++) {
      Event e;
      e.set_metric_sint64(1);
      call_rescue(e, {rate_stream});
    }
  };

  std::vector<std::thread> threads;
  for (size_t i = 0; i < thread_num; i++) {
    threads.push_back(std::move(std::thread(run_fn)));
  }

  for (size_t i = 0; i < thread_num; i++) {
    threads[i].join();
  }

  mock_sched.process_event_time(5);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(iterations * thread_num / 5.0, v[0].metric_d());

  mock_sched.clear();
}

TEST(coalesce_test_case, test)
{
  std::vector<Event> v;

  mock_sched.clear();

  auto coalesce_stream = coalesce({sink(v)});

  Event e;

  e.set_host("a");
  e.set_service("a");
  e.set_time(1);
  call_rescue(e, {coalesce_stream});
  v.clear();

  e.set_host("b");
  e.set_service("b");
  e.set_time(1);
  call_rescue(e, {coalesce_stream});
  v.clear();

  e.set_host("c");
  e.set_service("c");
  e.set_time(1);
  call_rescue(e, {coalesce_stream});

  ASSERT_EQ(3, v.size());
  v.clear();

  e.set_host("b");
  e.set_service("b");
  e.set_time(2);
  call_rescue(e, {coalesce_stream});

  ASSERT_EQ(3, v.size());
  bool ok = false;
  for (const auto & p: v) {
    if (p.host() == "b" && p.time() == 2) {
      ok = true;
      break;
    }
  }
  ASSERT_TRUE(ok);
  v.clear();

  mock_sched.process_event_time(100);
  e.set_host("b");
  e.set_service("b");
  e.set_time(90);
  call_rescue(e, {coalesce_stream});
  ASSERT_EQ(3, v.size());
  v.clear();

  e.set_host("b");
  e.set_service("b");
  e.set_time(91);
  call_rescue(e, {coalesce_stream});
  ASSERT_EQ(1, v.size());
}

TEST(project_test_case, test)
{
  std::vector<Event> v;

  mock_sched.clear();

  auto m1 = PRED(e.host() == "a");
  auto m2 = PRED(e.host() == "b");
  auto m3 = PRED(e.host() == "c");

  auto project_stream = project({m1, m2, m3}, {sink(v)});

  Event e;

  e.set_host("a");
  e.set_service("a");
  e.set_time(1);
  call_rescue(e, {project_stream});
  v.clear();

  e.set_host("b");
  e.set_service("b");
  e.set_time(1);
  call_rescue(e, {project_stream});
  v.clear();

  e.set_host("c");
  e.set_service("c");
  e.set_time(1);
  call_rescue(e, {project_stream});

  ASSERT_EQ(3, v.size());
  v.clear();

  e.set_host("b");
  e.set_service("b");
  e.set_time(2);
  call_rescue(e, {project_stream});

  ASSERT_EQ(3, v.size());
  bool ok = false;
  for (const auto & p: v) {
    if (p.host() == "b" && p.time() == 2) {
      ok = true;
      break;
    }
  }
  ASSERT_TRUE(ok);
  v.clear();

  mock_sched.process_event_time(100);
  e.set_host("b");
  e.set_service("b");
  e.set_time(90);
  call_rescue(e, {project_stream});
  ASSERT_EQ(3, v.size());
  v.clear();

  e.set_host("b");
  e.set_service("b");
  e.set_time(91);
  call_rescue(e, {project_stream});
  ASSERT_EQ(1, v.size());
}


TEST(changed_state_test_case, test)
{
  std::vector<Event> v;

  auto changed_stream = changed_state("a", {sink(v)});

  Event e;
  for (auto s : {"a", "a", "b", "b", "a", "b", "b"}) {
    e.set_state(s);
    call_rescue(e, {changed_stream});
  }

  ASSERT_EQ(3, v.size());
  ASSERT_EQ("b", v[0].state());
  ASSERT_EQ("a", v[1].state());
  ASSERT_EQ("b", v[2].state());
}

TEST(tagged_any_test_case, test)
{
  std::vector<Event> v;

  auto tag_stream = tagged_any({"foo", "bar"}, {sink(v)});

  Event e;

  call_rescue(e,  {tag_stream});
  ASSERT_EQ(0, v.size());

  *(e.add_tags()) = "baz";
  call_rescue(e,  {tag_stream});
  ASSERT_EQ(0, v.size());

 *(e.add_tags()) = "foo";
  call_rescue(e,  {tag_stream});
  ASSERT_EQ(1, v.size());

 *(e.add_tags()) = "bar";
  call_rescue(e,  {tag_stream});
  ASSERT_EQ(2, v.size());
}

TEST(tagged_all_test_case, test)
{
  std::vector<Event> v;

  auto tag_stream = tagged_all({"foo", "bar", "baz"}, {sink(v)});

  Event e;

  call_rescue(e,  {tag_stream});
  ASSERT_EQ(0, v.size());

  *(e.add_tags()) = "baz";
  call_rescue(e,  {tag_stream});
  ASSERT_EQ(0, v.size());

 *(e.add_tags()) = "foo";
  call_rescue(e,  {tag_stream});
  ASSERT_EQ(0, v.size());

 *(e.add_tags()) = "bar";
  call_rescue(e,  {tag_stream});
  ASSERT_EQ(1, v.size());
}

TEST(smap_test_case, test)
{
  std::vector<Event> v;

  auto smap_stream = smap(TR(e.set_host("foo")), {sink(v)});

  Event e;
  e.set_host("bar");

  call_rescue(e, {smap_stream});
  ASSERT_EQ(1, v.size());
  ASSERT_EQ("foo", v[0].host());
}

TEST(streams_test_case, test)
{
 std::vector<Event> v;

 streams s;
 s.add_stream(sink(v));

 Msg message;
 message.add_events();

 s.process_message(message);
 ASSERT_EQ(1, v.size());

 s.push_event({});
 ASSERT_EQ(2, v.size());
}

TEST(moving_event_window_test_case, test)
{
  std::vector<Event> v;

  auto moving_stream = moving_event_window(3, {sink(v)});

  Event e;

  e.set_metric_sint64(0);
  call_rescue(e, {moving_stream});
  ASSERT_EQ(1, v.size());
  v.clear();

  e.set_metric_sint64(1);
  call_rescue(e, {moving_stream});
  ASSERT_EQ(2, v.size());
  v.clear();

  e.set_metric_sint64(2);
  call_rescue(e, {moving_stream});
  ASSERT_EQ(3, v.size());
  v.clear();

  e.set_metric_sint64(3);
  call_rescue(e, {moving_stream});
  ASSERT_EQ(3, v.size());
  ASSERT_EQ(1, v[0].metric_sint64());
  ASSERT_EQ(2, v[1].metric_sint64());
  ASSERT_EQ(3, v[2].metric_sint64());

  v.clear();
}

TEST(fixed_event_window_test_case, test)
{
  std::vector<Event> v;

  auto moving_stream = fixed_event_window(3, {sink(v)});

  Event e;

  e.set_metric_sint64(0);
  call_rescue(e, {moving_stream});
  e.set_metric_sint64(1);
  call_rescue(e, {moving_stream});

  ASSERT_EQ(0, v.size());

  e.set_metric_sint64(2);
  call_rescue(e, {moving_stream});

  ASSERT_EQ(3, v.size());
  ASSERT_EQ(0, v[0].metric_sint64());
  ASSERT_EQ(1, v[1].metric_sint64());
  ASSERT_EQ(2, v[2].metric_sint64());

  v.clear();

  e.set_metric_sint64(3);
  call_rescue(e, {moving_stream});
  e.set_metric_sint64(4);
  call_rescue(e, {moving_stream});
  ASSERT_EQ(0, v.size());

  e.set_metric_sint64(5);
  call_rescue(e, {moving_stream});
  ASSERT_EQ(3, v.size());
  ASSERT_EQ(3, v[0].metric_sint64());
  ASSERT_EQ(4, v[1].metric_sint64());
  ASSERT_EQ(5, v[2].metric_sint64());
}

TEST(moving_time_window_test_case, test)
{
  std::vector<Event> v;

  auto moving_stream = moving_time_window(3, {sink(v)});

  Event e;

  // Push 3 events
  for (auto i = 0; i < 3; i++) {
    e.set_metric_sint64(i);
    e.set_time(i);
    call_rescue(e, {moving_stream});

    ASSERT_EQ(i + 1, v.size());
    v.clear();
  }

  e.set_metric_sint64(3);
  e.set_time(3);
  call_rescue(e, {moving_stream});

  ASSERT_EQ(3, v.size());
  ASSERT_EQ(1, v[0].metric_sint64());
  ASSERT_EQ(2, v[1].metric_sint64());
  ASSERT_EQ(3, v[2].metric_sint64());
  v.clear();


  e.set_metric_sint64(5);
  e.set_time(5);
  call_rescue(e, {moving_stream});
  ASSERT_EQ(2, v.size());
  v.clear();

  e.set_metric_sint64(4);
  e.set_time(4);
  call_rescue(e, {moving_stream});
  ASSERT_EQ(3, v.size());
  v.clear();

  e.set_metric_sint64(10);
  e.set_time(10);
  call_rescue(e, {moving_stream});
  ASSERT_EQ(1, v.size());
  v.clear();

  e.clear_time();
  call_rescue(e, {moving_stream});
  ASSERT_EQ(1, v.size());
  v.clear();
}

TEST(fixed_time_window_test_case, test)
{
  std::vector<Event> v;

  auto moving_stream = fixed_time_window(3, {sink(v)});

  Event e;

  // Push 3 events
  for (auto i = 0; i < 3; i++) {
    e.set_metric_sint64(i);
    e.set_time(i);
    call_rescue(e, {moving_stream});

    ASSERT_EQ(0, v.size());
    v.clear();
  }

  e.set_metric_sint64(3);
  e.set_time(3);
  call_rescue(e, {moving_stream});

  ASSERT_EQ(3, v.size());
  ASSERT_EQ(0, v[0].metric_sint64());
  ASSERT_EQ(1, v[1].metric_sint64());
  ASSERT_EQ(2, v[2].metric_sint64());
  v.clear();

  e.set_metric_sint64(4);
  e.set_time(4);
  call_rescue(e, {moving_stream});

  ASSERT_EQ(0, v.size());
  v.clear();

  e.set_metric_sint64(5);
  e.set_time(5);
  call_rescue(e, {moving_stream});

  ASSERT_EQ(0, v.size());
  v.clear();

  e.set_metric_sint64(6);
  e.set_time(6);
  call_rescue(e, {moving_stream});

  ASSERT_EQ(3, v.size());
  ASSERT_EQ(3, v[0].metric_sint64());
  ASSERT_EQ(4, v[1].metric_sint64());
  ASSERT_EQ(5, v[2].metric_sint64());
  v.clear();

  e.set_metric_sint64(10);
  e.set_time(10);
  call_rescue(e, {moving_stream});
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(6, v[0].metric_sint64());
  v.clear();


  e.set_metric_sint64(14);
  e.set_time(14);
  call_rescue(e, {moving_stream});
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(10, v[0].metric_sint64());
  v.clear();

  e.set_metric_sint64(1);
  e.set_time(1);
  call_rescue(e, {moving_stream});
  ASSERT_EQ(0, v.size());
  v.clear();
}

TEST(stable_test_case, test)
{
  std::vector<Event> v;

  auto stable_stream = stable(3, {sink(v)});

  Event e;

  e.set_metric_sint64(0);
  e.set_state("ok");
  e.set_time(0);
  call_rescue(e, {stable_stream});

  e.set_metric_sint64(1);
  e.set_state("ok");
  e.set_time(1);
  call_rescue(e, {stable_stream});

  ASSERT_EQ(0, v.size());
  v.clear();

  e.set_metric_sint64(4);
  e.set_state("ok");
  e.set_time(4);
  call_rescue(e, {stable_stream});

  ASSERT_EQ(3, v.size());
  ASSERT_EQ(0, v[0].metric_sint64());
  ASSERT_EQ(1, v[1].metric_sint64());
  ASSERT_EQ(4, v[2].metric_sint64());
  v.clear();

  e.set_metric_sint64(5);
  e.set_state("info");
  e.set_time(5);
  call_rescue(e, {stable_stream});
  ASSERT_EQ(0, v.size());

  e.set_metric_sint64(6);
  e.set_state("critical");
  e.set_time(6);
  call_rescue(e, {stable_stream});
  ASSERT_EQ(0, v.size());

  e.set_metric_sint64(7);
  e.set_state("critical");
  e.set_time(7);
  call_rescue(e, {stable_stream});
  ASSERT_EQ(0, v.size());

  e.set_metric_sint64(9);
  e.set_state("critical");
  e.set_time(9);
  call_rescue(e, {stable_stream});
  ASSERT_EQ(3, v.size());
  ASSERT_EQ(6, v[0].metric_sint64());
  ASSERT_EQ(7, v[1].metric_sint64());
  ASSERT_EQ(9, v[2].metric_sint64());

  e.set_metric_sint64(12);
  e.set_state("warning");
  e.set_time(12);
  call_rescue(e, {stable_stream});
}

TEST(throttle_test_case, test)
{
  std::vector<Event> v;

  auto throttle_stream = throttle(3, 5, {sink(v)});

  Event e;

  for (auto i = 0; i < 3; i++) {
    e.set_time(1);
    call_rescue(e, {throttle_stream});
    ASSERT_EQ(1, v.size());
    v.clear();
  }

  e.set_time(1);
  call_rescue(e, {throttle_stream});
  ASSERT_EQ(0, v.size());

  for (auto i = 0; i < 3; i++) {
    e.set_time(7);
    call_rescue(e, {throttle_stream});
    ASSERT_EQ(1, v.size());
    v.clear();
  }
}

TEST(above_test_case, test)
{
  std::vector<Event> v;

  auto above_stream = above(5, {sink(v)});

  Event e;

  e.set_metric_d(2);
  call_rescue(e, {above_stream});
  ASSERT_EQ(0, v.size());

  e.set_metric_d(7);
  call_rescue(e, {above_stream});
  ASSERT_EQ(1, v.size());
}

TEST(under_test_case, test)
{
  std::vector<Event> v;

  auto under_stream = under(5, {sink(v)});

  Event e;

  e.set_metric_d(7);
  call_rescue(e, {under_stream});
  ASSERT_EQ(0, v.size());

  e.set_metric_d(2);
  call_rescue(e, {under_stream});
  ASSERT_EQ(1, v.size());
}

TEST(within_test_case, test)
{
  std::vector<Event> v;

  auto within_stream = within(5, 8, {sink(v)});

  Event e;

  e.set_metric_d(2);
  call_rescue(e, {within_stream});
  e.set_metric_d(9);
  call_rescue(e, {within_stream});
  ASSERT_EQ(0, v.size());

  e.set_metric_d(6);
  call_rescue(e, {within_stream});
  ASSERT_EQ(1, v.size());
}

TEST(without_test_case, test)
{
  std::vector<Event> v;

  auto without_stream = without(5, 8, {sink(v)});

  Event e;

  e.set_metric_d(6);
  call_rescue(e, {without_stream});
  ASSERT_EQ(0, v.size());

  e.set_metric_d(2);
  call_rescue(e, {without_stream});
  e.set_metric_d(9);
  call_rescue(e, {without_stream});
  ASSERT_EQ(2, v.size());
}

TEST(scale_test_case, test)
{
  std::vector<Event> v;

  auto scale_stream = scale(2, {sink(v)});

  Event e;

  e.set_metric_d(6);
  call_rescue(e, {scale_stream});
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(12, metric_to_double(v[0]));
}

TEST(counter_test_case, test)
{
  std::vector<Event> v;

  auto counter_stream = counter({sink(v)});

  Event e;

  e.set_metric_d(1);
  call_rescue(e, {counter_stream});
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(1, metric_to_double(v[0]));
  v.clear();

  call_rescue(e, {counter_stream});
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(2, metric_to_double(v[0]));
  v.clear();

  call_rescue(e, {counter_stream});
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(3, metric_to_double(v[0]));
  v.clear();
}

TEST(tag_test_case, test)
{
  std::vector<Event> v;

  auto tag_stream = tag({"foo", "bar"}, {sink(v)});

  Event e;
  call_rescue(e, {tag_stream});
  ASSERT_EQ(1, v.size());
  ASSERT_TRUE(tagged_all_(v[0], {"foo", "bar"}));
}

TEST(expired_test_case, test)
{
  std::vector<Event> v;
  mock_sched.clear();

  auto expired_stream = expired({sink(v)});

  Event e;
  e.set_time(0);

  call_rescue(e, {expired_stream});
  ASSERT_EQ(0, v.size());

  e.set_state("critical");
  call_rescue(e, {expired_stream});
  ASSERT_EQ(0, v.size());

  e.set_state("expired");
  call_rescue(e, {expired_stream});
  ASSERT_EQ(1, v.size());
  v.clear();

  e.set_time(5);
  e.clear_state();
  call_rescue(e, {expired_stream});
  ASSERT_EQ(0, v.size());
  v.clear();

  mock_sched.process_event_time(100);
  call_rescue(e, {expired_stream});
  ASSERT_EQ(1, v.size());
}

#endif
