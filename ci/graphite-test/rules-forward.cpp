#include <rules.h>

streams_t* rules() {

  return new streams_t(forward("localhost", 15555));

}
