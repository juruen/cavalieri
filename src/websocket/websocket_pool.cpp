#include <glog/logging.h>
#include <unordered_set>
#include <atomic>
#include <util/util.h>
#include <iostream>
#include <transport/ws_connection.h>
#include <websocket/websocket_pool.h>
#include <driver.h>
#include <expression.h>


namespace {

const float  k_ws_interval = 5;

std::function<bool(const Event&)> filter_query(const std::string uri) {

  std::string index;
  std::map<std::string, std::string> params;

  if (!parse_uri(uri, index, params)) {
    VLOG(1) << "failed to parse uri";
    return {};
  }

  query_context query_ctx;
  queryparser::driver driver(query_ctx);

  if (driver.parse_string(params["query"], "query")) {
    return  query_ctx.evaluate();
  } else {
    LOG(ERROR) << "failed to parse query: " << params["query"];
    return {};
  }
}

async_fd::mode conn_to_mode(const tcp_connection & conn) {
  if (conn.pending_read() && conn.pending_write()) {
    return async_fd::readwrite;
  } else if (conn.pending_read()) {
    return async_fd::read;
  } else if (conn.pending_write()) {
    return async_fd::write;
  } else {
    return async_fd::none;
  }
}

std::atomic<int> token(0);

size_t next_token() {
  token.fetch_add(1);
  return token;
}

}

using namespace std::placeholders;

websocket_pool::websocket_pool(size_t thread_num, pub_sub & pubsub,
                               real_index & index)
:
  index_(index),
  tcp_pool_(thread_num,
            {},
            std::bind(&websocket_pool::on_new_cnx, this, _1, _2, _3),
            std::bind(&websocket_pool::on_fd_ready, this, _1, _2),
            std::bind(&websocket_pool::on_async_signal, this, _1)),
  fd_ctxes_(thread_num),
  event_queues_(thread_num)
{

  pubsub.subscribe("index", [=](const Event & e){ worker_pool_.add_event(e); });

  for (auto & queue: event_queues_) {
    queue.set_capacity(k_max_ws_queue_size);
  }

  tcp_pool_.start_threads();
}

void websocket_pool::add_client(const int fd) {
  tcp_pool_.add_client(fd);
}

void websocket_pool::stop() {
  VLOG(3) << "stop()";

  tcp_pool_.stop_threads();
  worker_pool_.stop();
}

void websocket_pool::on_new_cnx(int fd, async_loop & loop,
                                 tcp_connection & cnx)
{
  VLOG(3) << "on_new_cnx: " << fd;

  fd_ctx_t fd_ctx = {ws_connection(cnx), {}, fd_queue_t{}, next_token()};

  fd_ctxes_[loop.id()].insert({fd, std::move(fd_ctx)});
}

void websocket_pool::on_fd_ready(async_fd & async, tcp_connection & tcp_conn) {

  const auto loop_id = async.loop().id();
  const auto fd = async.fd();
  auto it = fd_ctxes_[loop_id].find(fd);
  CHECK(it != fd_ctxes_[loop_id].end()) << "couldn't find fd";
  auto & fd_ctx = it->second;

  fd_ctx.ws_cnx.callback(async);
  async.set_mode(conn_to_mode(tcp_conn));

  if (tcp_conn.close_connection) {

    VLOG(3) << "closing ws cnx for fd" << fd;

    fd_ctxes_[loop_id].erase(it);
    update_filters(async.loop().id());

    return;
  }

  if  (pending_ws_init(fd_ctx)) {

    if (!handle_ws_init(async, fd_ctx)) {

      tcp_conn.close_connection = true;
      fd_ctxes_[loop_id].erase(it);

    }
  }

}

void websocket_pool::on_async_signal(async_loop & loop) {

  size_t loop_id =  loop.id();

  auto & event_queue = event_queues_[loop_id];

  std::unordered_set<int> available_fds;
  for (const auto & kv : fd_ctxes_[loop_id]) {
    available_fds.insert(kv.first);
  }

  while (!event_queue.empty()) {

    event_t e;
    if (!event_queue.try_pop(e)) {
      continue;
    }

    auto it = fd_ctxes_[loop_id].find(e.fd);

    if (it == fd_ctxes_[loop_id].end()) {
      VLOG(3) << "response to an already closed fd";
      continue;
    }

    auto & fd_ctx = it->second;

    if (fd_ctx.token !=  e.token) {
      VLOG(3) << "invalid token, this event belongs to a prevous fd";
      continue;
    }

    if (fd_ctx.queue.size() > k_max_ws_queue_size) {
      LOG(WARNING) << "queue for fd " << e.fd << " full, dropping event.";
      continue;
    }



    fd_ctx.queue.push(e.event.json_str());

    if (!flush_fd_queue(loop, e.fd, fd_ctx)) {
      available_fds.erase(e.fd);

      if (available_fds.empty()) {
        VLOG(3) << "all fd queues are full";
        break;
      }
    }

  }

}

bool websocket_pool::pending_ws_init(websocket_pool::fd_ctx_t & fd_ctx) {

  return (!fd_ctx.query_fn
          && (fd_ctx.ws_cnx.state() | ws_connection::k_read_frame_header));

}

bool websocket_pool::handle_ws_init(async_fd & async,
                                    websocket_pool::fd_ctx_t & fd_ctx)
{

    auto query_fn = filter_query(fd_ctx.ws_cnx.uri());

    if (query_fn) {

      fd_ctx.query_fn = query_fn;

      auto events(index_.query_index(query_fn, k_max_ws_queue_size));

      for (const auto & event : events) {
        fd_ctx.queue.emplace(event.json_str());
      }

      if (!fd_ctx.queue.empty()) {
        flush_fd_queue(async.loop(), async.fd(), fd_ctx);
      }

      update_filters(async.loop().id());

      return true;

    } else {

      return false;

    }


}

bool websocket_pool::flush_fd_queue(async_loop & loop, int fd,
                                    websocket_pool::fd_ctx_t & fd_ctx)
{

  while (!fd_ctx.queue.empty()) {

    const std::string str_event(fd_ctx.queue.front());

    if (fd_ctx.ws_cnx.send_frame(str_event)) {

      loop.set_fd_mode(fd, async_fd::readwrite);
      fd_ctx.queue.pop();

    } else {

      VLOG(3) << "ouput ws socket buffer full";

      return false;
    }

  }

  return true;
}

void websocket_pool::update_filters(const size_t loop_id) {

    event_filters_t filters;

    for (const auto & kv  : fd_ctxes_[loop_id]) {

      auto query_fn = kv.second.query_fn;
      auto fd = kv.first;
      auto token = kv.second.token;

      auto fn = [=](const Event & event) {

        if (!query_fn(event)) {
          return;
        }

        if (!event_queues_[loop_id].try_push({event, fd, token})) {
          return;
        }

        tcp_pool_.signal_thread(loop_id);

      };

      filters.push_back(fn);

    }

    worker_pool_.update_filters(loop_id, filters);

}
