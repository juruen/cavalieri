Cavalieri How-To
================

Debugging
---------

You can add print functions to your streams to see what is happening at
a given point.

This is useful to debug your rules and figure out why something is not doing
what you expect.

*prn()* will print the events that pass through it. You can also use
*prn("something seems wrong here")* to print an arbitrary string before
the event.

```cpp

service("eth0_in") >> prn("before scale") >> scale(8) >> prn("after scale")

```

If this is not enough, you can always create your own stream function that
logs information in the way you need.

Instrumenting your system
-------------------------

#### Internal Cavallieri metrics

Cavalieri reports internal metrics that are useful to measure its health and
performance.

They are tagged with *cavalieri::internals*. You can fire up your Riemann
dashboard and use the query *tagged = "cavalieri::internals"* to see how
Cavalieri is doing in real time.

You will find intersting metrics such as the rate of events being processed
along with its latency distribution. Number of connections, queues and
memory consumption.

You can enable or disable these internal metrics in your configuration.

#### Measure CPU, memory and disk usage

You can use any of the fantastic Riemann clients that exist to monitor these
metrics.

[Riemann-tools](https://github.com/aphyr/riemann-tools) are a great for this.

#### Custom event attributes

The Riemann protocol allows clients to send arbitrary key-value pairs. This can
be used to extend the information that is encoded in the events when its
regular fields are not enough.

See this snippet for a Ruby client sending a custom event attribute.

```ruby
client << {service: "thumbnailer rate",
           metric:  5.0,
           build:   "7543"}
```
*build* is the custom attributes. Custom attributes are restricted by the wire
protocol to be strings.

Let's now use this attribute in a stream.

But first, we create some helper functions. We start with a function to
compute a per-build rate.

```cpp

auto per_build_rate = by({"build"}) >> rate(5);

```

We continue with a stream that appends the build version to the service name
of the event.

```cpp

auto append_build = create_stream(
    [](e_t e)
    {
        auto ne(e);
        ne.set_service(ne.service() + "-" attribute_value(e, "build"));
        return {ne};
    });

```

The next step is a predicate function to match build versions that are
less than *1055*. Please note that attributes are strings and we need to
cast the value first. If events match that version check, we scale its building
number.


```cpp

auto match_version   = PRED(std::stoi(attribute_value(e, "build")) < 1055);

auto scale_old_build = where(match_version, send_index()) >> scale(0.5);

```

We have everything we need. Our final stream looks like this:

```cpp

service("thumbnailer rate")
  >> has_attribute("build")
    >> append_build
      >> sdo(per_build_rate, scale_old_build)
        >> send_index();

```

Working with streams
--------------------

#### Multiple streams

Cavalieri streams are functions that take an event and may return a list of
new events, an empty list of events, or a list containing just the same event.

These functions take a *const Event &*, normally abbreviated with *e_t*. Note
that it's a const reference. That means that if you need to modify the event,
you need to create a copy. Passing constant references helps a lot with
concurrency.

Most of the functions build a linear stream, where events are passed from one
stream to another. However,  some functions help you to
send events that come out from one function to multiple streams.

The simplest function that allows you to do this is *sdo()*. It takes a list
of streams, and upon receiving an event, this event is sent to all of its
streams.

The output of the streams is send to the stream function after *sdo()*. This
effectively allows you to combine multiple streams in one.

```cpp

auto production = tagged("production") >> above(10) >> set_state("critical");
auto testing = tagged("testing") >> above(5) >> set_state("warning");
auto experimental = tagged("experimental") >> above(5) >> set_state("warning");

/* Events with a "foo" service are sent to production, testing and experimental.
   The result of going through them is sent to prn(), but only for production
   and testing. experimental is followed by null() which is used as a sink.
*/
service("foo") >> sdo(production, testing, experimental >> null()) >> prn()

```

If you don't want any of the streams in *sdo()* to forward their events, you
can add *null()* to them.


*split()*, just like *sdo()* can be used to send events to multiple streams.
And like *sdo()* the output of the streams is forwarded to the stream function
after *split()*.

#### Distinct streams for each host, service, etc.

A common use case is to replicate a stream for a all hosts or services.

When you see yourself writing rules like these:

```cpp

auto service_foo1 = service("foo1")
                      >> coalesce(sum)
                        >> above(25)
                          >> set_state("critical");

auto service_foo2 = service("foo2")
                      >> coalesce(sum)
                        >> above(25)
                          >> set_state("critical");

            [....]

auto service_fooN = service("fooN")
                      >> coalesce(sum)
                        >> above(25)
                          >> set_state("critical");
```

It is obvious that the code above does not scale well, especially if you do not
even know the service names beforehand.

You can use *by()* to achive exactly that. This function takes a list of
strings, which are event fields, and replicates the streams for every distinct
combination of them.

The above code can be rewritten as follows:

```cpp

service_like("foo%")
  >> by({"service"})
    >> coalesce(sum)
      >> above(25)
        >> set_state("critical");

```

#### Filter events

Most of the time, when you are writing rules, the first thing that you need is
to select relevant events.

Cavalieri comes with a few functions that alllow you to that.

Here is a list of functions that are used to filter events.

* service (const std::string service)

* service_any (const std::vector&lt;std::string> services)

* service_like (const std::string pattern)

* service_like_any (const std::vector&lt;std::string> patterns)

* state (const std::string state)

* state_any (const std::vector&lt;std::string> states)

* has_attribute (const std::string attribute)

* tagged_any (const tags_t & tags)

* tagged_all (const tags_t & tags)

* tagged (const std::string tag)

* above (double k)

* under (double k)

* within (double a, double b)

* without (double a, double b)

* expired ()

The rule below only sends emails with events that contain an *error* state.

```cpp

state("error") >> email("ops@foobar.com")

```

And the next rule selects, in a first step, events that contain the substring
*cassandra*, and as a second step, it only forwads those that are *critical*.

```cpp

service_like("%cassandra%") >> state("critical") >> email("ops@foobar.com")

```


See
[stream function documentation](https://github.com/juruen/cavalieri/#stream-functions)
for some examples.



If the above functions are not enough you can use *where()* with any of the
[predicate functions](https://github.com/juruen/cavalieri#predicate-functions).

```cpp

where(p::match_like(description, "%mysql%")) >> prn("mysql exceptions")

```

You can create your own predicate function.

```cpp

auto my_predicate = PRED((metric_to_double(e) * 100) > 2.5);

```

That can be used as follows:

```cpp

where(my_predicate) >> prn("my predicate in full effect");

```

Note that *my_predicate* makes use of a *PRED* macro. It is equivalent to:

```cpp

auto my_predicate = [=](e_t e) { return ((metric_to_double(e) * 100) > 2.5); };

```

Or you can just create your own stream function:

```cpp

auto my_filter = create_stream(
  [=](e_t e)
  {
    if ((metric_to_double(e) * 100) > 2.5) {
      return {e};
    } else {
      return {};
    }
  }


my_filter() >> prn("hey, I just created a stream function to filter stuff");

```

#### Set thresholds

You can use
[critical_above](https://github.com/juruen/cavalieri#critical_above-double-value)
or
[critical_under](https://github.com/juruen/cavalieri#critical_under-double-value)
to automatically set to either *OK* or *critical* for events that go through it.

If you need more fine-grained thresholds, you can build your own using *split*.

```cpp

split_clauses_t thresholds = {{p::under(10), set_state("ok")},
                              {p::under(30), set_state("warning")},
                              {set_state("critical"}};

split(thresholds) >> send_index();
```

#### State changes

Most of the time, when you need to notify via email or Pager Duty, it is useful
to only do so when there is a state change. You are most certainly instersted
in state transitions: from *ok* to *critical*, or *critical* to *ok*.

*changed_state()* helps you to implement the above behavior. One thing to note
is that, this function will handle the pairs of *host* and *service*
independently.


```cpp

change_state() >> email("ops@foobar");

```

*change_state()* assummes that the initial state is *ok*. If you would like to
change that you can use *change_state("other_initial_state")*.

#### Measue your application latency

Say your application is reporting how long it takes for an individual request
to be proccessed. And you would like to get a distribution of its latency to
give you an overall idea of how healthy your system is.

Your clients send something like this:

```json
{
  :service "api request"
  :metric 0.140
}
```

To calculate the latency distribution we use
[percentiles](https://github.com/juruen/cavalieri#percentiles-time_t-interval-const-stdvector-percentiles).

This function takes an *interval* in seconds, and a vector of doubles
representing the percentiles that you are interested in. Events
enter the function, and a distribution of its metrics is stored in it. Every
*interval* seconds, a list of events containing the value of every percentile
is emitted.

```cpp

auto request_latency = percentiles(5, {0.0, 0.5, 0.9, 0.95, 1});

```

We also want to report the rate of API requests that our system is handling. To
do that, we use *rate*, which takes an *interval* in seconds. During this
*interval*, *rate* accumulates the metrics that receives, and at the end of the
interval, the accumulated value is divided by *interval*. Note that we need
to set the metric to *1* before entering *rate*. Otherwise, we wouldn't be
computing the request rate.

```cpp

auto request_rate = set_metric(1) >> rate(5);

```

Finally, we use *request_latency* and *request_rate*. The events that are
outputed by these two streams are set to state *ok* and get indexed so we
can nicely see them in green on our dashboard.

```cpp

sdo(request_latency, request_rate) >> set_state("ok") >> send_index;

```

#### Report exceptions

It is easy to report exceptions using Riemann clients from your app.

If that is the case, it is simple to be notified, for example, via email.

```cpp

tagged("exception") >> email("crash@bar.org");

tagged_all("exception", "cassandra") >> email("cassandra@bar.org");

```

#### Throttle events


