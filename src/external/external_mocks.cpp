#include <external_mocks.h>

void external_mocks::add_call(const std::string & external, const Event & e) {
  auto it = calls_.insert({external, {e}});
  if (!it.second) {
    it.first->second.push_back(e);
  }
}

stream_t pd_trigger(const std::string &) {
  return [=](e_t e) {
   g_external_mocks.add_call("pg_trigger", e);
  };
}

stream_t pd_resolve(const std::string &) {
  return [=](e_t e) {
   g_external_mocks.add_call("pg_resolve", e);
  };
}

stream_t pd_acknowledge(const std::string &) {
  return [=](e_t e) {
   g_external_mocks.add_call("pg_ackknowledge", e);
  };
}
