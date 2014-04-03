#ifndef MAILER_H
#define MAILER_H

#include <streams/stream_infra.h>

streams_t email(const std::string & server, const std::string & from,
                const std::string & to);

#endif
