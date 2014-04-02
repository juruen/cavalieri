cavalieri
=========

Introduction
------------

Install
-------

Create your rules
-----------------

Test your rules
---------------

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





### Fold functions

### Predicate functions

### Some utility functions

### Some typedefs

Dashboard
---------
