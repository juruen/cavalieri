#ifndef ASYNC_ASYNC_LOOP_H
#define ASYNC_ASYNC_LOOP_H

#include <functional>
#include <memory>
#include <cstddef>

class async_loop;

class async_fd {
public:
  enum mode {
    none,
    read,
    write,
    readwrite
  };
  virtual int fd() const = 0;
  virtual bool error() const = 0;
  virtual void stop() = 0;
  virtual bool ready_read() const = 0;
  virtual bool ready_write() const = 0;
  virtual void set_mode(const mode&) =  0;
  virtual async_loop & loop() = 0;
};

typedef std::function<void(async_fd&)> fd_cb_fn_t;
typedef std::function<void(async_loop&)> timer_cb_fn_t;
typedef std::function<void()> task_cb_fn_t;

class async_loop {
public:
  virtual size_t id() const = 0;
  virtual void start() = 0;
  virtual void stop() = 0;
  virtual void signal() = 0;
  virtual void add_fd(const int fd, const async_fd::mode mode,
                      fd_cb_fn_t fd_cb_fn) = 0;
  virtual void remove_fd(const int fd) = 0;
  virtual void set_fd_mode(const int fd, const async_fd::mode mode) = 0;
  virtual void set_timer_interval(const float t) = 0;
};

typedef std::function<void(async_loop&)> async_cb_fn_t;

class async_events_interface {
public:
  virtual void start_loop(size_t loop_id) = 0;
  virtual void signal_loop(size_t loop_id) = 0;
  virtual void stop_all_loops() = 0;
  virtual async_loop & loop(size_t loop_id) = 0;
  virtual ~async_events_interface() {};
};

std::unique_ptr<async_events_interface> make_async_events(size_t,
                                                          async_cb_fn_t);

std::unique_ptr<async_events_interface> make_async_events(size_t,
                                                          async_cb_fn_t,
                                                          const float,
                                                          timer_cb_fn_t);

typedef std::function<void(const int fd)> on_new_client_fn_t;
typedef std::function<void(const int signal)> on_signal_fn_t;

class main_async_loop_interface {
public:
  virtual void start() = 0;
  virtual void add_tcp_listen_fd(const int fd, on_new_client_fn_t fn) = 0;
  virtual void add_periodic_task(task_cb_fn_t task, float interval) = 0;
  virtual ~main_async_loop_interface() {};
};

std::unique_ptr<main_async_loop_interface> make_main_async_loop();

#endif
