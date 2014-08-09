#ifndef CAVALIERI_OS_FUNCTIONS_H
#define CAVALIERI_OS_FUNCTIONS_H

#include <sys/types.h>

class os_functions_interface {
public:
  virtual ssize_t recv(int fd, void *buf, size_t len, int flags) = 0;
  virtual ssize_t write(int fd, void *buf, size_t count) = 0;
};

class os_functions {
public:
  os_functions(os_functions_interface&);
  ssize_t recv(int fd, void *buf, size_t len, int flags);
  ssize_t write(int fd, void *buf, size_t count);

private:
  os_functions_interface & impl_;
};

extern os_functions g_os_functions;

#endif
