Cavalieri [![Build Status](https://drone.io/github.com/juruen/cavalieri/status.png)](https://drone.io/github.com/juruen/cavalieri/latest)
=========


Introduction
------------

*Cavalieri* is a C++ event stream processing tool inspired by the
awesome [riemann.io](http://riemann.io) project.

It implements the original [riemann.io](http://riemann.io) protocol. That means
you can leverage the existing *riemann* clients and tools. It also tries to
mimic its stream API where possible.

Cavalieri's current version *0.0.6* is considered to be  in **alpha** state.
We expect to release a beta version in the following weeks.

Current benchmarks show that it can process more than one million events per
second with simple streams.

Install
-------

#### Ubuntu packages

You can install a deb package for Ubuntu 13.10 and 14.04 by adding this ppa:

```
sudo add-apt-repository ppa:juruen/cavalieri
sudo apt-get update
sudo apt-get install cavalieri
```

#### Debian

Packges coming soon!

#### Build from source

Have a look at the build dependencies extracted from the deb package:


```
cmake, subversion, protobuf-compiler,libprotobuf-dev, libev-dev, libgflags-dev, 
libgoogle-glog-dev, libpython-dev,libcurl4-openssl-dev, libssl-dev, libtbb-dev,
libjsoncpp-dev, lcov, flex, bison, libgoogle-glog-dev, libboost-filesystem-dev,
libboost-system-dev

```

You will also need a C++11 enabled compiler. GCC >= 4.7 or an equivalent Clang.

Once the depencies are met. To build and install, do the following:

```
mkdir build
cd build
cmake ..
make install
```

Create your rules
-----------------

Clone *cavalieri-rules*, a template project to create your own rules.

```sh
git clone https://github.com/juruen/cavalieri-rules.git
cd cavalieri-rules
```

Open *rules.cpp* and add your rules. The default rule will send an email
containing a critical event when a metric from *requests_rate*
service is above 40.

```cpp
#include <rules.h>
#include <external/pagerduty.h>
#include <external/email.h>

streams_t rules() {

  auto mail_stream = email("localhost", "cavalieri@localhost",
                           "devops@localhost");

  auto s =  service("requests_rate")
               >> above(40)
                 >> set_state("critical")
                   >> changed_state("ok")
                     >>  mail_stream;

  return s;
}

EXPORT_RULES(rules)
```

Build a plugin containing your rules that will be loaded by *cavalieri*.

```sh
mkdir build
cd build
cmake ..
make
```

The above step generates a *librules.so* file that *cavalieri* will load.

Execute cavalieri in the build directory or use the **-rules_directory**
flag to specifify where the plugin is.

```
$ cavalieri
I0403 23:15:44 config.cpp:36] config:
I0403 23:15:44 config.cpp:37]      events_port: 5555
I0403 23:15:44 config.cpp:38]      rimeann_tcp_pool_size:: 1
I0403 23:15:44 config.cpp:39]      ws_port: 5556
I0403 23:15:44 config.cpp:40]      ws_pool_size: 1
I0403 23:15:44 config.cpp:41]      index_expire_interval: 60
I0403 23:15:44 config.cpp:42]      rules_directory: .
I0403 23:15:44 config.cpp:43] --
I0403 23:15:44 rules_loader.cpp:60] rules loaded succesfully from librules.so

```

Test your rules
---------------

You can easily test your rules without putting them in production.

In the root directory of *cavalieri-template* there is a Python script that will
help you to find out whether your rules are doing what you expect or not.

The way it works is pretty simple. Open *test_rules.py* and add the events that 
are supposed to trigger your alerts.  By default, you already have some events
that will trigger an alert for the default rules:

```python
events = [
  {'host': 'foo.org', 'service': 'requests_rate', 'metric': 10, 'time': 0},
  {'host': 'foo.org', 'service': 'requests_rate', 'metric': 20, 'time': 60},
  {'host': 'foo.org', 'service': 'requests_rate', 'metric': 40, 'time': 120},
  {'host': 'foo.org', 'service': 'requests_rate', 'metric': 45, 'time': 180},
  {'host': 'foo.org', 'service': 'requests_rate', 'metric': 45, 'time': 240},
  {'host': 'foo.org', 'service': 'requests_rate', 'metric': 20, 'time': 300},
  {'host': 'foo.org', 'service': 'requests_rate', 'metric': 10, 'time': 360}]
  ```

Let's have a look at the default rules again:

```cpp

[...]

auto s =  service("requests_rate")
           >> above(40)
             >> set_state("critical")
               >> changed_state("ok")
                 >>  mail_stream;

[...]

```

As you can see, the events defined in *test_rules.py* will trigger an alert
when the metric is above 40. This happens for the event that is sent
at time *180*.

You can execute *test_rules.py* from the root directory of
*cavalieri-template*. And you will magically see what happens to your
rules when those events are passed through them:

```json
{
   "index" : [],
   "reports" : [
      [
         "email",
         {
            "event" : {
               "description" : "",
               "host" : "foo.org",
               "metric" : 45.0,
               "service" : "requests_rate",
               "state" : "critical",
               "tags" : [],
               "time" : 180
            },
            "extra" : "",
            "message" : "send email from: cavalieri@localhost to: devops@localhost",
            "time" : 180
         }
      ]
   ]
}

```

As you can see in the above output, *test_rules.py* is reporting that an email
would have been sent at time *180* to report that the *requests_rate* service
is critical.

*test_rules.py* makes use of *cavalieri_tester*, a binary that is capable of
loading your rules and send events to them.

However, it does so in an special environment,  where all the external calls
such as email or pagerduty are mocked.  It also mocks the scheduler, that means
you can test months worth of events in just a few seconds.

This feature allows you to easily add your alert rules to your
continous integration process.


Sending events
--------------

You can use any of the existing [riemann.io](http://riemann.io/clients.html)
clients. Just make them send events to a host running *cavalieri*.

Streams API
------------

### Stream functions

#### What is a stream function?

#### prn()

It prints events that pass through it.

#### prn (const std::string  str)

It prints events that pass through it and also the string that takes as an argument.

#### service (const std::string service)

It forwards events that contain the given service.

#### service_any (const std::vector&lt;std::string> services)

It forwards events that contain any of the given services. This
behaves just like *service* but it takes a list of services instead of
a single one.


#### service_like (const std::string pattern)

It forards events which services match the given pattern.

```cpp
service_like("foo%") >> prn("service starting with foo");
```

#### service_like_any (const std::vector&lt;std::string> patterns)

It forwards events which services match any of the given pattern. This
behaves just like *service_like* but it takes a list of patterns instead of
a single one.

#### set_state (const std::string state)

It sets the events state to *state* and forwards them.


#### set_metric (const double value);

It sets the events metric to *value* and forwards them.


#### with (const with_changes_t & changes)

Modifies the event. It takes a map that contains the keys to be modified
and their corresponding new value.

```cpp
// Change host field and description
with({{"host", "cluster-001"}, {"description", "aggregated master metrics"});
```
#### default_to (const with_changes_t & changes)

It takes a map that contains key-value pairs to be added to the event, but only
in case the key is not set in the event already.


```cpp
// Default ttl to 120. Only events with the ttl field not set are modified.
default_to({"ttl", 120});
```

#### split (const split_clauses_t clauses)

It takes a list of pairs. Each pair contains a predicate function and a stream.
When an event is received, the event is passed to the first stream which
predicate returns true.

```cpp
split({above_pred(10), set_state("ok")},
      {under_pred(5),  set_state("critical"});
```

#### split (const split_clauses_t clauses, const streams_t default_stream)

It takes a list of pairs and a default stream. Each pair contains a predicate
function and a stream.  When an event is received, the event is passed to the
first stream which predicate returns true. If none of the predicates match,
the event is passed to the default stream.

```cpp
split({above_pred(10), set_state("ok")},
      {under_pred(5),  set_state("critical")},
      set_state("warning"));
```

#### where (const predicate_t & predicate)

It passes events that make the predicate function return true.

```cpp
where(under_pred(5)) >> set_state("critical") >> notiy_email();
```

#### where (const predicate_t & predicate, const streams_t else_stream)

It passes events that make the predicate function return true.
Otherwise, events are passed to *else_stream*.

```cpp
above_stream = set_state("ok") >> prn("metric is above 5");

where(under_pred(5), above_stream) >> set_state("critical") >> notiy_email(); 
```

#### by (const by_keys_t  & keys, const streams_t stream)

It takes a list of event's fields. When an event enters this function,
the field(s) are retrieved, for every new value that has not been seen before,
it will create a copy of *stream* and the event will be passed to it.
If the value was seen before, it will pass the event to the previously
created stream.

Let's see this in action. We are going to use a stream function called *rate*
which simply sums the event metrics that receives during *dt* seconds and
divides the result by *dt*. Let's assume our servers send an event called
*backend_exception* every time a request can't be handled and we would
like to see the exception rate per server.

Note that if we just do what is below, we wouldn't get a per host rate,
we would get a global rate.

```cpp
auto rate_stream = set_metric(1) >> rate(60) >> prn("exceptions per second:");
```

If we want to compute the rate per host, that's when *by()* comes in handy.
It helps us  replicate the stream per each host so we can compute the rates
individually.

```cpp
auto rate_stream = set_metric(1)
                      >> rate(60)
                        >> prn("exceptions per second:");

// Use the host field and replicate rate_stream for evey distinct host.
by({"host"}, rate_stream);
```

You can pass several fields to *by()*.

#### by (const by_keys_t  & keys)

This is similar to *by(const by keys_t & keys, const streams_t streams)*. But
instead of passing the streams to clone for every distinct combinations of
*keys* as a parameter, it will duplicate the streams that are concatenated
after it.

Let's see the example of the other *by()* function using this one.

```cpp
// Use the host field and replicate the stream that is next to it.
by({"host"}) >> set_metric(1) >> rate(60) >> prn("exceptions per second:");
```


#### rate (const uint32 & dt)

It sums the metrics of the received events for *dt* seconds. After that period,
an event is forwarded and its metric contains the accumulated value divided
by *dt*.


```cpp
// An easy way to count the rate of events that go through this stream
with({"metric", 1) >> rate(60) >> prn("events per second");
```

#### coalesce (const fold_fn_t & fold_fn)

It keeps a map with the receivied events. Events are inserted in the map by
using the combination of ttheir host and service as a key.

Every time a new event is received, the map is updated and all the events in it
are forwarded to the fold function.

This function is useful to aggregate metrics from different hosts.


#### project (const predicates_t predicates, const fold_fn_t & fold_fn)

Similar to *coalesce* and more suitable when you just need a few events.

It takes a list of predicates.  For every predicate, the last event that
matches is stored. Whenever a new event arrives and matches
any of the predicates, all the stored events are forwared to *fold_fn*.


```cpp
// Create a new event metric that is the sum of foo and bar
project({service_pred("foo"), service_pred("bar"), sum) >> prn("foo + bar");
```

#### changed_state (const std::string & initial)

It only forwards events if there is a state change for every host and service.
It assummes *initial* as the first state.

If you are sending emails, this is useful to not spam yourself and only send
emails when something goes from *ok* to *critical* and viceversa.

#### tagged_any (const tags_t & tags)

It forwards events only if they contain any of the given *tags*.

```cpp
tagged_any({"debian", "ubuntu"}) >> above(5) >> email();
```

#### tagged_all (const tags_t & tags)

It forwards events only if they contain all the given *tags*.

```cpp
tagged_any({"production", "london"}) >> above(5) >> email();
```

#### tagged (const std::string tag)

It forwards events only if they contain the given *tag*.

```cpp
tagged("production") >> above(5) >> email();
```

#### smap (const smap_fn_t fn)

Events are recevied and passed to *fn* which returns a new event.
This new event is forwarded.

Use this when you need to modify events dynamically, as in opposed to
statically, use *with()* for the latter.

```cpp
// Function that takes and event and returns a new event which service
// has the host appended.
Event host_service(e_t e)
{
  auto ne(e);
  ne.set_service(e.service() + "-" + e.host());
  return ne;
};

smap(host_service) >> prn("new shiny service string");
```

#### moving_event_window (const size_t n, const fold_fn_t fn)

Every time an event is received, the last *n* events are passed to *fn* which
returns a new event that is forwarded.


#### fixed_event_window (const size_t n, const fold_fn_t fn)

It passes non-overlapping windows of *n* events to *fn* which returns
a new event that is forwarded.

#### moving_event_window (const size_t dt, const fold_fn_t fn)

Every time an event is received, the last events within a *dt* window are
passed to *fn* which returns a new event that is forwarded.

#### fixed_time_window (const size_t dt, const fold_fn_t fn)

It passes non-overlapping windows of the events received within a *dt* window
to *fn* which returns a new event that is forwarded.

#### stable (const time_t dt)

It forwards events only when their state is the same for *dt* seconds.
This is useful to avoid spikes.

#### throttle (size_t n, time_t dt)

It only forwards a maximum of *n* events during *dt* seconds.

#### above (double k)

It forwards events with metrics above *k*.

#### under (double k)

It forwards events with metrics under *k*.

#### within (double a, double b)

It forwards events with metrics between *a* and *b*.

#### without (double a, double b)

It forwards events with metrics not in the  (*a*, *b*) interval.

#### scale (double s)

It scales events' metric by  *s* and forwards them.


```cpp
// Transform bytes in bits
service("eth0_incoming") >> scale(8);
```

#### sdo ()

It just forwards events.

#### counter ()

It counts the number of events that pass through it.

#### expired ()

It forwards events that are expired.

#### tags (tags_t tags)

It adds the list of passed *tags* to events and forwards them.

```cpp
tags({"processed"}) >> prn("tag added")
```

#### ddt ()

It differenciates two subsequent events. The metric of the forwarded
event is (metric_current - metric_previous) / (time_current - time_previous).

#### send_index()

It indexes the receivied events. Indexed events can be queried through
the websocket.

This is usful to know the current state of an event from a dashboard.

#### send_graphite(const std::string host, const int port)

It forwards the received events to a graphite server using new line carbon
TCP protocol.

This is an *external* function, meaning that cavalieri will talk to an external
service, in this case, a graphite server.

#### forward(const std::string host, const int port)

It forwards the received events to a cavalieri or riemann server using
TCP.

This is an *external* function, meaning that cavalieri will talk to an external
service, in this case, a cavalieri or riemann server.

#### email(const std::string server, const std::string from, const std::string to)

It emails the received events with *from* sender to *to* recipient using
the specified SMTP *server*.

This is an *external* function, meaning that cavalieri will talk to an external
service, in this case, an SMTP server.



#### pagerduty_trigger(const std::string pd_key)

It triggers a Pager Duty incident based on the received event and using
*pd_key* as the API key. Note that service key will be event's host and
service.

This is an *external* function, meaning that cavalieri will talk to an external
service, in this case, Pager Duty.

#### pagerduty_acknowoledge(const std::string pd_key)

It acknowledges a Pager Duty incident based on the received event and using
*pd_key* as the API key. Note that service key will be event's host and
service.

This is an *external* function, meaning that cavalieri will talk to an external
service, in this case, Pager Duty.

#### pagerduty_resolve(const std::string pd_key)

It resolves a Pager Duty incident based on the received event and using
*pd_key* as the API key. Note that service key will be event's host and
service.

This is an *external* function, meaning that cavalieri will talk to an external
service, in this case, Pager Duty.


### Fold functions

Fold functions are functions that take a list of events, do some processing
with them such as reducing and return an event with the result.

These functions are mostly meant to be used with stream functions that forward
a list of events.

#### sum(const std::vector<Event> events)

It returns an event that contains the sum of the metrics of *events*.

#### product(const std::vector<Event> events)

It returns an event that contains the product of the metrics of *events*.

#### difference(const std::vector<Event> events)

It returns an event that contains the difference of the metrics of *events*.

#### mean(const std::vector<Event> events)

It returns an event that contains the mean of the metrics of *events*.

#### minimum(const std::vector<Event> events)

It returns an event that contains the minimum value of the metrics of *events*.

#### maximum(const std::vector<Event> events)

It returns an event that contains the maximum of the metrics of *events*.

### Common Rules

These rules are based on the above stream functions, but they are more
high-level and more opinionated.

They asumme that two states *critical* and *ok* are enough. Events coming
out from these function have their state set to any of them.

#### critical_above (double value)

It sets state to critical if metric is above *value*. Otherwise, it sets it to
ok.

#### critical_under (double value)

It sets state to ok if metric is under *value*. Otherwise, it sets it to
critical.

#### stable_metric (double dt, predicate_t trigger)

It takes *trigger* as a function predicate to check events. It sets the state
to critical when *trigger* has returned *true* for more than *dt* seconds.

It sets it back to critical when *trigger* has returned *false* for more than
*dt* seconds.

This is useful to avoid spikes.

```cpp
stable_metric( /* seconds */ 300, above_pred(200))
  >> changed_state("ok")
    >>  email();
```

#### stable_metric (double dt, predicate_t trigger, predicate_t cancel)

Similar to the above function but taking an extra predicate *cancel* that is
used as a threshold to set it back to ok.


It sets the state to critical when *trigger* has return *true* for more than
*dt* seconds.

It sets it back to critical when *cancel* has returned *true* for more than
*dt* seconds.

This is useful to avoid spikes.

```cpp
stable_metric( /* seconds */ 300, above_pred(200))
  >> changed_state("ok")
    >>  email();
```


#### agg_stable_metric (double dt, fold_fn_t fold_fn, predicate_t trigger, predicate_t cancel)

This function aggregates metrics of events that are received using
*fold_fn* (See fold functions). The event that results is passed to a
*stable_metric* stream using *dt*, *trigger* and *cancel*.

Let's see an example. Say we have a bunch of web servers in our London data
center. Those servers are reporting a metric called *failed_requests_rate*. We
would like to create another metric that is the aggregated sum of all the
servers and trigger an alert when that value is above a given value for
more than *dt* seconds.

```cpp
service("failed_requests_rate")
  >> tagged("datacenter::london")
    >> agg_stable_metric(/* secs */ 300, sum, above_pred(200), under_pred(50))
      >> changed_state("ok")
        >> email();
```


#### max_critical_hosts(size_t n)

This function sets the state of the events to critical when it receives
more than *n* different critical events.

In the example below, we trigger an alert when more than 20 servers
report a puppet failure in a DC.

```cpp
service("puppet")
  >> tagged("datacenter::paris")
    >> max_critial_hosts(20)
      >> changed_state("ok")
        >> set_host("datacenter::paris")
          >> set_service("too many puppet failures")
            >> email();
```


### Predicate functions

### Some utility functions

### Some typedefs

Dashboard
---------

You can use the standard [riemann.io dahsboard](http://riemann.io/dashboard.html)
to query and visualize the state of the index.

