#include <ev++.h>
#include <glog/logging.h>
#include "tcpserver.h"
#include "streams.h"

int main(int argc, char **argv)
{
  google::InitGoogleLogging(argv[0]);
  Streams streams;

  /* Stream example */
  streams.add_stream(

      /* Set metric to 1 */
      with({{"metric", "1"}},

           /* Compute rate in a 5-second interval */
           CHILD(rate(5,

                 /* Change description */
                 CHILD(with({{"description", "events/second"}},

                       /* Print event */
                       CHILD(prn())))))));

  ev::default_loop  loop;
  TCPServer tcp(5555, streams);

  loop.run(0);

  return 0;
}
