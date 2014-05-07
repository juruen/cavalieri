#ifndef EXTERNAL_GRAPHITE_H
#define EXTERNAL_GRAPHITE_H

#include <proto.pb.h>
#include <mutex>
#include <config/config.h>
#include <external/graphite_pool.h>

class graphite {
public:
  graphite(const config conf);
  void push_event(const std::string host, const int port, const Event & event);

private:
  std::shared_ptr<graphite_pool> pool(const std::string host,
                                      const int port);
private:
  const config config_;
  std::unordered_map<std::string,
                     std::shared_ptr<graphite_pool>> pool_map_;
  std::mutex mutex_;

};

#endif
