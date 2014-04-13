#ifndef GRAPHITE_GRAPHITE_H
#define GRAPHITE_GRAPHITE_H

#include <proto.pb.h>
#include <mutex>
#include <graphite/graphite_pool.h>

class graphite {
public:
  void push_event(const std::string host, const int port, const Event & event);

private:
  std::shared_ptr<graphite_pool> pool(const std::string host,
                                      const int port);
private:
  std::unordered_map<std::string,
                     std::shared_ptr<graphite_pool>> pool_map_;
  std::mutex mutex_;

};

#endif
