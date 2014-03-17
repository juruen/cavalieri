#include <index/index.h>

index::index(index_interface & impl) : impl_(impl) {}

void index::add_event(const Event & event) {
  impl_.add_event(event);
}
