0.0.7 TBA
=========
  * Change stream data structures and functions
  * BY() macro is not necessary any more
  * Add support to replicate streams after by()
  * rate() works again
  * Make websockets more robust
  * Scheduler has its own thread pool for timers
  * Timers revamp
  * Add percentiles() stream function
  * Add has_attribute() stream function
  * Make prn() return the same event
  * Add null() stream function
  * Move predicates to their own file and namespace
  * Add changed_state() assuming "ok" as initial state
  * Add not_expired() stream function
  * Add count() fold function
  * Add set_host() function
  * Add email() signature for multiple receipients
  * Catch also SIGTERM
  * Add wrapper class to profobuf event class

0.0.6 2014-08-05
================

  * Fix issue with queries mixing int and doubles
  * Support "true" as a query and not only "(true)"
  * Add internal metrics
  * Graphite fixes
  * Limit concurrent curl connections
  * Use boost::circular to read/write from sockets
  * Simplify stream infrastructure
  * Use lock-based index
  * Add set_description()
  * Add also lock-based stream functions
  * Use JsonCpp in event_to_json()


0.0.5 2014-05-21
================

  * Add multi-threaded libcurl pool to implement HTTP(S) and emails clients.
  * Get rid of python interpreter
  * Stability fixes
  * Set SO_REUSEADDR on listening sockets

0.0.4 2014-04-19
================

  * Add some convenient stream functions

0.0.3 2014-04-19
================

  * Add TCP client pool infra
  * Add graphite support (new line TCP carbon protocl)
  * Add forward support to riemann/cavalieri (TCP protocol)
  * Add g_scheduler to g_core

0.0.2 2014-04-10
================

  * Fix websocket and pubsub
  * Add integration test for websocket
  * Improve documentation

0.0.1 2014-04-06
================

  * Initial release
