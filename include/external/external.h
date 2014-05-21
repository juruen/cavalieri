#ifndef EXTERNAL_EXTERNAL_H
#define EXTERNAL_EXTERNAL_H

#include <memory>
#include <proto.pb.h>

class external_interface {
public:

  // Forward to other cavalieri/riemann server
  virtual void forward(const std::string server, const int port,
                       const Event event) = 0;

  // Send to graphite using TCP carbon with new line separator
  virtual void graphite(const std::string server, const int port,
                        const Event event) = 0;

  // Pager pager_duty
  virtual void pager_duty_trigger(const std::string pg_key,
                                  const Event event) = 0;
  virtual void pager_duty_resolve(const std::string pg_key,
                                  const Event event) = 0;
  virtual void pager_duty_acknowledge(const std::string pg_key,
                                      const Event event) = 0;

  // Send email
  virtual void email(const std::string server, const std::string from,
                     const std::string to, const Event event) = 0;

  virtual ~external_interface() {};
};

#endif

