#include <glog/logging.h>
#include <functional>
#include <memory>
#include <utility>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <fcntl.h>
#include <transport/tcp_connection.h>
#include <transport/curl_pool.h>

namespace {

const size_t k_queue_capacity = 10000;
const float k_initial_interval_secs = 2;

char error_[10 * 1024];


async_fd::mode curl_to_async_mode(const int mode) {

  async_fd::mode a_mode;

  switch (mode) {
    case CURL_POLL_NONE:
      a_mode = async_fd::none;
      break;
    case CURL_POLL_IN:
      a_mode = async_fd::read;
      break;
    case CURL_POLL_OUT:
      a_mode = async_fd::write;
      break;
    case CURL_POLL_INOUT:
      a_mode = async_fd::readwrite;
      break;
    default:
      a_mode = async_fd::none;
      LOG(ERROR) << "mode is to remove";
  }

  return a_mode;
}


void multi_delete(CURLM *curlm) {
  if (curlm) {
    curl_multi_cleanup(curlm);
  } else {
    LOG(ERROR) << "culrm is 0";
  }
}

void easy_delete(CURL *curl) {
  if (curl) {
    curl_easy_cleanup(curl);
  } else {
    LOG(ERROR) << "curl is 0";
  }
}

size_t write_cb(void *ptr, size_t size, size_t nmemb, void *) {
  std::string buffer((char*)ptr, (char*)ptr + (size * nmemb));

  return size * nmemb;
}

curl_socket_t sock_cb (void* fn, curlsocktype, struct curl_sockaddr *addr) {
  int fd = socket(addr->family, addr->socktype, addr->protocol);

  if (fd > -1) {

    fcntl(fd, F_SETFL, fcntl(fd, F_GETFL, 0) | O_NONBLOCK);

  } else {

    LOG(ERROR) << "Failed to create fd. Run out of file descriptors?";

  }

  CHECK(fn) << "fn is empty";

  (*static_cast<curl_pool::create_socket_cb_t*>(fn))(fd);

  return fd;
}

void multi_timer_cb(CURLM *, long ms, void *fn) {

  CHECK(fn) << "fn is empty";

  (*static_cast<curl_pool::set_timer_cb_t*>(fn))(ms);

}

int multi_sock_cb(CURL *, curl_socket_t socket, int mode, void *fn, void *in) {

  CHECK(fn) << "fn is empty";

  (*static_cast<curl_pool::multi_socket_cb_t*>(fn))(socket, mode, in);

  return 0;
}

}

using namespace std::placeholders;

curl_pool::curl_pool(const size_t thread_num,
                     const curl_event_fn_t curl_event_fn)
:
  tcp_pool_(
    thread_num,
    {},
    std::bind(&curl_pool::on_create, this, _1, _2, _3),
    std::bind(&curl_pool::on_ready, this, _1, _2),
    k_initial_interval_secs,
    std::bind(&curl_pool::timer, this, _1),
    std::bind(&curl_pool::async, this, _1)
  ),
  curl_event_fn_(curl_event_fn),
  thread_event_queues_(0),
  curl_conns_(thread_num),
  fd_curl_conns_(thread_num),
  fd_create_socket_cbs_(thread_num),
  finished_fds_(thread_num),
  fd_initial_modes_(thread_num),
  next_thread_(0),
  sock_inited_(new bool(true))
{

  VLOG(3) << "curl_pool()";

  for (size_t i = 0; i < thread_num; i++) {

    auto queue = std::make_shared<event_queue_t>();
    queue->set_capacity(k_queue_capacity);

    thread_event_queues_.push_back(queue);

    curl_multis_.push_back({curl_multi_init(), multi_delete});

    // Disable pipelining
    curl_multi_setopt(curl_multis_[i].get(), CURLMOPT_PIPELINING, 0);

    // Set timer
    auto timer_cb = [=](const long ms)
    {
      multi_timer(i, ms);
    };

    set_timer_cbs_.push_back(
        std::unique_ptr<set_timer_cb_t>(new set_timer_cb_t(timer_cb)));


    curl_multi_setopt(curl_multis_[i].get(), CURLMOPT_TIMERFUNCTION,
                      multi_timer_cb);

    curl_multi_setopt(curl_multis_[i].get(), CURLMOPT_TIMERDATA,
                      set_timer_cbs_[i].get());

   // Set socket
    auto sock_cb = [=](const int fd, const int mode, const bool initialized)
    {
      multi_socket(i, fd, mode, initialized);
    };

    multi_socket_cbs_.push_back(
        std::unique_ptr<multi_socket_cb_t>(new multi_socket_cb_t(sock_cb)));


   curl_multi_setopt(curl_multis_[i].get(), CURLMOPT_SOCKETFUNCTION,
                      multi_sock_cb);

   curl_multi_setopt(curl_multis_[i].get(), CURLMOPT_SOCKETDATA,
                     multi_socket_cbs_[i].get());

  }

  tcp_pool_.start_threads();
}

void curl_pool::push_event(const Event & event, boost::any extra) {

  auto & queue = thread_event_queues_[next_thread_];

  if (!queue->try_push({event, extra})) {
    LOG(ERROR) << "queue of thread " << next_thread_ << " is full";
    return;
  }

  tcp_pool_.signal_thread(next_thread_);

  next_thread_ = (next_thread_ + 1) % thread_event_queues_.size();

}

void curl_pool::set_fd(const size_t loop_id, const int fd,
                       const async_fd::mode mode)
{
  tcp_pool_.loop(loop_id).set_fd_mode(fd, mode);
}

void curl_pool::remove_fd(const size_t loop_id, const int fd) {

  finished_fds_[loop_id].insert(fd);

}

void curl_pool::on_create(int fd, async_loop & loop, tcp_connection & tcp_conn)
{
  curl_multi_assign(curl_multis_[loop.id()].get(), fd, sock_inited_.get());

  auto it = fd_initial_modes_[loop.id()].find(fd);

  CHECK(it != fd_initial_modes_[loop.id()].end()) << "failed to find fd";

  set_fd(loop.id(), fd, curl_to_async_mode(it->second));

  fd_initial_modes_[loop.id()].erase(it);
}

void curl_pool::on_ready(async_fd & async, tcp_connection & tcp_conn) {
  auto loop_id = async.loop().id();

  int action = async.ready_read() ? CURL_POLL_IN : 0 |
               async.ready_write() ? CURL_POLL_OUT : 0;

  auto multi = curl_multis_[loop_id].get();

  int still_running;

  auto rc = curl_multi_socket_action(multi, async.fd(), action, &still_running);

  if (rc != CURLM_OK) {

    LOG(ERROR) << "loop " << loop_id << " reports a curl error: " << rc;

  }


  check_multi_info(loop_id);

  auto it = finished_fds_[loop_id].find(async.fd());

  if (it == finished_fds_[loop_id].end()) {
    return;
  }

  // Remove fd and clean up
  tcp_conn.close_connection = true;

  finished_fds_[loop_id].erase(it);

  fd_create_socket_cbs_[loop_id].erase(async.fd());

  auto curl_conn_it = fd_curl_conns_[loop_id].find(async.fd());
  CHECK(curl_conn_it != fd_curl_conns_[loop_id].end()) << "fd not found";

  curl_multi_remove_handle(curl_multis_[loop_id].get(),
                           curl_conn_it->second.get());

  for (auto p_it = curl_conns_[loop_id].begin();
       p_it != curl_conns_[loop_id].end();
       p_it++)
  {

    if (p_it->first == curl_conn_it->second) {

      auto & clean_up_fn = p_it->second.second;

      if (clean_up_fn) {
        clean_up_fn();
      } else {
        VLOG(1) << "clean_fn is not defined!";
      }

      curl_conns_[loop_id].erase(p_it);

      break;
    }

  }

  fd_curl_conns_[loop_id].erase(curl_conn_it);

}

void curl_pool::async(async_loop & loop) {

  size_t loop_id =  loop.id();

  auto event_queue = thread_event_queues_[loop_id];

  while (!event_queue->empty()) {

    queued_event_t event;
    if (!event_queue->try_pop(event)) {
      continue;
    }

    auto curl_conn = std::shared_ptr<CURL>(curl_easy_init(), easy_delete);
    auto easy = curl_conn.get();

    std::shared_ptr<create_socket_cb_t> create_socket_fn(
        new create_socket_cb_t(
          [=](const int fd) {
            if (fd > 0) {
              fd_curl_conns_[loop_id].insert({fd, curl_conn});
              tcp_pool_.add_client(loop_id, fd);
            }
          }));

    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION,  write_cb);
    curl_easy_setopt(easy, CURLOPT_VERBOSE, 1L);
    curl_easy_setopt(easy, CURLOPT_ERRORBUFFER, &error_[0]);
    curl_easy_setopt(easy, CURLOPT_LOW_SPEED_TIME, 3L);
    curl_easy_setopt(easy, CURLOPT_LOW_SPEED_LIMIT, 10L);
    curl_easy_setopt(easy, CURLOPT_OPENSOCKETDATA, create_socket_fn.get());
    curl_easy_setopt(easy, CURLOPT_OPENSOCKETFUNCTION, sock_cb);

    clean_up_fn_t clean_fn;

    curl_event_fn_(event, curl_conn, clean_fn);

    curl_conns_[loop_id].push_back({curl_conn, {create_socket_fn, clean_fn}});

    auto rc = curl_multi_add_handle(curl_multis_[loop_id].get(), easy);

    if (rc != CURLM_OK) {
      LOG(ERROR) << "error adding handle to curl_multi";

      auto & clean_up_fn = curl_conns_[loop_id].back().second.second;

      if (clean_up_fn) {
        clean_up_fn();
      } else {
        VLOG(1) << "clean_fn is not defined!";
      }

      curl_conns_[loop_id].pop_back();
    }


  }

}

void curl_pool::timer(async_loop & loop) {

  tcp_pool_.loop(loop.id()).set_timer_interval(k_initial_interval_secs);

  multi_socket_action(loop.id());
}

void curl_pool::check_multi_info(const size_t loop_id) {

  int msg_left;

  auto multi = curl_multis_[loop_id].get();

  while ( CURLMsg *msg = curl_multi_info_read(multi, &msg_left) ) {

      if (msg->msg != CURLMSG_DONE) {
        continue;
      }

      auto easy = msg->easy_handle;

      char *eff_url;

      curl_easy_getinfo(easy, CURLINFO_EFFECTIVE_URL, &eff_url);

      VLOG(3) << "url: " << eff_url << " done with result: "
              << msg->data.result;
  }

}

void curl_pool::multi_timer(const size_t loop_id, const long ms) {

  if (ms > 0 && ms < 5000) {

    float t = ms / 1000;
    tcp_pool_.loop(loop_id).set_timer_interval(t);

  } else {

    multi_socket_action(loop_id);

  }

}

void curl_pool::multi_socket_action(const size_t loop_id) {

  int still_running;

  auto rc = curl_multi_socket_action(curl_multis_[loop_id].get(),
                                     CURL_SOCKET_TIMEOUT, 0,
                                     &still_running);

  if (rc != CURLM_OK) {

    LOG(ERROR) << "loop " << loop_id << " reports a curl error: " << rc;
    return;

  }

  check_multi_info(loop_id);
}

void curl_pool::multi_socket(const size_t loop_id, const int fd,
                             const int mode, const bool initialized)
{

  if (mode == CURL_POLL_REMOVE) {
    remove_fd(loop_id, fd);
    return;
  }

  if (!initialized) {
    fd_initial_modes_[loop_id].insert({fd, mode});
    return;
  }

   set_fd(loop_id, fd, curl_to_async_mode(mode));
}
