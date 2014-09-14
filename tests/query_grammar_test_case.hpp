#ifndef QUERY_GRAMMAR_TEST_CASE
#define QUERY_GRAMMAR_TEST_CASE

#include <glog/logging.h>
#include <driver.h>
#include <expression.h>
#include <iostream>

TEST(query_grammar_true_test_case, test)
{
  query_context query_ctx;
  queryparser::driver driver(query_ctx);

  std::string query = "(true)";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  auto eval_fn = query_ctx.evaluate();

  Event e;
  ASSERT_TRUE(eval_fn(e));

  query = "true";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  ASSERT_TRUE(eval_fn(e));

}


TEST(query_grammar_tagged_test_case, test)
{
  query_context query_ctx;
  queryparser::driver driver(query_ctx);

  std::string query = "(tagged = \"foo\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  auto eval_fn = query_ctx.evaluate();

  Event e;
  // Check empty tags do not match anything
  ASSERT_FALSE(eval_fn(e));

  // Check foo matches
  e.add_tag("foo");
  ASSERT_TRUE(eval_fn(e));


  query = "(tagged = \"foo\" and tagged = \"bar\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  // Check that a missing "bar" tag doesn't match
  ASSERT_FALSE(eval_fn(e));

  //Check foo and bar match
  e.add_tag("bar");
  ASSERT_TRUE(eval_fn(e));

  query = "(tagged = \"foo\" or tagged = \"bar\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  // Check that only foo matches
  e.clear_tags();
  e.add_tag("foo");
  ASSERT_TRUE(eval_fn(e));

  //Check only bar matches
  e.clear_tags();
  e.add_tag("bar");
  ASSERT_TRUE(eval_fn(e));


  // Check that foo and bar match
  e.add_tag("foo");
  ASSERT_TRUE(eval_fn(e));

  query = "((tagged = \"foo\" or tagged = \"bar\") and (tagged = \"baz\"))";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  // Check that missing baz doesn't match
  e.clear_tags();
  e.add_tag("foo").add_tag("bar");
  ASSERT_FALSE(eval_fn(e));

  // Check that only foo and baz match
  e.clear_tags();
  e.add_tag("foo");
  e.add_tag("baz");
  ASSERT_TRUE(eval_fn(e));

 // Check that only bar and baz match
  e.clear_tags();
  e.add_tag("bar");
  e.add_tag("baz");
  ASSERT_TRUE(eval_fn(e));

  // Check that foo, bar and baz match
  e.clear_tags();
  e.add_tag("foo").add_tag("bar").add_tag("baz");
  ASSERT_TRUE(eval_fn(e));

  query = "(not tagged = \"foo\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  // Check that missing foo matches
  e.clear_tags();
  e.add_tag("bar");
  ASSERT_TRUE(eval_fn(e));

  // Check that foo doesn't match
  e.clear_tags();
  e.add_tag("foo");
  ASSERT_FALSE(eval_fn(e));

  query = "((tagged = \"foo\" or tagged = \"bar\") and not (tagged = \"baz\"))";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  // Check that missing baz matches
  e.clear_tags();
  e.add_tag("foo").add_tag("bar");
  ASSERT_TRUE(eval_fn(e));

  // Check thatbaz doesn't match
  e.clear_tags();
  e.add_tag("foo").add_tag("bar").add_tag("baz");
  ASSERT_FALSE(eval_fn(e));
}

TEST(query_grammar_fields_test_case, test)
{
  query_context query_ctx;
  queryparser::driver driver(query_ctx);

  std::string query = "(service = \"foo\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  auto eval_fn = query_ctx.evaluate();

  Event e;
  ASSERT_FALSE(eval_fn(e));

  e.set_service("foo");
  ASSERT_TRUE(eval_fn(e));

  query = "(host = \"foo\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  ASSERT_FALSE(eval_fn(e));

  e.clear_service();
  e.set_host("foo");
  ASSERT_TRUE(eval_fn(e));

  query = "(state = \"foo\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  ASSERT_FALSE(eval_fn(e));

  e.clear_host();
  e.set_state("foo");
  ASSERT_TRUE(eval_fn(e));

  query = "(description = \"foo\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  ASSERT_FALSE(eval_fn(e));

  e.clear_state();
  e.set_description("foo");
  ASSERT_TRUE(eval_fn(e));

  query = "(description =~ \"foob%\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  ASSERT_FALSE(eval_fn(e));

  e.clear_state();
  e.set_description("foobar");
  ASSERT_TRUE(eval_fn(e));


  query = "(time = 7)";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  ASSERT_FALSE(eval_fn(e));

  e.clear_description();
  e.set_time(7);
  ASSERT_TRUE(eval_fn(e));

  query = "(ttl = 7)";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  ASSERT_FALSE(eval_fn(e));

  e.clear_time();
  e.set_ttl(7);
  ASSERT_TRUE(eval_fn(e));

  query = "(ttl > 10)";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  ASSERT_FALSE(eval_fn(e));

  e.clear_time();
  e.set_ttl(11);
  ASSERT_TRUE(eval_fn(e));

  query = "(ttl >= 10)";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  e.clear_time();
  e.set_ttl(10);
  ASSERT_TRUE(eval_fn(e));

  query = "(ttl < 10)";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  ASSERT_FALSE(eval_fn(e));

  e.clear_time();
  e.set_ttl(9);
  ASSERT_TRUE(eval_fn(e));

  query = "(ttl <= 10)";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  e.clear_time();
  e.set_ttl(10);
  ASSERT_TRUE(eval_fn(e));


  query = "(metric = 0)";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  ASSERT_FALSE(eval_fn(e));

  e.clear_metric();
  e.set_metric_sint64(0);
  ASSERT_TRUE(eval_fn(e));


  query = "(metric = 7.1)";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  ASSERT_FALSE(eval_fn(e));

  e.clear_ttl();
  e.clear_metric();
  e.set_metric_d(7.1);
  ASSERT_TRUE(eval_fn(e));

  query = "(foo = \"bar\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  ASSERT_FALSE(eval_fn(e));

  e.clear_metric_d();
  e.set_attr("foo", "bar");
  ASSERT_TRUE(eval_fn(e));

  query = "(foo = 7.1)";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  ASSERT_FALSE(eval_fn(e));

  e.clear_attrs();
  e.set_attr("foo", "7.1");
  ASSERT_TRUE(eval_fn(e));

  query = "(host = nil)";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.evaluate();

  e.set_host("foo");
  ASSERT_FALSE(eval_fn(e));

  e.clear_host();
  ASSERT_TRUE(eval_fn(e));

}

#endif
