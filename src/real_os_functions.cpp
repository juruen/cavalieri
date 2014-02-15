#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <real_os_functions.h>

ssize_t real_os_functions::recv(int fd, void *buf, size_t len, int flags) {
  return ::recv(fd, buf, len, flags);
}

ssize_t real_os_functions::write(int fd, void *buf, size_t count) {
  return ::write(fd, buf, count);
}

real_os_functions real_os;
os_functions g_os_functions(real_os);
