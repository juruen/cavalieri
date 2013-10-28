#include <ev++.h>
#include <glog/logging.h>
#include <iostream>
#include <thread>
#include "index.h"
#include "tcpserver.h"
#include "riemanntcpconnection.h"
#include "websocket.h"
#include "streams.h"
#include "util.h"
#include "pubsub.h"
#include "driver.h"
#include "pagerduty.h"
#include "websocket.h"
#include "wsutil.h"


int main(int argc, char **argv)
{
  google::InitGoogleLogging(argv[0]);
  streams all_streams;
  pub_sub pubsub;
  UNUSED_VAR(argc);

  /* Stream example */
  all_streams.add_stream(

      /* Set metric to 1 */
      with({{"metric", "1"}},

           /* Compute rate in a 5-second interval */
           CHILD(rate(5,

                 /* Change description */
                 CHILD(with({{"description", "events/second"}},

                       /* Print event and add it to index*/
                       {prn()}))))));

  /* Another stream example using split */
  all_streams.add_stream(

      split({
              {
                PRED(metric_to_double(e) > 0.8),

                CHILD(with({{"description", "metric > 0.8"}},

                      CHILD(prn() )))

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
  all_streams.add_stream(

      by({"host", "service"},

         CHILD(
           BY(prn()))

      ));


  /* Example using checking tags */
  all_streams.add_stream(

      tagged_all({"stress-test", "baz"},

        CHILD(with({{"description", "tagged with stress-test and baz"}},

               CHILD(prn())))));

  /* Everything goes to the index. Check for expired events every 3 seconds */
  class index index(pubsub, [&](const Event& e){ all_streams.push_event (e);  }, 3);

  all_streams.add_stream(

      /* Set default tt to 9 sec */
      with_ifempty({{"ttl", "9"}},

        /* Push it to index */
        CHILD(send_index(index))));

  /*
  all_streams.add_stream(

          where(PRED(e.host() == "host0"),

                {where(PRED(metric_to_double(e) > 0.5),

                        {pd_trigger("foobar1234")},

                        {pd_resolve("foobar124")})}));
  */


  ev::default_loop  loop;

  tcp_server tcp_rieman_server(
      5555,
      [&](int sfd) {
        return std::make_shared<riemann_tcp_connection>(sfd, all_streams);
      }
  );

  tcp_server websocket(
      5556,
      [&](int sfd) {
        return std::make_shared<ws_connection>(sfd, new ws_util(), pubsub);
      }
  );


  loop.run(0);

  return 0;
}
