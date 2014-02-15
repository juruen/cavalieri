#include <os_functions.h>

os_functions::os_functions(os_functions_interface & impl) : impl_(impl)
{
}

ssize_t os_functions::recv(int fd, void *buf, size_t len, int flags) {
  return impl_.recv(fd, buf, len, flags);
}

ssize_t os_functions::write(int fd, void *buf, size_t count) {
  return impl_.write(fd, buf, count);
}
