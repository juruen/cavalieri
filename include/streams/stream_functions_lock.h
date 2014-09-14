#ifndef CAVALIERI_STREAMS_STREAM_FUNCTIONS_LOCK_H
#define CAVALIERI_STREAMS_STREAM_FUNCTIONS_LOCK_H

#include <streams/stream_functions.h>

streams_t by_lock(const by_keys_t & keys, const streams_t stream);

streams_t by_lock(const by_keys_t & keys);

streams_t coalesce_lock(fold_fn_t fold);

streams_t project_lock(const predicates_t predicates, fold_fn_t fold);

streams_t changed_state_lock(std::string initial);

streams_t moving_event_window_lock(size_t n, fold_fn_t fold);

streams_t fixed_event_window_lock(size_t n, fold_fn_t fold);

streams_t moving_time_window_lock(time_t dt, fold_fn_t fold);

streams_t fixed_time_window_lock(time_t dt, fold_fn_t fold);

streams_t stable_lock(time_t dt);

streams_t throttle_lock(size_t n, time_t dt);

streams_t percentiles_lock(time_t interval, std::vector<double> percentiles);

streams_t ddt_lock();

#endif
