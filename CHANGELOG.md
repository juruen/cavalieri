0.0.6 TBD
=========

  * Fix issue with queries mixing int and doubles
  * Support "true" as a query and not only "(true)"
  * Add internal metrics
  * Graphite fixes

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
