#include <rules.h>

streams_t* rules() {

  auto mail_stream = email("localhost", "cavalieri@localhost",
                           "ubuntu@localhost");

  return new streams_t(mail_stream);
}
