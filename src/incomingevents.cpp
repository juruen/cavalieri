#include <glog/logging.h>
#include <memory>
#include <incomingevents.h>

incoming_events::incoming_events(streams & streams_)
  :
    streams_(streams_) { }

void incoming_events::add_undecoded_msg(std::vector<unsigned char> undecoded_msg) {
  Msg msg;
  if (!msg.ParseFromArray(&undecoded_msg[0], undecoded_msg.size())) {
    VLOG(2) << "error parsing protobuf payload";
    return;
  }
  streams_.process_message(msg);
}
