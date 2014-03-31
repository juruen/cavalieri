#include <rules/rules.h>
#include <external/pagerduty.h>
#include <external/email.h>

streams_t* rules() {
//  return new streams_t(email("localhost", "juruen@foobar", "juruen@fooops"));
  return new streams_t(prn());
}
