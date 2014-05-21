#include <rules.h>
#include <external/pagerduty.h>
#include <external/email.h>

streams_t* rules() {

  return new streams_t(send_graphite("localhost", 2003));

}
