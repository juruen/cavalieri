#ifndef TCP_CONNECTION_TEST_CASE
#define TCP_CONNECTION_TEST_CASE

#include <vector>
#include <string>
#include <transport/tcp_connection.h>
#include <mock_os_functions.h>

extern mock_os_functions mock_os;

/*
TEST(tcp_connection_read_test_case, test)
{
  mock_os.buffer.clear();
  for (int i = 0; i < 1024; i++) {
    mock_os.buffer.push_back(i);
  }

  tcp_connection conn(0);

  conn.read(3);
  ASSERT_EQ(conn.close_connection, false);
  ASSERT_EQ(conn.r_buffer[0], 0);
  ASSERT_EQ(conn.r_buffer[1], 1);
  ASSERT_EQ(conn.r_buffer[2], 2);
  ASSERT_EQ(conn.bytes_read, 3);

  conn.read(0);
  ASSERT_EQ(conn.close_connection, true);
}

TEST(tcp_connection_copy_to_write_buffer_test_case, test)
{
  mock_os.buffer.clear();

  tcp_connection conn(0);

  std::string buffer("foobar");

  auto ret = conn.copy_to_write_buffer(buffer.c_str(), buffer.size());
  ASSERT_TRUE(ret);
  ASSERT_EQ(conn.bytes_to_write, buffer.size());

  ret = conn.copy_to_write_buffer(buffer.c_str(), conn.buffer_size + 1);
  ASSERT_FALSE(ret);

  ASSERT_TRUE(conn.pending_write());
  ASSERT_TRUE(conn.pending_read());
}

TEST(tcp_connection_write_test_case, test)
{
  mock_os.buffer.clear();
  mock_os.buffer.reserve(100);
  tcp_connection conn(0);

  std::string buffer("foobar");

  auto ret = conn.copy_to_write_buffer(buffer.c_str(), buffer.size());
  ASSERT_TRUE(ret);
  ASSERT_EQ(conn.bytes_to_write, buffer.size());

  ret = conn.write();
  ASSERT_EQ(conn.bytes_written, buffer.size());
  ASSERT_EQ(conn.bytes_to_write,0);
  ASSERT_TRUE((buffer
              == std::string{mock_os.buffer.begin(), mock_os.buffer.end()}));

}

*/
#endif
