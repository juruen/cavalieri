#ifndef MOCK_ASYNC_FD_H
#define MOCK_ASYNC_FD_H

#include <gmock/gmock.h>
#include <async/async_loop.h>

class mock_async_fd : public async_fd {
  public:
    MOCK_CONST_METHOD0(fd, int());
    MOCK_CONST_METHOD0(error, bool());
    MOCK_METHOD0(stop, void());
    MOCK_CONST_METHOD0(ready_read, bool());
    MOCK_CONST_METHOD0(ready_write, bool());
    MOCK_METHOD1(set_mode, void(const mode&));
    MOCK_METHOD0(loop, async_loop&());
};


#endif
