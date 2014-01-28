#include <mock_index.h>

void mock_index::add_event(const Event & e) {
  map_.insert({key(e), e});
}

std::string mock_index::key(const Event & e) const {
  return (e.host() + " " + e.service());
}

std::vector<Event> mock_index::events() const {
  std::vector<Event> events;
  for (const auto & p: map_) {
    events.push_back(p.second);
  }
  return events;
}
