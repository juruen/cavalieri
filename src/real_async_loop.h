#ifndef REAL_ASYNC_LOOP_H
#define REAL_ASYNC_LOOP_H

#include <async_loop.h>
#include <ev++.h>
#include <vector>
#include <map>

class real_async_loop;

class real_async_fd : public async_fd {
public:
  real_async_fd(
      int fd,
      async_fd::mode initial_mode,
      real_async_loop & loop,
      fd_cb_fn_t fd_cb_fn
  );
  int fd() const;
  bool error() const;
  void stop();
  bool ready_read() const;
  bool ready_write() const;
  void set_mode(const mode& mode);
  void async_cb(ev::io &, int);
  async_loop & loop();

private:
  async_loop & async_loop_;
  std::unique_ptr<ev::io> io_;
  int fd_;
  bool error_;
  bool read_;
  bool write_;
  fd_cb_fn_t fd_cb_fn_;
};


class real_async_loop : public async_loop {
public:
  real_async_loop();
  void set_id(size_t id);
  void set_async_cb(async_cb_fn_t async_cb);
  void set_timer(const float interval, timer_cb_fn_t timer_cb_fn);
  size_t id() const;
  void start();
  void stop();
  void signal();
  void add_fd(const int fd, const async_fd::mode mode,
              fd_cb_fn_t fd_cb_fn);
  void remove_fd(const int fd);
  void set_fd_mode(const int fd, const async_fd::mode mode);
  ev::dynamic_loop & loop();

private:
  void async_callback(ev::async &, int);
  void timer_callback(ev::timer &, int);

private:
  bool stop_;
  size_t id_;
  async_cb_fn_t async_cb_fn_;
  timer_cb_fn_t timer_cb_fn_;
  ev::dynamic_loop loop_;
  ev::async async_;
  std::map<int, std::shared_ptr<real_async_fd>> fds_;
  ev::timer timer_;
};

class real_async_events : public async_events_interface {
public:
  real_async_events(size_t num_loops, async_cb_fn_t cb_fn);
  real_async_events(size_t num_loops, async_cb_fn_t cb_fn,
                    const float interval, timer_cb_fn_t timer_cb_fn);
  void start_loop(const size_t loop_id);
  void signal_loop(const size_t loop_id);
  void stop_all_loops();
  async_loop & loop(const size_t loop_id);

private:
  size_t num_loops_;
  async_cb_fn_t cb_fn_;
  std::vector<real_async_loop> loops_;
};

class listen_io {
public:
  listen_io(const int fd, on_new_client_fn_t on_new_client);

private:
  void io_accept_cb(ev::io & io, int revents);

private:
  ev::io io_;
  on_new_client_fn_t on_new_client_;
};

class real_main_async_loop : public main_async_loop_interface {
public:
  real_main_async_loop();
  void start();
  void add_tcp_listen_fd(const int fd, on_new_client_fn_t on_new_client);

private:
  void signal_cb(ev::sig & signal, int revents);

private:
  ev::default_loop default_loop_;
  ev::sig sig_;
  std::vector<std::shared_ptr<listen_io>> listen_ios_;
};

real_main_async_loop real_loop;

main_async_loop g_main_loop(real_loop);

#endif
