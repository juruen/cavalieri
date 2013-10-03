#include <ev++.h>
#include <glog/logging.h>
#include "tcpserver.h"
#include "streams.h"

int main(int argc, char **argv)
{
  google::InitGoogleLogging(argv[0]);
  Streams streams;
  streams.add_stream(Streams::prn());

  ev::default_loop  loop;
  TCPServer tcp(5555, streams);

  loop.run(0);

  return 0;
}
