#ifndef EXTERNAL_MOCKS_H
#define EXTERNAL_MOCKS_H

#include <pagerduty.h>
#include <unordered_map>

typedef struct {
  std::string external_method;
  std::string message;
  std::string extra;
  time_t time;
  Event e;
} external_event_t;

class external_mocks {
public:
  void add_call(const std::string & external, const std::string & message,
                const std::string & extra, const Event & e);
  std::vector<external_event_t> calls() const;

private:
  std::vector<external_event_t> calls_;
};

extern external_mocks g_external_mocks;
#endif
