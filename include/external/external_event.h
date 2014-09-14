#ifndef CAVALIERI_EXTERNAL_EXTERNAL_EVENT_H
#define CAVALIERI_EXTERNAL_EXTERNAL_EVENT_H

#include <string>
#include <proto.pb.h>

typedef struct {
  const std::string external_method;
  const std::string message;
  const std::string extra;
  const time_t time;
  const Event e;
} external_event_t;

#endif
