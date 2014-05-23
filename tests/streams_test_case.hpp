#ifndef XTREAMS_TEST_CASE
#define XTREAMS_TEST_CASE

#include <streams/stream_infra.h>
#include <streams/stream_functions.h>
#include <scheduler/scheduler.h>
#include <core/core.h>
#include <util.h>
#include <iostream>

streams_t create_c_stream(const std::string c) {
  return create_stream([=](forward_fn_t forward, const Event & e)
      {
        Event ne(e);
        ne.set_host(ne.host() + c);
        forward(ne);
      });
}

streams_t sink(std::vector<Event> & v) {
  return create_stream([&](forward_fn_t, const Event & e)
      {
        v.push_back(e);
      });
}

std::function<Event(const std::vector<Event>)> msink(std::vector<Event> & v) {
  return [&](const events_t evs) {
    v = evs;
    Event e;
    return e;
  };
}

TEST(streams_test_case, test)
{
  auto a = create_c_stream("a");
  auto b = create_c_stream("b");
  auto c = create_c_stream("c");
  auto d = create_c_stream("d");
  auto f = create_c_stream("f");
  auto g = create_c_stream("g");

  std::vector<Event> v;

  Event e;

  push_event(sink(v), e);
  ASSERT_EQ(1, v.size());
  v.clear();

  push_event(a >> sink(v), e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ("a", v[0].host());
  v.clear();

  push_event(a >> b >> sink(v), e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ("ab", v[0].host());
  v.clear();

  push_event(a >> b >> c >> sink(v), e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ("abc", v[0].host());
  v.clear();

  push_event(a >> b >> c >> d >> sink(v), e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ("abcd", v[0].host());
  v.clear();

  push_event(a >>  b >> c >> d >> (sink(v), f, g), e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ("abcd", v[0].host());
  v.clear();

  push_event(a >>  b >> c >> d >> (f, sink(v),  g), e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ("abcd", v[0].host());
  v.clear();

  push_event(a >>  b >> c >> d >> (f, g, sink(v)), e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ("abcd", v[0].host());
  v.clear();

  auto streams = a >> b >> sink(v);
  push_event(c >> streams, e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ("cab", v[0].host());
  v.clear();

}

TEST(with_streams_test_case, test)
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

  push_event(with(changes) >> sink(v), e);

  ASSERT_EQ(1, v.size());
  EXPECT_EQ("host", v[0].host());
  EXPECT_EQ("service", v[0].service());
  EXPECT_EQ("description", v[0].description());
  EXPECT_EQ("state", v[0].state());
  EXPECT_EQ(1, v[0].metric_sint64());
  EXPECT_EQ(2, v[0].ttl());

  v.clear();

  changes = {{"metric", 1.0}};
  push_event(with(changes) >> sink(v), e);
  ASSERT_EQ(1, v.size());
  EXPECT_EQ(1.0, v[0].metric_d());
  ASSERT_FALSE(v[0].has_metric_sint64());
  ASSERT_FALSE(v[0].has_metric_f());


  v.clear();
  changes = {{"attribute", "foo"}};
  push_event(with(changes) >> sink(v), e);
  ASSERT_EQ(1, v.size());
  EXPECT_EQ(1, v[0].attributes_size());
  EXPECT_EQ("attribute", v[0].attributes(0).key());
  EXPECT_EQ("foo", v[0].attributes(0).value());
}

TEST(default_to_streams_test_case, test)
{
  std::vector<Event> v;

  with_changes_t changes = {
    {"host", "host"},
    {"service", "service"},
  };

  Event e;
  e.set_host("localhost");
  push_event(default_to(changes) >> sink(v), e);

  ASSERT_EQ(1, v.size());
  EXPECT_EQ("localhost", v[0].host());
  EXPECT_EQ("service", v[0].service());

  v.clear();

  ASSERT_FALSE(e.has_metric_d());
  ASSERT_FALSE(e.has_metric_f());
  ASSERT_FALSE(e.has_metric_sint64());

  changes = {{"metric", 1.0}};
  push_event(default_to(changes) >> sink(v), e);
  ASSERT_TRUE(v[0].has_metric_d());
  ASSERT_EQ(1, v.size());
  EXPECT_EQ(1.0, v[0].metric_d());
  ASSERT_FALSE(v[0].has_metric_sint64());
  ASSERT_FALSE(v[0].has_metric_f());

  v.clear();

  changes = {{"metric", 2.0}};
  e.set_metric_d(1.0);
  push_event(default_to(changes) >> sink(v), e);
  ASSERT_EQ(1, v.size());
  EXPECT_EQ(1.0, v[0].metric_d());
  ASSERT_FALSE(v[0].has_metric_sint64());
  ASSERT_FALSE(v[0].has_metric_f());

  v.clear();

  changes = {{"metric", 2}};
  e.set_metric_d(1.0);
  push_event(default_to(changes) >> sink(v), e);
  ASSERT_EQ(1, v.size());
  EXPECT_EQ(1.0, v[0].metric_d());
  ASSERT_FALSE(v[0].has_metric_sint64());
  ASSERT_FALSE(v[0].has_metric_f());
}

TEST(split_streams_test_case, test)
{
  std::vector<Event> v1, v2, v3;

  split_clauses_t clauses =
    {
      {PRED(e.host() == "host1"),       sink(v1)},
      {PRED(metric_to_double(e) > 3.3), sink(v2)}
    };

  Event e;
  push_event(split(clauses), e);
  ASSERT_EQ(0, v1.size());
  ASSERT_EQ(0, v2.size());

  e.set_host("host2");
  push_event(split(clauses), e);
  ASSERT_EQ(0, v1.size());
  ASSERT_EQ(0, v2.size());

  e.set_host("host1");
  push_event(split(clauses), e);
  ASSERT_EQ(1, v1.size());
  ASSERT_EQ(0, v2.size());

  v1.clear();

  e.set_host("host1");
  e.set_metric_d(3.4);
  push_event(split(clauses), e);
  ASSERT_EQ(1, v1.size());
  ASSERT_EQ(0, v2.size());

  v1.clear();

  e.set_host("host2");
  e.set_metric_d(3.4);
  push_event(split(clauses), e);
  ASSERT_EQ(0, v1.size());
  ASSERT_EQ(1, v2.size());

  v2.clear();

  e.set_host("host3");
  e.set_metric_d(1.0);

  push_event(split(clauses, sink(v3)), e);
  ASSERT_EQ(0, v1.size());
  ASSERT_EQ(0, v2.size());
  ASSERT_EQ(1, v3.size());

  v3.clear();

  split_clauses_t clauses_stream =
  {
    {PRED(e.host() == "host1"),       sdo()},
    {PRED(metric_to_double(e) > 3.3), sdo()}
  };

  e.set_host("host1");
  push_event(split(clauses_stream) >> sink(v3), e);
  ASSERT_EQ(0, v1.size());
  ASSERT_EQ(0, v2.size());
  ASSERT_EQ(1, v3.size());
}

TEST(where_xstream_test_case, test)
{
  std::vector<Event> v1, v2;

  Event e;
  predicate_t predicate = PRED(e.host() == "foo");

  push_event(where(predicate) >> sink(v1), e);
  ASSERT_EQ(0, v1.size());


  e.set_host("foo");
  push_event(where(predicate) >> sink(v1), e);
  ASSERT_EQ(1, v1.size());

  e.set_host("bar");
  push_event(where(predicate, sink(v2)) >> sink(v1), e);
  ASSERT_EQ(1, v1.size());
  ASSERT_EQ(1, v2.size());
}

TEST(by_streams_test_case, test)
{
  std::vector<std::vector<Event>> v;
  int i = 0;
  auto by_sink = [&]()
  {
    v.resize(++i);

    return create_stream([=,&v](forward_fn_t, const Event & e)
        {
          v[i - 1].push_back(e);
        });
  };

  by_keys_t by_keys = {"host", "service"};

  auto by_stream = by(by_keys, {by_sink});

  Event e1, e2, e3;
  e1.set_host("host1"); e1.set_service("service1");
  e2.set_host("host2"); e2.set_service("service2");
  e3.set_host("host3"); e3.set_service("service3");

  push_event(by_stream, e1);
  push_event(by_stream, e2);
  push_event(by_stream, e3);

  ASSERT_EQ(3, v.size());
  ASSERT_EQ(1, v[0].size());
  ASSERT_EQ(1, v[1].size());
  ASSERT_EQ(1, v[2].size());

  push_event(by_stream, e1);
  push_event(by_stream, e2);
  push_event(by_stream, e3);

  ASSERT_EQ(3, v.size());
  ASSERT_EQ(2, v[0].size());
  ASSERT_EQ(2, v[1].size());
  ASSERT_EQ(2, v[2].size());
}

TEST(changed_state_streams_test_case, test)
{
  std::vector<Event> v;

  auto changed_stream = changed_state("a") >>  sink(v);

  Event e;

  e.set_host("foo");
  e.set_service("bar");

  for (auto s : {"a", "a", "b", "b", "a", "b", "b"}) {
    e.set_state(s);
    push_event(changed_stream, e);
  }

  ASSERT_EQ(3, v.size());
  ASSERT_EQ("b", v[0].state());
  ASSERT_EQ("a", v[1].state());
  ASSERT_EQ("b", v[2].state());
  v.clear();

  e.set_service("baz");
  push_event(changed_stream, e);

  ASSERT_EQ(1, v.size());
  ASSERT_EQ("b", v[0].state());

}

TEST(tagged_any_test_case, test)
{
  std::vector<Event> v;

  auto tag_stream = tagged_any({"foo", "bar"}) >> sink(v);

  Event e;

  push_event(tag_stream, e);
  ASSERT_EQ(0, v.size());

  *(e.add_tags()) = "baz";

  push_event(tag_stream, e);
  ASSERT_EQ(0, v.size());

 *(e.add_tags()) = "foo";
  push_event(tag_stream, e);
  ASSERT_EQ(1, v.size());

 *(e.add_tags()) = "bar";
  push_event(tag_stream, e);
  ASSERT_EQ(2, v.size());
}

TEST(tagged_all_streams_test_case, test)
{
  std::vector<Event> v;

  auto tag_stream = tagged_all({"foo", "bar", "baz"}) >>  sink(v);

  Event e;

  push_event(tag_stream, e);
  ASSERT_EQ(0, v.size());

  *(e.add_tags()) = "baz";
  push_event(tag_stream, e);
  ASSERT_EQ(0, v.size());

 *(e.add_tags()) = "foo";
  push_event(tag_stream, e);
  ASSERT_EQ(0, v.size());

 *(e.add_tags()) = "bar";
  push_event(tag_stream, e);
  ASSERT_EQ(1, v.size());
}

TEST(tagged_streams_test_case, test)
{
  std::vector<Event> v;

  auto tag_stream = tagged("foo") >>  sink(v);

  Event e;

  push_event(tag_stream, e);
  ASSERT_EQ(0, v.size());

  *(e.add_tags()) = "baz";
  push_event(tag_stream, e);
  ASSERT_EQ(0, v.size());

 *(e.add_tags()) = "foo";
  push_event(tag_stream, e);
  ASSERT_EQ(1, v.size());
}



TEST(smap_streams_test_case, test)
{
  std::vector<Event> v;

  auto smap_stream = smap(TR(e.set_host("foo"))) >>  sink(v);

  Event e;
  e.set_host("bar");

  push_event(smap_stream, e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ("foo", v[0].host());
}

TEST(stable_streams_test_case, test)
{
  std::vector<Event> v;

  auto stable_stream = stable(3) >>  sink(v);

  Event e;

  e.set_metric_sint64(0);
  e.set_state("ok");
  e.set_time(0);
  push_event(stable_stream, e);

  e.set_metric_sint64(1);
  e.set_state("ok");
  e.set_time(1);
  push_event(stable_stream, e);


  ASSERT_EQ(0, v.size());
  v.clear();

  e.set_metric_sint64(4);
  e.set_state("ok");
  e.set_time(4);
  push_event(stable_stream, e);

  ASSERT_EQ(3, v.size());
  ASSERT_EQ(0, v[0].metric_sint64());
  ASSERT_EQ(1, v[1].metric_sint64());
  ASSERT_EQ(4, v[2].metric_sint64());
  v.clear();

  e.set_metric_sint64(5);
  e.set_state("info");
  e.set_time(5);
  push_event(stable_stream, e);
  ASSERT_EQ(0, v.size());

  e.set_metric_sint64(6);
  e.set_state("critical");
  e.set_time(6);
  push_event(stable_stream, e);
  ASSERT_EQ(0, v.size());

  e.set_metric_sint64(7);
  e.set_state("critical");
  e.set_time(7);
  push_event(stable_stream, e);
  ASSERT_EQ(0, v.size());

  e.set_metric_sint64(9);
  e.set_state("critical");
  e.set_time(9);
  push_event(stable_stream, e);
  ASSERT_EQ(3, v.size());
  ASSERT_EQ(6, v[0].metric_sint64());
  ASSERT_EQ(7, v[1].metric_sint64());
  ASSERT_EQ(9, v[2].metric_sint64());

  e.set_metric_sint64(12);
  e.set_state("warning");
  e.set_time(12);
  push_event(stable_stream, e);
}

TEST(throttle_streams_test_case, test)
{
  std::vector<Event> v;

  auto throttle_stream = throttle(3, 5) >>  sink(v);

  Event e;

  for (auto i = 0; i < 3; i++) {
    e.set_time(1);
    push_event(throttle_stream, e);
    ASSERT_EQ(1, v.size());
    v.clear();
  }

  e.set_time(1);
  push_event(throttle_stream, e);
  ASSERT_EQ(0, v.size());

  for (auto i = 0; i < 3; i++) {
    e.set_time(7);
    push_event(throttle_stream, e);
    ASSERT_EQ(1, v.size());
    v.clear();
  }
}

TEST(above_streams_test_case, test)
{
  std::vector<Event> v;

  auto above_stream = above(5) >>  sink(v);

  Event e;

  e.set_metric_d(2);
  push_event(above_stream, e);
  ASSERT_EQ(0, v.size());

  e.set_metric_d(7);
  push_event(above_stream, e);
  ASSERT_EQ(1, v.size());
}

TEST(under_streams_test_case, test)
{
  std::vector<Event> v;

  auto under_stream = under(5) >>  sink(v);

  Event e;

  e.set_metric_d(7);
  push_event(under_stream, e);
  ASSERT_EQ(0, v.size());

  e.set_metric_d(2);
  push_event(under_stream, e);
  ASSERT_EQ(1, v.size());
}

TEST(within_streams_test_case, test)
{
  std::vector<Event> v;

  auto within_stream = within(5, 8) >>  sink(v);

  Event e;

  e.set_metric_d(2);
  push_event(within_stream, e);
  e.set_metric_d(9);
  push_event(within_stream, e);
  ASSERT_EQ(0, v.size());

  e.set_metric_d(6);
  push_event(within_stream, e);
  ASSERT_EQ(1, v.size());
}

TEST(without_streams_test_case, test)
{
  std::vector<Event> v;

  auto without_stream = without(5, 8) >>  sink(v);

  Event e;

  e.set_metric_d(6);
  push_event(without_stream, e);
  ASSERT_EQ(0, v.size());

  e.set_metric_d(2);
  push_event(without_stream, e);
  e.set_metric_d(9);
  push_event(without_stream, e);
  ASSERT_EQ(2, v.size());
}

TEST(scale_streams_test_case, test)
{
  std::vector<Event> v;

  auto scale_stream = scale(2) >>  sink(v);

  Event e;

  e.set_metric_d(6);
  push_event(scale_stream, e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(12, metric_to_double(v[0]));
}

TEST(counter_streams_test_case, test)
{
  std::vector<Event> v;

  auto counter_stream = counter() >> sink(v);

  Event e;

  e.set_metric_d(1);
  push_event(counter_stream, e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(1, metric_to_double(v[0]));
  v.clear();

  push_event(counter_stream, e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(2, metric_to_double(v[0]));
  v.clear();

  push_event(counter_stream, e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(3, metric_to_double(v[0]));
  v.clear();
}

TEST(tag_streams_test_case, test)
{
  std::vector<Event> v;

  auto tag_stream = tag({"foo", "bar"}) >>  sink(v);

  Event e;
  push_event(tag_stream, e);
  ASSERT_EQ(1, v.size());
  ASSERT_TRUE(tagged_all_(v[0], {"foo", "bar"}));
}

TEST(expired_streams_test_case, test)
{
  std::vector<Event> v;
  g_core->sched().clear();

  auto expired_stream = expired() >> sink(v);

  Event e;
  e.set_time(0);

  push_event(expired_stream, e);
  ASSERT_EQ(0, v.size());

  e.set_state("critical");
  push_event(expired_stream, e);
  ASSERT_EQ(0, v.size());

  e.set_state("expired");
  push_event(expired_stream, e);
  ASSERT_EQ(1, v.size());
  v.clear();

  e.set_time(5);
  e.clear_state();
  push_event(expired_stream, e);
  ASSERT_EQ(0, v.size());
  v.clear();

  g_core->sched().set_time(100);
  push_event(expired_stream, e);
  ASSERT_EQ(1, v.size());
}

TEST(rate_streams_test_case, test)
{
  std::vector<Event> v;
  g_core->sched().clear();

  Event e1, e2, e3;

  auto rate_stream = rate(5) >>  sink(v);

  // Check that we send a 0-valued metric if no event is received
  g_core->sched().set_time(5);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(0, v[0].metric_d());


  // Check {e1.metric_d=10.0, e2.metric_d=20.0, e3.metric_d=30.0}
  // gives us ((10.0 + 20.0 + 30.0) / 5) = 12
  e1.set_metric_d(10.0);
  e2.set_metric_d(20.0);
  e3.set_metric_d(30.0);
  push_event(rate_stream, e1);
  push_event(rate_stream, e2);
  push_event(rate_stream, e3);
  g_core->sched().set_time(10);
  ASSERT_EQ(2, v.size());
  ASSERT_EQ(12, v[1].metric_d());

  e1.clear_metric_d();
  e2.clear_metric_d();
  e3.clear_metric_d();

  // Same as above but mixing different metric types
  e1.set_metric_d(10.0);
  e2.set_metric_f(20.0);
  e3.set_metric_sint64(30);
  push_event(rate_stream, e1);
  push_event(rate_stream, e2);
  push_event(rate_stream, e3);
  g_core->sched().set_time(15);
  ASSERT_EQ(3, v.size());
  ASSERT_EQ(12, v[1].metric_d());

  g_core->sched().clear();
}

TEST(coalesce_streams_test_case, test)
{
  std::vector<Event> v;

  g_core->sched().clear();

  auto coalesce_stream = coalesce(msink(v));

  Event e;

  e.set_host("a");
  e.set_service("a");
  e.set_time(1);
  push_event(coalesce_stream, e);
  v.clear();

  e.set_host("b");
  e.set_service("b");
  e.set_time(1);
  push_event(coalesce_stream, e);
  v.clear();

  e.set_host("c");
  e.set_service("c");
  e.set_time(1);
  push_event(coalesce_stream, e);

  ASSERT_EQ(3, v.size());
  v.clear();

  e.set_host("b");
  e.set_service("b");
  e.set_time(2);
  push_event(coalesce_stream, e);

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

  g_core->sched().set_time(100);
  e.set_host("b");
  e.set_service("b");
  e.set_time(90);
  push_event(coalesce_stream, e);
  ASSERT_EQ(1, v.size());
  v.clear();

  e.set_host("b");
  e.set_service("b");
  e.set_time(91);
  push_event(coalesce_stream, e);
  ASSERT_EQ(1, v.size());
}

TEST(project_streams_test_case, test)
{
  std::vector<Event> v;

  g_core->sched().clear();

  auto m1 = PRED(e.host() == "a");
  auto m2 = PRED(e.host() == "b");
  auto m3 = PRED(e.host() == "c");

  auto project_stream = project({m1, m2, m3}, msink(v));

  Event e;

  e.set_host("a");
  e.set_service("a");
  e.set_time(1);
  push_event(project_stream, e);
  v.clear();

  e.set_host("b");
  e.set_service("b");
  e.set_time(1);
  push_event(project_stream, e);
  v.clear();

  e.set_host("c");
  e.set_service("c");
  e.set_time(1);
  push_event(project_stream, e);

  ASSERT_EQ(3, v.size());
  v.clear();

  e.set_host("b");
  e.set_service("b");
  e.set_time(2);
  push_event(project_stream, e);

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

  g_core->sched().set_time(100);
  e.set_host("b");
  e.set_service("b");
  e.set_time(90);
  push_event(project_stream, e);
  ASSERT_EQ(1, v.size());
  v.clear();

  e.set_host("b");
  e.set_service("b");
  e.set_time(91);
  push_event(project_stream, e);
  ASSERT_EQ(1, v.size());
}

TEST(moving_event_window_streams_test_case, test)
{
  std::vector<Event> v;

  auto moving_stream = moving_event_window(3, msink(v));

  Event e;

  e.set_metric_sint64(0);
  push_event(moving_stream, e);
  ASSERT_EQ(1, v.size());
  v.clear();

  e.set_metric_sint64(1);
  push_event(moving_stream, e);
  ASSERT_EQ(2, v.size());
  v.clear();

  e.set_metric_sint64(2);
  push_event(moving_stream, e);
  ASSERT_EQ(3, v.size());
  v.clear();

  e.set_metric_sint64(3);
  push_event(moving_stream, e);
  ASSERT_EQ(3, v.size());
  ASSERT_EQ(1, v[0].metric_sint64());
  ASSERT_EQ(2, v[1].metric_sint64());
  ASSERT_EQ(3, v[2].metric_sint64());

  v.clear();
}

TEST(fixed_event_window_streams_test_case, test)
{
  std::vector<Event> v;

  auto moving_stream = fixed_event_window(3, msink(v));

  Event e;

  e.set_metric_sint64(0);
  push_event(moving_stream, e);
  e.set_metric_sint64(1);
  push_event(moving_stream, e);

  ASSERT_EQ(0, v.size());

  e.set_metric_sint64(2);
  push_event(moving_stream, e);

  ASSERT_EQ(3, v.size());
  ASSERT_EQ(0, v[0].metric_sint64());
  ASSERT_EQ(1, v[1].metric_sint64());
  ASSERT_EQ(2, v[2].metric_sint64());

  v.clear();

  e.set_metric_sint64(3);
  push_event(moving_stream, e);
  e.set_metric_sint64(4);
  push_event(moving_stream, e);
  ASSERT_EQ(0, v.size());

  e.set_metric_sint64(5);
  push_event(moving_stream, e);
  ASSERT_EQ(3, v.size());
  ASSERT_EQ(3, v[0].metric_sint64());
  ASSERT_EQ(4, v[1].metric_sint64());
  ASSERT_EQ(5, v[2].metric_sint64());
}

TEST(moving_time_window_streams_test_case, test)
{
  std::vector<Event> v;

  auto moving_stream = moving_time_window(3, msink(v));

  Event e;

  // Push 3 events
  for (auto i = 0; i < 3; i++) {
    e.set_metric_sint64(i);
    e.set_time(i);
    push_event(moving_stream, e);

    ASSERT_EQ(i + 1, v.size());
    v.clear();
  }

  e.set_metric_sint64(3);
  e.set_time(3);
  push_event(moving_stream, e);

  ASSERT_EQ(3, v.size());
  ASSERT_EQ(1, v[0].metric_sint64());
  ASSERT_EQ(2, v[1].metric_sint64());
  ASSERT_EQ(3, v[2].metric_sint64());
  v.clear();


  e.set_metric_sint64(5);
  e.set_time(5);
  push_event(moving_stream, e);
  ASSERT_EQ(2, v.size());
  v.clear();

  e.set_metric_sint64(4);
  e.set_time(4);
  push_event(moving_stream, e);
  ASSERT_EQ(3, v.size());
  v.clear();

  e.set_metric_sint64(10);
  e.set_time(10);
  push_event(moving_stream, e);
  ASSERT_EQ(1, v.size());
  v.clear();

  e.clear_time();
  push_event(moving_stream, e);
  ASSERT_EQ(1, v.size());
  v.clear();
}

TEST(fixed_time_window_streams_test_case, test)
{
  std::vector<Event> v;

  auto moving_stream = fixed_time_window(3, msink(v));

  Event e;

  // Push 3 events
  for (auto i = 0; i < 3; i++) {
    e.set_metric_sint64(i);
    e.set_time(i);
    push_event(moving_stream, e);

    ASSERT_EQ(0, v.size());
    v.clear();
  }

  e.set_metric_sint64(3);
  e.set_time(3);
  push_event(moving_stream, e);

  ASSERT_EQ(3, v.size());
  ASSERT_EQ(0, v[0].metric_sint64());
  ASSERT_EQ(1, v[1].metric_sint64());
  ASSERT_EQ(2, v[2].metric_sint64());
  v.clear();

  e.set_metric_sint64(4);
  e.set_time(4);
  push_event(moving_stream, e);

  ASSERT_EQ(0, v.size());
  v.clear();

  e.set_metric_sint64(5);
  e.set_time(5);
  push_event(moving_stream, e);

  ASSERT_EQ(0, v.size());
  v.clear();

  e.set_metric_sint64(6);
  e.set_time(6);
  push_event(moving_stream, e);

  ASSERT_EQ(3, v.size());
  ASSERT_EQ(3, v[0].metric_sint64());
  ASSERT_EQ(4, v[1].metric_sint64());
  ASSERT_EQ(5, v[2].metric_sint64());
  v.clear();

  e.set_metric_sint64(10);
  e.set_time(10);
  push_event(moving_stream, e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(6, v[0].metric_sint64());
  v.clear();


  e.set_metric_sint64(14);
  e.set_time(14);
  push_event(moving_stream, e);
  ASSERT_EQ(1, v.size());
  ASSERT_EQ(10, v[0].metric_sint64());
  v.clear();

  e.set_metric_sint64(1);
  e.set_time(1);
  push_event(moving_stream, e);
  ASSERT_EQ(0, v.size());
  v.clear();
}

TEST(service_streams_test_case, test)
{
  std::vector<Event> v;

  auto service_stream = service("foo")  >>  sink(v);

  Event e;

  push_event(service_stream, e);

  ASSERT_EQ(0, v.size());

  e.set_service("foo");
  push_event(service_stream, e);

  ASSERT_EQ(1, v.size());
  ASSERT_EQ("foo", v[0].service());
}

TEST(service_any_streams_test_case, test)
{
  std::vector<Event> v;

  auto service_stream = service_any({"foo", "bar"})  >>  sink(v);

  Event e;

  e.set_service("baz");
  push_event(service_stream, e);

  ASSERT_EQ(0, v.size());

  e.set_service("foo");
  push_event(service_stream, e);

  ASSERT_EQ(1, v.size());
  ASSERT_EQ("foo", v[0].service());
  v.clear();

  e.set_service("bar");
  push_event(service_stream, e);

  ASSERT_EQ(1, v.size());
  ASSERT_EQ("bar", v[0].service());
  v.clear();

}

TEST(service_like_streams_test_case, test)
{
  std::vector<Event> v;

  auto service_stream = service_like("foo%")  >>  sink(v);

  Event e;

  push_event(service_stream, e);

  ASSERT_EQ(0, v.size());

  e.set_service("foobar");
  push_event(service_stream, e);

  ASSERT_EQ(1, v.size());
  ASSERT_EQ("foobar", v[0].service());
}

TEST(service_like_any_streams_test_case, test)
{
  std::vector<Event> v;

  auto service_stream = service_like_any({"foo%", "bar%"})  >>  sink(v);

  Event e;

  e.set_service("baz");
  push_event(service_stream, e);

  ASSERT_EQ(0, v.size());

  e.set_service("foobar");
  push_event(service_stream, e);

  ASSERT_EQ(1, v.size());
  ASSERT_EQ("foobar", v[0].service());
  v.clear();

  e.set_service("bart");
  push_event(service_stream, e);

  ASSERT_EQ(1, v.size());
  ASSERT_EQ("bart", v[0].service());
  v.clear();

}

TEST(set_state_streams_test_case, test)
{
  std::vector<Event> v;

  auto set_stream = set_state("ok")  >>  sink(v);

  Event e;

  push_event(set_stream, e);

  ASSERT_EQ(1, v.size());
  ASSERT_EQ("ok", v[0].state());
}

TEST(set_metric_streams_test_case, test)
{
  std::vector<Event> v;

  auto set_stream = set_metric(1)  >>  sink(v);

  Event e;

  push_event(set_stream, e);

  ASSERT_EQ(1, v.size());
  ASSERT_EQ(1, metric_to_double(v[0]));
}

TEST(state_streams_test_case, test)
{
  std::vector<Event> v;

  auto state_stream = state("foo")  >>  sink(v);

  Event e;

  push_event(state_stream, e);

  ASSERT_EQ(0, v.size());

  e.set_state("foo");
  push_event(state_stream, e);

  ASSERT_EQ(1, v.size());
  ASSERT_EQ("foo", v[0].state());
}

TEST(ddt_streams_test_case, test)
{
  std::vector<Event> v;

  auto ddt_stream = ddt() >>  sink(v);

  Event e;

  e.set_metric_sint64(1);
  e.set_time(1);

  push_event(ddt_stream, e);

  ASSERT_EQ(0, v.size());

  e.set_metric_sint64(5);
  e.set_time(1);

  push_event(ddt_stream, e);

  ASSERT_EQ(0, v.size());


  e.set_metric_sint64(9);
  e.set_time(3);

  push_event(ddt_stream, e);

  ASSERT_EQ(1, v.size());
  ASSERT_EQ(2, v[0].metric_d());

}



#endif
