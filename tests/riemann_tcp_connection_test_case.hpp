#ifndef RIEMANN_TCP_CONNECTION_TEST_CASE
#define RIEMANN_TCP_CONNECTION_TEST_CASE

#include <vector>
#include <string>
#include <netinet/in.h>
#include <riemann_tcp_connection.h>
#include "mock_async_fd.hpp"
#include <os/mock_os_functions.h>
#include <proto.pb.h>
#include <async/async_loop.h>

extern mock_os_functions mock_os;

/*
 * XXX Temporarily disabled
TEST(riemann_tcp_connection_ok_message_test_case, test)
{
  tcp_connection conn(0);

  std::vector<unsigned char> sink;

  riemann_tcp_connection rconn (conn,
                                [&](std::vector<unsigned char> m) { sink = m;});

  mock_async_fd async_fd;
  using ::testing::Return;

  EXPECT_CALL(async_fd, ready_read()).WillRepeatedly(Return(true));
  EXPECT_CALL(async_fd, ready_write()).WillRepeatedly(Return(true));

  Msg msg;
  msg.set_ok(true);
  msg.add_events()->set_host("foo");

  mock_os.buffer.clear();

  auto nsize = htonl(msg.ByteSize());

  mock_os.buffer.resize(sizeof(nsize) + msg.ByteSize());

  memcpy(&mock_os.buffer[0], static_cast<void*>(&nsize), sizeof(nsize));
  msg.SerializeToArray(&mock_os.buffer[0] + sizeof(nsize), msg.ByteSize());

  rconn.callback(async_fd);

  // Check the raw msg is forwarded correctly
  ASSERT_EQ(msg.ByteSize(), sink.size());

  Msg msg_from_rconn;
  msg_from_rconn.ParseFromArray(&sink[0], sink.size());
  ASSERT_TRUE(msg_from_rconn.ok());
  ASSERT_EQ(1, msg_from_rconn.events_size());
  ASSERT_EQ("foo", msg_from_rconn.events(0).host());

  // Check riemann_tcp_connection acks the msg
  uint32_t response_size;
  memcpy(static_cast<void*>(&response_size), &mock_os.buffer[0], 4);
  response_size = ntohl(response_size);

  Msg msg_ack;
  msg_ack.ParseFromArray(&mock_os.buffer[4], response_size);
  ASSERT_TRUE(msg_ack.ok());
}
*/

TEST(riemann_tcp_connection_broken_size_test_case, test)
{
  tcp_connection conn(0);

  std::vector<unsigned char> sink;

  riemann_tcp_connection rconn (conn,
                                [&](std::vector<unsigned char> m) { sink = m;});

  mock_async_fd async_fd;
  using ::testing::Return;

  EXPECT_CALL(async_fd, ready_read()).WillRepeatedly(Return(true));
  EXPECT_CALL(async_fd, ready_write()).WillRepeatedly(Return(true));

  Msg msg;
  msg.set_ok(true);

  mock_os.buffer.clear();

  auto nsize = htonl(conn.buff_size + 1);

  mock_os.buffer.resize(sizeof(nsize) + msg.ByteSize());

  memcpy(&mock_os.buffer[0], static_cast<void*>(&nsize), sizeof(nsize));

  rconn.callback(async_fd);

  // Check the connection is closed when header reports a huge msg size
  ASSERT_TRUE(conn.close_connection);
}

#endif
