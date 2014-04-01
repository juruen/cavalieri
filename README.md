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

Streams API
------------

### Stream functions

#### What is a stream function?

#### prn()

Prints events that pass through it.

#### with (const with_changes_t & changes)

Modifies the event. It takes a map that contains the keys to be modified and their corresponding new value.

    // Change host field and description
    with({{"host", "cluster-001"}, {"description", "aggregated master metrics"})

#### default_to (const with_changes_t & changes)

Sets event's fields only if they are not set.  It takes a map that contains the keys to be added to the event.

    // Default ttl to 120
    default_to({"ttl", 120})


### Fold functions

### Predicate functions

### Some utility functions

### Some typedefs
