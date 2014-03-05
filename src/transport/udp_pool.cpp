#include <glog/logging.h>
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>
#include <transport/udp_pool.h>
#include <util.h>

using namespace std::placeholders;

namespace {

const int k_default_udp_port = 5555;
const size_t k_udp_buffer_size = 4096;

int create_listen_udp_socket(int port) {
  int sd = socket(PF_INET, SOCK_DGRAM, 0);

  struct sockaddr_in addr;

  bzero(&addr, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(port);
  addr.sin_addr.s_addr = INADDR_ANY;

  if (bind(sd, (struct sockaddr*) &addr, sizeof(addr)) != 0) {
    LOG(FATAL) << "bind";
    exit(1);
  }

  return sd;
}
}

udp_pool::udp_pool(
    size_t thread_num,
    udp_read_fn_t udp_read_fn)
:
  thread_pool_(thread_num),
  udp_read_fn_(udp_read_fn)
{
  VLOG(3) << "udp_pool() size: " << thread_num;
  thread_pool_.set_run_hook(std::bind(&udp_pool::run_hook, this, _1));
}

void udp_pool::run_hook(async_loop & loop) {
  size_t tid = loop.id();
  VLOG(3) << "udp pool run_hook() tid: " << tid;

  auto socket_cb = std::bind(&udp_pool::socket_callback, this, _1);
  loop.add_fd(create_listen_udp_socket(k_default_udp_port), async_fd::read,
              socket_cb);
}

void udp_pool::socket_callback(async_fd & async) {

  auto tid = async.loop().id();
  VLOG(3) << "socket_callback() tid: " << tid;

  if (async.error()) {
    VLOG(3) << "got invalid event: " << strerror(errno);
    return;
  }

  std::vector<unsigned char> buffer(k_udp_buffer_size);

  struct sockaddr_in addr;
  socklen_t addr_len;

  size_t bytes = recvfrom(async.fd(), reinterpret_cast<char*>(&buffer[0]),
                          buffer.size(), 0,
                          reinterpret_cast<struct sockaddr*>(&addr),
                          static_cast<socklen_t*>(&addr_len));

  buffer.resize(bytes);

  udp_read_fn_(buffer);

  async.set_mode(async_fd::read);

  VLOG(3) << "--socket_callback() tid: " << tid;
}

void udp_pool::start_threads() {
  thread_pool_.start_threads();
}

void udp_pool::stop_threads() {
  thread_pool_.stop_threads();
}

udp_pool::~udp_pool() {
  thread_pool_.stop_threads();
}
