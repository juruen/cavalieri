#ifndef MAILER_POOL_H
#define MAILER_POOL_H

#include <transport/curl_pool.h>

class mailer_pool {
public:
  mailer_pool(const size_t thread_num);
  void push_event(const std::string server, const std::string from,
                  const std::vector<std::string> to, const Event & event);
private:
  void curl_event(const queued_event_t,
                  const std::shared_ptr<CURL>,
                  std::function<void()>&);

private:
  curl_pool curl_pool_;
};

#endif
