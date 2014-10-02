#ifndef CAVALIERI_ASYNC_REAL_ASYNC_LOOP_H
#define CAVALIERI_ASYNC_REAL_ASYNC_LOOP_H

#include <async/async_loop.h>
#include <ev++.h>
#include <vector>
#include <mutex>
#include <queue>
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
  ~real_async_fd();
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
  size_t id() const;
  void start();
  void stop();
  void signal();
  void add_fd(const int fd, const async_fd::mode mode,
              fd_cb_fn_t fd_cb_fn);
  void remove_fd(const int fd);
  void set_fd_mode(const int fd, const async_fd::mode mode);
  ev::dynamic_loop & loop();
  timer_id_t add_once_task(const timer_cb_fn_t, const float t);
  timer_id_t add_periodic_task(const timer_cb_fn_t, const float t);
  void set_task_interval(const timer_id_t, const float t);
  bool remove_task(const timer_id_t);

private:
  using sched_task_t =  struct {
    task_cb_fn_t task;
    timer_id_t id;
    unsigned long interval_ms;
    unsigned long time_ms;
  };

  class sched_task_cmp
  {
    public:
      bool operator() (const sched_task_t & lhs, const sched_task_t& rhs) const
      {
        return (lhs.time_ms > rhs.time_ms);
      }
  };


  using fd_ctx_t = std::map<int, std::shared_ptr<real_async_fd>>;

private:
  void async_callback(ev::async &, int);
  void timer_callback(ev::timer &, int);
  timer_id_t add_task(const timer_cb_fn_t, bool, float);
  void sched_next_task();

private:
  bool stop_;
  size_t id_;
  async_cb_fn_t async_cb_fn_;
  ev::dynamic_loop loop_;
  ev::async async_;
  ev::timer timer_;
  fd_ctx_t fds_;
  std::priority_queue<sched_task_t,
                      std::vector<sched_task_t>,
                      sched_task_cmp> tasks_;
  uint64_t next_timer_id_;
};

class real_async_events : public async_events_interface {
public:
  real_async_events(size_t num_loops, async_cb_fn_t cb_fn);
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

class timer_io {
public:
  timer_io(task_cb_fn_t task, float interval);

private:
  void timer(ev::timer & timer, int revents);

private:
  ev::timer timer_;
  task_cb_fn_t task_fn_;
};

class real_main_async_loop : public main_async_loop_interface {
public:
  real_main_async_loop();
  void start();
  void add_tcp_listen_fd(const int fd, on_new_client_fn_t on_new_client);
  void add_periodic_task(task_cb_fn_t task, float interval);

private:
  void signal_cb(ev::sig & signal, int revents);
  void async_cb(ev::async &, int);

private:
  ev::default_loop default_loop_;
  ev::sig sigint_;
  ev::sig sigterm_;
  ev::async async_;
  std::vector<std::shared_ptr<listen_io>> listen_ios_;
  std::vector<std::shared_ptr<timer_io>> timer_ios_;
  std::queue<std::pair<task_cb_fn_t, float>> new_tasks_;
  std::mutex mutex_;
};

#endif
