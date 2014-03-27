#include <rules/rules.h>
#include <pagerduty.h>

streams_t* rules() {
  return new streams_t(prn());
}
