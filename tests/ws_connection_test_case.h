#ifndef WS_CONNECTION_TEST_CASE
#define WS_CONNECTION_TEST_CASE

#include <vector>
#include <string>
#include <netinet/in.h>
#include <transport/ws_connection.h>
#include "mock_async_fd.h"
#include <os/mock_os_functions.h>
#include <common/event.h>
#include <async/async_loop.h>

extern mock_os_functions mock_os;

const  std::string k_ws_http_header(
  "GET /index?subscribed=true&query=(true) HTTP/1.1\r\n"
  "Upgrade: websocket\r\n"
  "Connection: Upgrade\r\n"
  "Host: localhost:5556\r\n"
  "Origin: http://localhost:5556\r\n"
  "Sec-WebSocket-Key: MPowoRLVQ9qicfX4apcBHg==\r\n"
  "Sec-WebSocket-Version: 13\r\n\r\n");

const std::string k_expected_response(
  "HTTP/1.1 101 Switching Protocols\r\n"
  "Upgrade: websocket\r\n"
  "Connection: Upgrade\r\n"
  "Sec-WebSocket-Accept: x53iN644piHAMydyR2BZ3wqY44U=\r\n\r\n");

std::string make_word(const size_t len) {
  size_t i = len;
  std::string word;
  while (i-- > 0) {
    word += 'a';
  }
  return word;
}

TEST(ws_connection_ok_message_test_case, test)
{
  tcp_connection conn(0);

  ws_connection ws_conn (conn);

  mock_async_fd async_fd;
  using ::testing::Return;

  EXPECT_CALL(async_fd, ready_read()).WillRepeatedly(Return(true));
  EXPECT_CALL(async_fd, ready_write()).WillRepeatedly(Return(true));

  mock_os.buffer.clear();
  mock_os.buffer.reserve(200000);

  std::copy(k_ws_http_header.begin(), k_ws_http_header.end(),
            back_inserter(mock_os.buffer));

  ASSERT_EQ(k_ws_http_header.size(), mock_os.buffer.size());

  ws_conn.callback(async_fd);

  // Check we upgrade to websocket
  ASSERT_EQ(
    k_expected_response,
    std::string(mock_os.buffer.begin() + k_ws_http_header.size(),
                mock_os.buffer.end())
  );

  ASSERT_TRUE(ws_conn.state() & ws_connection::k_read_frame_header);

  ASSERT_EQ("/index?subscribed=true&query=(true)", ws_conn.uri());

  // Check we can send frames of size < 125
  mock_os.buffer.clear();

  EXPECT_CALL(async_fd, ready_read()).WillRepeatedly(Return(false));

  auto word_64 = make_word(64);
  ws_conn.send_frame(word_64);
  ws_conn.callback(async_fd);

  ASSERT_EQ(0x81, mock_os.buffer[0]);
  ASSERT_EQ(0x40, mock_os.buffer[1]);
  ASSERT_EQ(word_64.c_str(),
            std::string(mock_os.buffer.begin() + 2, mock_os.buffer.end()));

  // Check we can send frames of 125 < size < 2**16
  mock_os.buffer.clear();

  EXPECT_CALL(async_fd, ready_read()).WillRepeatedly(Return(false));

  auto word_256 = make_word(256);
  ws_conn.send_frame(word_256);
  ws_conn.callback(async_fd);

  ASSERT_EQ(0x81, mock_os.buffer[0]);
  ASSERT_EQ(0x7e, mock_os.buffer[1]);
  ASSERT_EQ(0x01, mock_os.buffer[2]);
  ASSERT_EQ(0x00, mock_os.buffer[3]);
  ASSERT_EQ(word_256.c_str(),
            std::string(mock_os.buffer.begin() + 4, mock_os.buffer.end()));

  // Check we can send frames of size > 2**16
  mock_os.buffer.clear();

  auto word_100_000 = make_word(100000);
  ws_conn.send_frame(word_100_000);
  ws_conn.callback(async_fd);

  ASSERT_EQ(0x81, mock_os.buffer[0]);
  ASSERT_EQ(0x7F, mock_os.buffer[1]);
  ASSERT_EQ(0x86, mock_os.buffer[2]);
  ASSERT_EQ(0xA0, mock_os.buffer[3]);
  ASSERT_EQ(word_100_000.c_str(),
            std::string(mock_os.buffer.begin() + 10, mock_os.buffer.end()));
}

#endif
