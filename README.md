Cavalieri [![Build Status](https://drone.io/github.com/juruen/cavalieri/status.png)](https://drone.io/github.com/juruen/cavalieri/latest)
=========


Introduction
------------

Install
-------

Create your rules
-----------------

Clone *cavalieri-rules*, a template project to create your own rules.

```sh
git clone https://github.com/juruen/cavalieri-rules.git
cd cavalieri-rules
```

Open *rules.cpp* and add your rules. The default rule will send an email
with a critical event when a metric from the *requests_rate*
service is above 40.

```cpp
#include <rules.h>
#include <external/pagerduty.h>
#include <external/email.h>

streams_t* rules() {

  auto mail_stream = email("localhost", "cavalieri@localhost",
                         "devops@localhost");

  auto s =  where(service_pred("requests_rate"))
               >> above(40)
                 >> with({{"state", "critical"}})
                   >> changed_state("ok")
                     >>  mail_stream;

  return new streams_t(s);
}
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
I0403 23:15:44.006975 13082 config.cpp:36] config:
I0403 23:15:44.007417 13082 config.cpp:37]      events_port: 5555
I0403 23:15:44.007490 13082 config.cpp:38]      rimeann_tcp_pool_size:: 1
I0403 23:15:44.007547 13082 config.cpp:39]      ws_port: 5556
I0403 23:15:44.007622 13082 config.cpp:40]      ws_pool_size: 1
I0403 23:15:44.007675 13082 config.cpp:41]      index_expire_interval: 60
I0403 23:15:44.007763 13082 config.cpp:42]      rules_directory: .
I0403 23:15:44.007843 13082 config.cpp:43] --
I0403 23:15:44.012259 13082 rules_loader.cpp:60] rules loaded succesfully from librules.so

```

Test your rules
---------------

You can easily test your rules without putting them in production.
In the root directory of *cavalieri-template* there is a Python script that will
help you to find out if your rules are doing what you expect.

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

auto s =  where(service_pred("requests_rate"))
           >> above(40)
             >> with({{"state", "critical"}})
               >> changed_state("ok")
                 >>  mail_stream;

[...]

```

As you can see, the events defined in *test_rules.py* will trigger an alert
when the metric is above 40.  This happens with the event sent at time *180*.

You can execute *test_rules.py* from the root directory of
*cavalieri-template*. And you will magically see what happens to your
rules when those events are sent:

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

As you can see in the above output, *test_rules.py* is reporting that an email would have been send at time *180*
to report that the service *requests_rate* is critical.

*test_rules.py* makes use of *cavalieri_tester*, a binary that is capable of loading your rules and send events to them.
However, it does so in an special environment where all the external calls such as email or pagerduty are mocked.
It also mocks the scheduler, so you can test months of events in just a few seconds.

This feature allows you to easily add your alert rules to your continous integration process.


Sending events
--------------

Streams API
------------

### Stream functions

#### What is a stream function?

#### prn()

Prints events that pass through it.

#### prn(const std::string & str)

Prints events that pass through it and also the string that takes as an argument.

#### with (const with_changes_t & changes)

Modifies the event. It takes a map that contains the keys to be modified and their corresponding new value.

```cpp
// Change host field and description
with({{"host", "cluster-001"}, {"description", "aggregated master metrics"});
```
#### default_to (const with_changes_t & changes)

It takes a map that contains the keys to be added to the event if the key is not set.

```cpp
// Default ttl to 120
default_to({"ttl", 120});
```

#### split (const split_clauses_t clauses)

It takes a list of pairs. Each pair contains a predicate function and a stream.
When an event is received, the event is passed to the first stream which predicate returns true.

```cpp
split({above_pred(10), set_state("ok")},
      {under_pred(5),  set_state("critical"});
```

#### split (const split_clauses_t clauses, const streams_t default_stream)

It takes a list of pairs and a default stream. Each pair contains a predicate function and a stream.
When an event is received, the event is passed to the first stream which predicate returns true. If
none of the predicates match, the event is passed to the default stream.

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

It passes events that make the predicate function return true. Otherwise, events are passed to *else_stream*.

```cpp
above_stream = set_state("ok") >> prn("metric is above 5");

where(under_pred(5), above_stream) >> set_state("critical") >> notiy_email(); 
```

#### by (const by_keys_t  & keys, const by_stream_t stream)

It takes a list of event's fields. When an event enters this function, the field(s) is retrieved, for every
new value that has not been seen before, it will create a copy of  *stream* and the event will be passed
to it. If the value was seen before, it will pass the event to the previously created stream.

Let's see an example. We are going to use a stream function called *rate* which simply sums the event
metrics that receives during *dt* seconds and divides the result by *dt*. Let's assume our servers
send an event called *backend_exception* everytime a request can't be handled and we would
like to see the exception rate per server.

Note that if we just do what is below, we wouldn't get a per host rate, we would get a global rate.

```cpp
auto rate_stream = with({"metric", 1}) >> rate(60) >> prn("exceptions per second:");
```

If we want to compute the rate per host, that's when *by()* comes in handy. It helps us to replicate the
stream per each host so we can compute the rates individually.

```cpp
auto rate_stream = BY(with({"metric", 1}) >> rate(60) >> prn("exceptions per second:"));

// Use the field host and replicate rate_stream for evey distinct host.
by({"host"}, rate_stream);
```

Note that we need to wrap the stream function that we want to replicate with the *BY()* macro.

You can pass several fields to *by()*.

#### rate (const uint32 & dt)

It sums the metrics of the received events for *dt* seconds. After that period, an event is forwarded where
its metric contains the accumulated value divided by *dt*.


```cpp
// An easy way to count the rate of events that go through this stream
with({"metric", 1) >> rate(60) >> prn("events per second");
```

#### coalesce (const fold_fn_t & fold_fn)

It keeps a map with the receivied events. Events are inserted in the map by using their host and service as a key.
Every time a new event is received, the map is updated and all the events in it are forwarded to the fold function.

This function is useful to aggregate metrics from different hosts.


#### project (const predicates_t predicates, const fold_fn_t & fold_fn)

Similar to *coalesce* and more suitable when you just need a few events. It takes a list of predicates.
For every predicate, the last event that matches is stored. Whenever a new event arrives and matches
any of the predicates, all the stored events are forwared to *fold_fn*.


```cpp
// Create a new event metric that is the sum of foo and bar
project({service_pred("foo"), service_pred("bar"), sum) >> prn("foo + bar");
```

#### changed_state (const std::string & initial)

It only forward events if there is a state change. It assummes *initial* as the first state.

If you are sending emails, this is useful to not spam yourself and only send emails when something goes
from *ok* to *critical* and viceversa.

#### tagged_any (const tags_t & tags)

It forwards events only if they contain any of the given *tags*.

```cpp
tagged_any({"debian", "ubuntu"}) >> where(above_pred(5)) >> email();
```

#### tagged_all (const tags_t & tags)

It forwards events only if they contain all the given *tags*.

```cpp
tagged_any({"production", "london"}) >> where(above_pred(5)) >> email();
```

#### smap (const smap_fn_t fn)

Events are recevied and passed to *fn* which returns a new event. This new event is forwarded. Use this
when you need to modify events dynamically, as in opposed to statically, that you can do using *with()*.


```cpp
  // Function that takes and event and returns a new event which service has the host appended
Event host_service(e_t e)
{
  auto ne(e);
  ne.set_service(e.service() + "-" + e.host());
  return ne;
};

smap(host_service) >> prn("new shiny service string");
```

#### moving_event_window (const size_t n, const fold_fn_t fn)

Every time an event is received, the last *n* events are passed to *fn* which returns a new event
that is forwarded.


#### fixed_event_window (const size_t n, const fold_fn_t fn)

It passes non-overlapping windows of *n* events to *fn* which returns a new event that is forward.

#### moving_event_window (const size_t dt, const fold_fn_t fn)

Every time an event is received, the last events within a *dt* window are passed to *fn* which returns a new event
that is forwarded.

#### fixed_time_window (const size_t dt, const fold_fn_t fn)

It passes non-overlapping windows of the events received within a *dt* window to *fn* 
which returns a new event that is forward.

#### stable (const time_t dt)

It forwards events only when their state is the same for *dt* seconds. This is useful to avoid spikes.

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
where(service_pred("eth0_incoming")) >> scale(8);
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




### Fold functions

### Predicate functions

### Some utility functions

### Some typedefs

Dashboard
---------
