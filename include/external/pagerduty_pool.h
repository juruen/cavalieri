#ifndef PAGERDUTY_POOL_H
#define PAGERDUTY_POOL_H

#include <transport/curl_pool.h>

class pagerduty_pool {
public:
  enum class pd_action {
    trigger,
    acknowledge,
    resolve
  };

public:
  pagerduty_pool(const size_t thread_num, const bool enable_debug);
  void push_event(const pd_action action, const std::string pd_key,
                  const Event & event);

private:
  void curl_event(const queued_event_t, const std::shared_ptr<CURL>,
                  std::function<void()>& clean_fn);

private:
  curl_pool curl_pool_;
  bool enable_debug_;
};

#endif
