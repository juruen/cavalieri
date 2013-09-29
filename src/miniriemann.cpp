#include <ev++.h>
#include <glog/logging.h>
#include "tcpserver.h"

int main(int argc, char **argv)
{
  int port = 5555;

  if (argc > 1)
    port = atoi(argv[1]);

  google::InitGoogleLogging(argv[0]);

  ev::default_loop  loop;
  TCPServer tcp(port);

  loop.run(0);

  return 0;
}
