#ifndef MOCK_INDEX_H
#define MOCK_INDEX_H

#include <index.h>
#include <unordered_map>

class mock_index : public index_interface {
public:
  void add_event(const Event& e);
  std::vector<Event> events() const;

private:
  std::string key(const Event& e) const;

private:
  std::unordered_map<std::string, Event> map_;
};

#endif
