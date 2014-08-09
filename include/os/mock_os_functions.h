#ifndef MOCK_OS_FUNCTIONS_H
#define MOCK_OS_FUNCTIONS_H

#include <vector>
#include <os/os_functions.h>

class mock_os_functions : public os_functions_interface {
public:
  ssize_t recv(int fd, void *buf, size_t len, int flags);
  ssize_t write(int fd, void *buf, size_t count);

  std::vector<unsigned char> buffer;
};

#endif
