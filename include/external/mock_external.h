#ifndef CAVALIERI_EXTERNAL_MOCK_EXTERNAL_H
#define CAVALIERI_EXTERNAL_MOCK_EXTERNAL_H

#include <external/external.h>
#include <external/external_event.h>

class mock_external : public external_interface {
public:
  void forward(const std::string server, const int port, const Event event);
  void graphite(const std::string server, const int port, const Event event);
  void pager_duty_trigger(const std::string pg_key, const Event event);
  void pager_duty_resolve(const std::string pg_key, const Event event);
  void pager_duty_acknowledge(const std::string pg_key, const Event event);
  void email(const std::string server, const std::string from,
             const std::vector<std::string> to, const Event event);

  std::vector<external_event_t> calls() const;

private:
  std::vector<external_event_t> calls_;

};

#endif
