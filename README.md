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
    with({{"host", "cluster-001"}, {"description", "aggregated master metrics"})
```
#### default_to (const with_changes_t & changes)

It takes a map that contains the keys to be added to the event if the key is not set.

```cpp
    // Default ttl to 120
    default_to({"ttl", 120})
```

#### split (const split_clauses_t clauses)

It takes a list of pairs. Each pair contains a predicate function and a stream.
When an event is received, the event is passed to the first stream which predicate returns true.

```cpp
    split({above_(10), set_state("ok")},
          {under_(5),  set_state("critical"})
```

#### split (const split_clauses_t clauses, const streams_t default_stream)

It takes a list of pairs and a default stream. Each pair contains a predicate function and a stream.
When an event is received, the event is passed to the first stream which predicate returns true. If
none of the predicates match, the event is passed to the default stream.

```cpp
    split({above_(10), set_state("ok")},
          {under_(5),  set_state("critical")},
          set_state("warning"))
```


### Fold functions

### Predicate functions

### Some utility functions

### Some typedefs

Dashboard
---------
