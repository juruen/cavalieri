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
const size_t k_max_concurrent_conns = 50;

char error_[CURL_ERROR_SIZE];


async_fd::mode curl_to_async_mode(const int mode) {

  VLOG(3) << "setting fd to mode " << mode;

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

  VLOG(3) << "multi_delete()";

  if (curlm) {
    curl_multi_cleanup(curlm);
  } else {
    LOG(ERROR) << "culrm is 0";
  }
}

void easy_delete(CURL* curl) { }

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

int close_sock_cb (void* fn, curl_socket_t fd) {

  VLOG(3) << "libcurl requests to close fd: " << fd;

  (*static_cast<curl_pool::create_socket_cb_t*>(fn))(fd);

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
    {},
    std::bind(&curl_pool::on_ready, this, _1, _2),
    k_initial_interval_secs,
    std::bind(&curl_pool::timer, this, _1),
    std::bind(&curl_pool::async, this, _1)
  ),
  curl_event_fn_(curl_event_fn),
  thread_event_queues_(0),
  curl_conns_(thread_num),
  finished_conns_(thread_num),
  next_thread_(0)
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

void curl_pool::stop() {

  VLOG(3) << "stop()";

  tcp_pool_.stop_threads();

  for (size_t i = 0; i < curl_conns_.size(); i++) {
    cleanup_conns(i);
  }

}

void curl_pool::set_fd(const size_t loop_id, const int fd,
                       const async_fd::mode mode)
{
  tcp_pool_.loop(loop_id).set_fd_mode(fd, mode);
}

void curl_pool::on_ready(async_fd & async, tcp_connection & tcp_conn) {

  VLOG(3) << "on_ready()++";

  auto loop_id = async.loop().id();

  int action;

  if (async.error()) {

    VLOG(3) << "error in socket detected";

    action = CURL_POLL_REMOVE;

  } else {

    action = async.ready_read() ? CURL_POLL_IN : 0 |
               async.ready_write() ? CURL_POLL_OUT : 0;

    auto multi = curl_multis_[loop_id].get();

    int still_running;

    auto rc = curl_multi_socket_action(multi, async.fd(), action,
                                       &still_running);

    if (rc != CURLM_OK) {

      LOG(ERROR) << "loop " << loop_id << " reports a curl error: " << rc;

    }

    check_multi_info(loop_id);

  }

  VLOG(3) << "on_ready()--";

}

void curl_pool::cleanup_conns(const size_t loop_id) {

  VLOG(3) << "cleanup_conns()++ ";

  for (auto curl_conn : finished_conns_[loop_id]) {

    VLOG(3) << "cleanup_cons() conn: " << curl_conn;

    auto curl_conn_it = curl_conns_[loop_id].find(curl_conn);

    CHECK(curl_conn_it != curl_conns_[loop_id].end()) << "curl_conn not found";

    auto rc = curl_multi_remove_handle(curl_multis_[loop_id].get(), curl_conn);

    if (rc != CURLM_OK) {

      LOG(ERROR) << "curl_multi_remove_handle" << loop_id
                  << " reports a curl error: " << rc;

    }

    auto cleanup_fn = curl_conn_it->second.cleanup_fn;

    if (cleanup_fn) {
      cleanup_fn();
    } else {
      VLOG(1) << "clean_fn is not defined!";
    }

    remove_fds(loop_id, curl_conn);

    curl_conns_[loop_id].erase(curl_conn_it);

    curl_easy_cleanup(curl_conn);

  }

  finished_conns_[loop_id].clear();

  VLOG(3) << "cleanup_conns()-- ";

}

void curl_pool::add_fd(size_t loop_id, CURL* curl_conn, int fd) {

  VLOG(3) << "add_fd fd: " << fd << " curl_conn " << curl_conn;

  // Add fd to curl_conn_data
  auto it = curl_conns_[loop_id].find(curl_conn);

  CHECK(it != curl_conns_[loop_id].end());

  auto & conn_data = it->second;
  conn_data.fds.insert(fd);

  // Add fd to tcp_pool loop
  tcp_pool_.add_client_sync(loop_id, fd);

}

void curl_pool::remove_fds(size_t loop_id, CURL* curl_conn) {

  VLOG(3) << "remove_fds() " <<  curl_conn << curl_conn;

  // Find curl_conn
  auto conn_it = curl_conns_[loop_id].find(curl_conn);

  CHECK(conn_it != curl_conns_[loop_id].end());

  auto & conn_data = conn_it->second;

  for (auto fd : conn_data.fds) {

    // Remove fd from  tcp_pool loop
    VLOG(3) << "remove fd: " << fd;

    tcp_pool_.remove_client_sync(loop_id, fd);
  }


}

void curl_pool::async(async_loop & loop) {

  VLOG(3) << "async()";

  size_t loop_id =  loop.id();

  if (curl_conns_[loop_id].size() > k_max_concurrent_conns) {
    LOG(ERROR) << "too many concurrent connections";
    return;
  }

  auto event_queue = thread_event_queues_[loop_id];

  while (!event_queue->empty()) {

    queued_event_t event;
    if (!event_queue->try_pop(event)) {
      continue;
    }

    auto easy = curl_easy_init();

    VLOG(3) << "async()  creating easy: " << easy;

    std::shared_ptr<create_socket_cb_t> create_socket_fn(
        new create_socket_cb_t(
          [=](const int fd) {

            VLOG(3) << "crete_socket_cb()";

            if (fd > 0) {
              add_fd(loop_id, easy, fd);
            } else {
              LOG(ERROR) << "run out of file descriptors!!!???";
            }

          }));

    std::shared_ptr<close_socket_cb_t> close_socket_fn(
        new create_socket_cb_t(
          [=](const int fd) {

            VLOG(3) << "close_socket_cb()";

            if (fd > 0) {
              set_fd(loop_id, fd, async_fd::mode::none);
            } else {
              LOG(ERROR) << "asked to close an invalid file descriptor";
            }

          }));



    curl_easy_setopt(easy, CURLOPT_WRITEFUNCTION,  write_cb);
    curl_easy_setopt(easy, CURLOPT_ERRORBUFFER, &error_[0]);
    curl_easy_setopt(easy, CURLOPT_LOW_SPEED_TIME, 3L);
    curl_easy_setopt(easy, CURLOPT_LOW_SPEED_LIMIT, 10L);
    curl_easy_setopt(easy, CURLOPT_OPENSOCKETDATA, create_socket_fn.get());
    curl_easy_setopt(easy, CURLOPT_OPENSOCKETFUNCTION, sock_cb);
    curl_easy_setopt(easy, CURLOPT_CLOSESOCKETFUNCTION, close_sock_cb);
    curl_easy_setopt(easy, CURLOPT_CLOSESOCKETDATA, close_socket_fn.get());
    curl_easy_setopt(easy, CURLOPT_FORBID_REUSE, 1L);

    std::function<void()> clean_fn;

    curl_event_fn_(event, std::shared_ptr<CURL>(easy, easy_delete), clean_fn);

    curl_conns_[loop_id].insert({easy,
                                {create_socket_fn, close_socket_fn,
                                 clean_fn, std::unordered_set<int>()}});


    auto rc = curl_multi_add_handle(curl_multis_[loop_id].get(), easy);

    if (rc != CURLM_OK) {
      LOG(ERROR) << "error adding handle to curl_multi";

      if (clean_fn) {
        clean_fn();
      } else {
        VLOG(1) << "clean_fn is not defined!";
      }

      curl_conns_[loop_id].erase(easy);

    }


  }

}

void curl_pool::timer(async_loop & loop) {

  cleanup_conns(loop.id());

  tcp_pool_.loop(loop.id()).set_timer_interval(k_initial_interval_secs);

  multi_socket_action(loop.id());
}

void curl_pool::check_multi_info(const size_t loop_id) {

  VLOG(3) << "check_multi_info()++";

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

      finished_conns_[loop_id].push_back(easy);
  }

  VLOG(3) << "check_multi_info()--";

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

  VLOG(3) << "multi_socket_action()++";

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

  VLOG(3) << "multi_socket() " << fd;

  if (mode == CURL_POLL_REMOVE) {
    VLOG(3) << "multi_socket() got CURL_POLL_REMOVE";
    return;
  }

   set_fd(loop_id, fd, curl_to_async_mode(mode));
}
