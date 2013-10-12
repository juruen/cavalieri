#ifndef INDEX_H
#define INDEX_H

#include <unordered_map>
#include <queue>
#include <proto.pb.h>
#include <pubsub.h>

typedef std::unordered_map<std::string, const Event> index_t;
typedef std::pair<Event&, index_t::iterator> expire_t;

class ExpireCompare
{
private:
  bool reverse;
public:
  ExpireCompare(const bool& revparam=false);
  bool operator() (const expire_t& lhs, const expire_t& rhs) const;
};

typedef std::priority_queue<expire_t, std::vector<expire_t>, ExpireCompare> expire_queue_t;

class Index {
private:
  index_t index;
  expire_queue_t expire;
  PubSub& pubsub;

public:
  Index(PubSub& pubsub);
  void add_event(const Event& e);
};

#endif
