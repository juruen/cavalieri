#ifndef CAVALIERI_EXTERNAL_GRAPHITE_POOL_H
#define CAVALIERI_EXTERNAL_GRAPHITE_POOL_H

#include <transport/tcp_client_pool.h>

class graphite_pool {
  public:
    graphite_pool(const size_t thread_num, const std::string host,
                  const int port);
    void push_event(const Event & event);

  private:
    std::vector<char> output_events(const std::vector<Event> events);

  private:
    tcp_client_pool tcp_client_pool_;
};

#endif
