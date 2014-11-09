Cavalieri [![Build Status](https://drone.io/github.com/juruen/cavalieri/status.png)](https://drone.io/github.com/juruen/cavalieri/latest)
=========


Introduction
------------

*Cavalieri* is a C++ event stream processing tool to monitor distrubuted
systems, it is inspired by the awesome [riemann.io](http://riemann.io) project.

It implements the original [riemann.io](http://riemann.io) protocol. That means
you can leverage the existing *riemann* clients and tools. It also tries to
mimic its stream API where possible.

Cavalieri's current version *0.1.2* is considered to be  in **beta** state.

Background
----------

We use Riemann at my $DAILY_WORK. I started this project to have a better
understanding of how Riemann works internally.

I kept working on it, and it reached a point where it became useful.

It's being tested by running side by side with our Riemann servers and seems
to be stable -at least with the subset of features that are being used- to
monitoring thousands of hosts.

Next steps are refactoring and cleaning up before adding more features.


Content
-------

* [Install](#install)
* [Create your rules](#create-your-rules)
* [Test your rules](#test-your-rules)
* [Sending events](#sending-events)
* [Stream functions](#streams-functions)
* [Fold functions](#fold-functions)
* [Predicate functions](#predicate-functions)
* [Common rules](#common-rules)
* [Event class](#event-class)
* [Dashboard](#Dashboard)
* [Cavalieri HOW-TO](https://github.com/juruen/cavalieri/blob/master/HOWTO.md)

Install
-------

#### Ubuntu packages

You can install a deb package for Ubuntry 14.04 (Trusty) by adding this ppa:

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
libgoogle-glog-dev, libcurl4-openssl-dev, libssl-dev, libtbb-dev,
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

streams_t cavalieri_rules() {

  auto mail_stream = email("localhost", "cavalieri@localhost",
                           "devops@localhost");

  return service("requests_rate")
           >> above(40)
             >> set_state("critical")
               >> changed_state()
                 >>  mail_stream;

}

EXPORT_RULES(cavalieri_rules)
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
               >> changed_state()
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

Reload rules
------------

When Cavalieri receives a SIGHUP signal, it will only reload the *.so* rule
libraries that have changed.

This allows deployments where several teams push rules, to only reload and
affect the namespaces that have changed.

This is something that Riemann doesn't allow and might be especially useful
when multiple teams mantain rules.

Sending events
--------------

You can use any of the existing [riemann.io](http://riemann.io/clients.html)
clients. Just make them send events to a host running *cavalieri*.

Streams API
------------

### Stream functions

#### What is a stream function?

#### prn ()

It prints events that pass through it.

#### prn (const std::string  str)

It prints events that pass through it and also the string that takes as an argument.

#### null ()

This can be used a as a sink that doesn't forward events.

#### service (const std::string service)

It forwards events that contain the given service.

#### service_any (const std::vector&lt;std::string> services)

It forwards events that contain any of the given services. This
behaves just like *service* but it takes a list of services instead of
a single one.


#### service_like (const std::string pattern)

It forwards events whose services match the given pattern.

```cpp
service_like("foo%") >> prn("service starting with foo");
```

#### service_like_any (const std::vector&lt;std::string> patterns)

It forwards events whose services match any of the given pattern. This
behaves just like *service_like* but it takes a list of patterns instead of
a single one.

#### state (const std::string state)

It forwards events whose state is set to *state*.

#### state_any (const std::vector&lt;std::string> states)

It forwards events whose state is any of *states*.

#### has_attribute (const std::string attribute)

It fowards events that have a set *attribute*.

#### set_service (const std::string service)

It sets the events service to *service* and forwards them.

#### set_state (const std::string state)

It sets the events state to *state* and forwards them.

#### set_host (const std::string host)

It sets the events host to *host* and forwards them.

#### set_metric (const double value);

It sets the events metric to *value* and forwards them.

#### set_description (const std::string description);

It sets the events metric to *description* and forwards them.

#### set_ttl (const double ttl);

It sets the events TTL to *tll* and forwards them.

#### default_service (const std::string service)

If service is not set, it sets the events service to *service*
and forwards them.

#### default_state (const std::string state)

If state is not set, it sets the events state to *state* and forwards them.

#### default_host (const std::string host)

If host is not set, it sets the events host to *host* and forwards them.

#### default_metric (const double value);

If metric is not set, it sets the events metric to *value* and forwards them.

#### default_description (const std::string description);

If description is not set, it sets the events metric to *description* and
forwards them.

#### default_ttl (const double ttl);

If TTL is not set, it sets the events TTL to *tll* and forwards them.


#### WITH(EXP)

Use this macro as a way to create a stream that modifies any of the event's
fields.

It defines *e* as an event within its scope. You can call mutable functions on
it. It then takes care of forwarding the modified event.

This is the actual macro:

```cpp
#define WITH(EXP)\
  create_stream(
      [](const Event & const_event)
      {
        Event e(const_event);

        (EXP);

        return {e};
      })
```

You can use it as follows:

```cpp
// Change host field and description
WITH(e.set_host("cluster-001").set_description("aggregated master metrics"));
```

#### split (const split_clauses_t clauses)

It takes a list of pairs. Each pair contains a predicate function and a stream.
When an event is received, the event is passed to the first stream which
predicate returns true.

You can see this function as a *switch case* statement where predicates are
the *cases*.

```cpp
split({{p::above(10), set_state("ok")},
       {p::under(5),  set_state("critical"}});
```

#### split (const split_clauses_t clauses, const streams_t default_stream)

It takes a list of pairs and a default stream. Each pair contains a predicate
function and a stream.  When an event is received, the event is passed to the
first stream which predicate returns true. If none of the predicates match,
the event is passed to the default stream.

You can see this function as a *switch case* statement where predicates are
the *cases*.

```cpp
split({{p::above(10), set_state("ok")},
       {p::under(5),  set_state("critical")}},
      set_state("warning"));
```

When an event enters the split function that we defined above, three different
scenarios can happen.

First scenario: the *p::above(10)* predicate returns *true*, and hence the
event is forwarded to *set_state("ok")*.

![Split-1](https://github.com/juruen/cavalieri/blob/master/docs/images/split-1.png)

Second scenario: metric is not above 10, but it is under 5 and then
*p::under(5)* returns *true*. The event is then forwarded to
*set_state("critical")*.


![Split-2](https://github.com/juruen/cavalieri/blob/master/docs/images/split-2.png)

Third and last scenario: metric is between 5 and 10. Neither *p::above(10)* or
*p::under(5)* return *true*. The event is then sent to the default stream. In
this case *set_state("warning")*.

![Split-3](https://github.com/juruen/cavalieri/blob/master/docs/images/split-3.png)

Note that in the three scenarios, the result of going through any of the
streams will be forwaded to any stream that is after split. This means that
the code below prints the event with the state that split sets.

```cpp
split({p::above(10), set_state("ok")},
      {p::under(5),  set_state("critical")},
      set_state("warning")) >> prn ("result after going through split: ")
```


#### where (const predicate_t & predicate)

It passes events that make the predicate function return true.

```cpp
where(p::under(5)) >> set_state("critical") >> notiy_email();
```

#### where (const predicate_t & predicate, const streams_t else_stream)

It passes events that make the predicate function return true.
Otherwise, events are passed to *else_stream*.

```cpp
above_stream = set_state("ok") >> prn("metric is above 5");

where(p::under(5), above_stream) >> set_state("critical") >> notiy_email(); 
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
project({p::serviced("foo"), p::service("bar"), sum) >> prn("foo + bar");
```

#### changed_state (const std::string & initial)

It only forwards events if there is a state change for a host and service.
It assummes *initial* as the first state. It uses *by({"host", "service"})*
internally.

If you are sending emails, this is useful to not spam yourself and only send
emails when something goes from *ok* to *critical* and viceversa.

#### changed_state ()

It only forwards events if there is a state change for a host and service.
It assummes *ok* as the first state. It uses *by({"host", "service"})*
internally.

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

Events are recevied and passed to *fn* as a mutable reference. You
are free to modify the received event as you wish.

The function below changes appends the host name to the service.

```cpp
void host_service(Event & e)
{
  e.set_service(e.service() + "-" + e.host());
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

#### percentiles (time_t interval, const std::vector<double> percentiles)

It creates a reservoir that represents a distribution of the metrics received.
Every *interval* seconds, it will emit a list of events with the given
*percentiles*.

The corresponding percentile will be added to the service name of the emitted
events.

```cpp
// This will create a distrbution of the request_time metrics, and every
// 2 seconds it will emit events containing percentils: 0th, 50th, 90th, 95th
// and 100th
service("request_time") >> percentiles(2, {0.0, 0.5, 0.90, 0.95, 1});
```



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

#### not_expired ()

It forwards events that are not expired.

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

#### count(const std::vector<Event> events)

It returns an event that contains the number of received *events*.

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
stable_metric( /* seconds */ 300, p::above(200))
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
stable_metric( /* seconds */ 300, p::above(200))
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
    >> agg_stable_metric(/* secs */ 300, sum, p::above(200), p::under(50))
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
      >> changed_state()
        >> set_host("datacenter::paris")
          >> set_service("too many puppet failures")
            >> email();
```


### Predicate functions

These functions are used to filter events. Its main purpose is to evaluate
events and return *true* or *false* based on what is being checked.

There are two types of predicate functions. Those returning *predicate_t*,
which can be used as arguments for *where()* and *split*. And those returning
*bool*, which can be returned to build your own *predicate_t* functions, or
within your own stream functions.

#### predicate_t above_eq (const double value)

Check if the event metric is greater than or equal  to *value*.

#### predicate_t above(const double value)

Check if the event metric is greater than *value*.

#### predicate_t under_eq(const double value)

Check if the event metric is less than or equal  to *value*.

#### predicate_t under(const double value)

Check if the event metric is less than *value*.

#### predicate_t state(const std::string state)

Check if the event state is equal to *state*.

#### predicate_t service(const std::string service)

Check if the event service is equal to *service*.

#### predicate_t match(const std::string key, const std::string value)

Check if the field *key* in the event is equal to *value*.

#### predicate_t match_any(const std::string key, const std::vector<std::string> values)

Check if the field *key* in the event is equal to any of the *values*.

#### predicate_t match_re(const std::string key, const std::string regex)

Check if the field *key* in the event matches the regular expression in
*regex*.

*regex* is a string containing a valid
[ECMAScript](http://www.cplusplus.com/reference/regex/ECMAScript/) regular
expression.

#### predicate_t match_re_any(const std::string key, const std::vector<std::string> regexes)

Check if the field *key* in the event matches any of the regular expression in
*regexes*.

#### predicate_t match_like(const std::string key, const std::string like)

Check if the field *key* in the event matches a *SQL like* string that uses
'%' to search for patterns.

#### predicate_t match_like_any(const std::string key, const std::vector<std::string> likes)

Check if the field *key* in the event matches any of the  *SQL like* strings.

#### predicate_t default_true()

This function always returns true.

#### bool tagged_any(e_t event, const tags_t& tags)

Check if any of the *tags* is present in *event*.

#### bool tagged_all(e_t event, const tags_t& tags)

Check if all *tags* are present in *event*.

#### bool expired(e_t event)

Check if *event* is expired.

#### bool above_eq(e_t event, const double value)

Check if metric in *event* is greater than or equal *value*.

#### bool above(e_t event, const double value)

Check if metric in *event* is greater than *value*.

#### bool under_eq(e_t event, const double value)

Check if metric in *event* is less than or equal to *value*.

#### bool under(e_t event, const double value)

Check if metric in*event* is less than *value*.

#### bool match(e_t event, const std::string key, const std::string value)

Check if the field *key* in *event* is equal to *value*.

#### bool match_re(e_t event, const std::string key, const std::string regex)

Check if the field *key* in *event* matches the *regex*.

#### bool match_like(e_t event, const std::string key, const std::string like)

Check if the field *key* in *event* matches the *SQL like* string that uses
'%' to search for patterns.

### Event class

This is the *Event* class that is used widely all over Cavalieri. It's
basically a wrapper to the generataed [Riemann
protobuf](https://github.com/juruen/cavalieri/blob/master/src/proto.proto)
class that adds some handy functions.

Here is the list of function members in
[this](https://github.com/juruen/cavalieri/blob/master/include/common/event.h)
class:

-   Event()

-   Event(const riemann::Event & event)

-   Event(const Event &) = default

-   Event copy() const

-   riemann::Event riemann_event() const

-   std::string host() const

-   Event & set_host(const std::string host)

-   bool has_host() const

-   Event & clear_host()

-   std::string service() const

-   Event & set_service(const std::string service)

-   bool has_service() const

-   Event & clear_service()

-   std::string state() const

-   Event & set_state(const std::string state)

-   bool has_state() const

-   Event & clear_state()

-   std::string description() const

-   Event & set_description(const std::string description)

-   bool has_description() const

-   Event & clear_description()

-   int64_t time() const

-   Event & set_time(const int64_t time)

-   bool has_time() const

-   Event & clear_time()

-   float ttl() const

-   Event & set_ttl(const float ttl)

-   bool has_ttl() const

-   Event & clear_ttl()

-   double metric() const

-   Event & set_metric(const double metric)

-   bool has_metric() const

-   Event & clear_metric()

-   std::string metric_to_str() const

-   float metric_f() const

-   Event & set_metric_f(const float metric)

-   bool has_metric_f() const

-   Event & clear_metric_f()

-   float metric_d() const

-   Event & set_metric_d(const double metric)

-   bool has_metric_d() const

-   Event & clear_metric_d()

-   int64_t metric_sint64() const

-   Event & set_metric_sint64(const int64_t metric)

-   bool has_metric_sint64() const

-   Event & clear_metric_sint64()

-   std::string value_to_str(const std::string field) const

-   bool has_field_set(const std::string field) const

-   std::string json_str() const

-   bool has_tag(const std::string tag) const

-   Event & add_tag(const std::string tag)

-   Event & clear_tags()

-   bool has_attr(const std::string attribute) const

-   std::string attr(const std::string attribute) const

-   Event & set_attr(const std::string attribute, const std::string value)

-   Event & clear_attrs()

### Some typedefs

Dashboard
---------

You can use the standard [riemann.io dahsboard](http://riemann.io/dashboard.html)
to query and visualize the state of the index.
