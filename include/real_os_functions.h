#ifndef REAL_OS_FUNCTIONS_H
#define REAL_OS_FUNCTIONS_H

#include <os_functions.h>

class real_os_functions : public os_functions_interface {
public:
  ssize_t recv(int fd, void *buf, size_t len, int flags);
  ssize_t write(int fd, void *buff, size_t count);
};


#endif
