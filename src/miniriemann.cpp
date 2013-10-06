#include <ev++.h>
#include <glog/logging.h>
#include "tcpserver.h"
#include "streams.h"
#include "util.h"

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

  /* Another stream example using split */
  streams.add_stream(

      split({
              {
                PRED(metric_to_double(e) > 0.8),

                CHILD(with({{"description", "metric > 0.8"}},

                      CHILD(prn())))

              },
              {
                PRED(metric_to_double(e) > 0.4),

                CHILD(with({{"description", "metric > 0.4"}},

                      CHILD(prn())))

              },
              {
                PRED(metric_to_double(e) > 0.2),

                CHILD(with({{"description", "metric > 0.2"}},

                      CHILD(prn())))

              }
            },

            /* Default stream */
            CHILD(with({{"description", "default < 0.2"}},
                  CHILD(prn())))));

  /* Yet another example using by() */
  streams.add_stream(

      by({"host", "service"},

         CHILD(
           BY(prn()))

      ));


  ev::default_loop  loop;
  TCPServer tcp(5555, streams);

  loop.run(0);

  return 0;
}
