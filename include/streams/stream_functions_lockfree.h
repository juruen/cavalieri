#ifndef STREAM_FUNCTIONS_LOCKFREE
#define STREAM_FUNCTIONS_LOCKFREE

#include <streams/stream_functions.h>


streams_t by_lockfree(const by_keys_t & keys, const by_stream_t stream);

streams_t coalesce_lockfree(fold_fn_t fold);

streams_t project_lockfree(const predicates_t predicates, fold_fn_t fold);

streams_t changed_state_lockfree(std::string initial);

streams_t moving_event_window_lockfree(size_t n, fold_fn_t fold);

streams_t fixed_event_window_lockfree(size_t n, fold_fn_t fold);

streams_t moving_time_window_lockfree(time_t dt, fold_fn_t fold);

streams_t fixed_time_window_lockfree(time_t dt, fold_fn_t fold);

streams_t stable_lockfree(time_t dt);

streams_t throttle_lockfree(size_t n, time_t dt);

streams_t ddt_lockfree();
#endif
