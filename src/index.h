#ifndef INDEX_H
#define INDEX_H

#include <unordered_map>
#include <proto.pb.h>
#include <pubsub.h>

typedef std::unordered_map<std::string, const Event> index_t;

class Index {
private:
  index_t index;
  PubSub& pubsub;

public:
  Index(PubSub&);
  void add_event(const Event& e);
};

#endif
