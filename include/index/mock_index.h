#ifndef CAVALIERI_INDEX_MOCK_INDEX_H
#define CAVALIERI_INDEX_MOCK_INDEX_H

#include <index/index.h>
#include <unordered_map>

typedef std::pair<time_t, Event> mock_index_events_t;

class mock_index : public index_interface {
public:
  void add_event(const Event& e);
  std::vector<mock_index_events_t> events() const;

private:
  std::vector<mock_index_events_t> events_;
};

#endif
