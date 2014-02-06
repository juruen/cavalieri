#include <rules/rules.h>
#include <pagerduty.h>

children_t rules(class index & idx) {
  return sdo(S(send_index(idx), prn(), pd_trigger("foo")));
}
