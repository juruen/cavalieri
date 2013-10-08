#include <ev++.h>
#include <glog/logging.h>
#include "index.h"
#include "tcpserver.h"
#include "websocket.h"
#include "streams.h"
#include "util.h"
#include "pubsub.h"

int main(int argc, char **argv)
{
  google::InitGoogleLogging(argv[0]);
  Streams streams;
  PubSub pubsub;
  Index index(pubsub);

  /* Stream example */
  streams.add_stream(

      /* Set metric to 1 */
      with({{"metric", "1"}},

           /* Compute rate in a 5-second interval */
           CHILD(rate(5,

                 /* Change description */
                 CHILD(with({{"description", "events/second"}},

                       /* Print event and add it to index*/
                       {prn(), send_index(index)}))))));

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


  /* Example using checking tags */
  streams.add_stream(

      tagged_all({"stress-test", "baz"},

                 CHILD(with({{"description", "tagged with stress-test and baz"}},

                       CHILD(prn())))));


  ev::default_loop  loop;
  TCPServer tcp(5555, streams);
  Websocket ws(5556, pubsub);

  loop.run(0);

  return 0;
}
