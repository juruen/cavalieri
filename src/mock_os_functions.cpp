#include <mock_os_functions.h>
#include <algorithm>

ssize_t mock_os_functions::recv(int, void *buf, size_t len, int) {
  auto min = std::min(len, buffer.size());
  std::copy(buffer.begin(), buffer.begin() + min, static_cast<char*>(buf));
  return min;
}

ssize_t mock_os_functions::write(int, void *buf, size_t count) {
  auto min = std::min(count, buffer.capacity());
  auto pbuf = static_cast<char*>(buf);
  buffer.insert(buffer.end(), pbuf, pbuf + min);
  return min;
}
