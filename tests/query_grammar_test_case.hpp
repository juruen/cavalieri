#ifndef QUERY_GRAMMAR_TEST_CASE
#define QUERY_GRAMMAR_TEST_CASE

#include <glog/logging.h>
#include <driver.h>
#include <expression.h>
#include <iostream>

TEST(query_grammar_true_test_case, test)
{
  QueryContext query_ctx;
  queryparser::Driver driver(query_ctx);

  std::string query = "(true)";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  auto eval_fn = query_ctx.expression->evaluate();

  Event e;
  ASSERT_TRUE(eval_fn(e));
}


TEST(query_grammar_tagged_test_case, test)
{
  QueryContext query_ctx;
  queryparser::Driver driver(query_ctx);

  std::string query = "(tagged = \"foo\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  auto eval_fn = query_ctx.expression->evaluate();

  Event e;
  // Check empty tags do not match anything
  ASSERT_FALSE(eval_fn(e));

  // Check foo matches
  *(e.add_tags()) = "foo";
  ASSERT_TRUE(eval_fn(e));


  query = "(tagged = \"foo\" and tagged = \"bar\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.expression->evaluate();

  // Check that a missing "bar" tag doesn't match
  ASSERT_FALSE(eval_fn(e));

  //Check foo and bar match
  *(e.add_tags()) = "bar";
  ASSERT_TRUE(eval_fn(e));

  query = "(tagged = \"foo\" or tagged = \"bar\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.expression->evaluate();

  // Check that only foo matches
  e.clear_tags();
  *(e.add_tags()) = "foo";
  ASSERT_TRUE(eval_fn(e));

  //Check only bar matches
  e.clear_tags();
  *(e.add_tags()) = "bar";
  ASSERT_TRUE(eval_fn(e));


  // Check that foo and bar match
  *(e.add_tags()) = "foo";
  ASSERT_TRUE(eval_fn(e));

  query = "((tagged = \"foo\" or tagged = \"bar\") and (tagged = \"baz\"))";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.expression->evaluate();

  // Check that missing baz doesn't match
  e.clear_tags();
  *(e.add_tags()) = "foo";
  *(e.add_tags()) = "bar";
  ASSERT_FALSE(eval_fn(e));

  // Check that only foo and baz match
  e.clear_tags();
  *(e.add_tags()) = "foo";
  *(e.add_tags()) = "baz";
  ASSERT_TRUE(eval_fn(e));

 // Check that only bar and baz match
  e.clear_tags();
  *(e.add_tags()) = "bar";
  *(e.add_tags()) = "baz";
  ASSERT_TRUE(eval_fn(e));

  // Check that foo, bar and baz match
  e.clear_tags();
  *(e.add_tags()) = "foo";
  *(e.add_tags()) = "bar";
  *(e.add_tags()) = "baz";
  ASSERT_TRUE(eval_fn(e));

  query = "(not tagged = \"foo\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.expression->evaluate();

  // Check that missing foo matches
  e.clear_tags();
  *(e.add_tags()) = "bar";
  ASSERT_TRUE(eval_fn(e));

  // Check that foo doesn't match
  e.clear_tags();
  *(e.add_tags()) = "foo";
  ASSERT_FALSE(eval_fn(e));

  query = "((tagged = \"foo\" or tagged = \"bar\") and not (tagged = \"baz\"))";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.expression->evaluate();

  // Check that missing baz matches
  e.clear_tags();
  *(e.add_tags()) = "foo";
  *(e.add_tags()) = "bar";
  ASSERT_TRUE(eval_fn(e));

  // Check thatbaz doesn't match
  e.clear_tags();
  *(e.add_tags()) = "foo";
  *(e.add_tags()) = "bar";
  *(e.add_tags()) = "baz";
  ASSERT_FALSE(eval_fn(e));
}

TEST(query_grammar_fields_test_case, test)
{
  QueryContext query_ctx;
  queryparser::Driver driver(query_ctx);

  std::string query = "(service = \"foo\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  auto eval_fn = query_ctx.expression->evaluate();

  Event e;
  // Check empty tags do not match anything
  ASSERT_FALSE(eval_fn(e));

  // Check foo matches
  e.set_service("foo");
  ASSERT_TRUE(eval_fn(e));

  query = "(host = \"foo\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.expression->evaluate();

  // Check empty tags do not match anything
  ASSERT_FALSE(eval_fn(e));

  // Check foo matches
  e.clear_service();
  e.set_host("foo");
  ASSERT_TRUE(eval_fn(e));

  query = "(state = \"foo\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.expression->evaluate();

  // Check empty tags do not match anything
  ASSERT_FALSE(eval_fn(e));

  // Check foo matches
  e.clear_host();
  e.set_state("foo");
  ASSERT_TRUE(eval_fn(e));

  query = "(description = \"foo\")";
  ASSERT_TRUE(driver.parse_string(query, "query"));

  eval_fn = query_ctx.expression->evaluate();

  // Check empty tags do not match anything
  ASSERT_FALSE(eval_fn(e));

  // Check foo matches
  e.clear_state();
  e.set_description("foo");
  ASSERT_TRUE(eval_fn(e));
}

#endif
