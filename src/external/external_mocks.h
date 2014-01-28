#ifndef EXTERNAL_MOCKS_H
#define EXTERNAL_MOCKS_H

#include <pagerduty.h>
#include <unordered_map>

class external_mocks {
public:
  void add_call(const std::string & external, const Event & e);

private:
  std::unordered_map<std::string, std::vector<Event>> calls_;
};

extern external_mocks g_external_mocks;
#endif
