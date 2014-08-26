#include <netinet/in.h>
#include <glog/logging.h>
#include <async/real_async_loop.h>

namespace {

int ev_mode(const async_fd::mode & initial_mode) {
  int mode = 0;
  if (initial_mode == async_fd::read) {
    mode |= EV_READ;
  }
  if (initial_mode == async_fd::write) {
    mode |= EV_WRITE;
  }
  if (initial_mode == async_fd::readwrite) {
    mode |= EV_WRITE | EV_READ;
  }
  return  mode;
}

};

real_async_fd::real_async_fd(
    int fd,
    async_fd::mode initial_mode,
    real_async_loop & loop,
    fd_cb_fn_t cb
) :
  async_fd(),
  async_loop_(loop),
  io_(new ev::io(loop.loop())),
  fd_(fd),
  error_(false),
  read_(false),
  write_(false),
  fd_cb_fn_(cb)
{
  io_->set(loop.loop());
  io_->set<real_async_fd, &real_async_fd::async_cb>(this);
  io_->start(fd, ev_mode(initial_mode));
}

int real_async_fd::fd() const {
  return fd_;
}

bool real_async_fd::error() const {
  return error_;
}

void real_async_fd::stop() {
  async_loop_.remove_fd(fd_);
}

void real_async_fd::async_cb(ev::io &, int revents) {
  VLOG(3) << "async_cb() events: " << revents;
  error_ = EV_ERROR & revents;
  if (error_)
    VLOG(3) << "EV_ERROR";
  read_ = EV_READ & revents;
  write_ = EV_WRITE & revents;
  fd_cb_fn_(*this);
}

bool real_async_fd::ready_read() const {
  return read_;
}

bool real_async_fd::ready_write() const {
  return write_;
}

void real_async_fd::set_mode(const async_fd::mode& mode) {
  io_->set(ev_mode(mode));
}

real_async_fd::~real_async_fd() {
  VLOG(3) << "~real_async_fd() " << fd_;
  io_->stop();
  close(fd_);
}

async_loop& real_async_fd::loop() {
  return async_loop_;
}

real_async_loop::real_async_loop() : async_loop(), stop_(false)
{
  async_.set(loop_);
  async_.set<real_async_loop, &real_async_loop::async_callback>(this);
}

void real_async_loop::set_id(size_t id) {
  id_ = id;
}

void real_async_loop::set_async_cb(async_cb_fn_t async_cb) {
  async_cb_fn_ = async_cb;
}


size_t real_async_loop::id() const {
  return id_;
}

void real_async_loop::async_callback(ev::async&, int) {
  if (!stop_) {
    CHECK(async_cb_fn_)  << "async_cb_fn_ not set";
    async_cb_fn_(*this);
  } else {
    loop_.unloop();
  }
}

void real_async_loop::start() {
  async_.start();
  loop_.run();
}

void real_async_loop::stop() {
  stop_ = true;
  async_.send();
}

void real_async_loop::signal() {
  async_.send();
}

void real_async_loop::add_fd(const int fd, const async_fd::mode mode,
                             fd_cb_fn_t fd_cb_fn)
{
  fds_.insert({fd, std::make_shared<real_async_fd>(fd, mode, *this, fd_cb_fn)});
}

void real_async_loop::remove_fd(const int fd)
{
  auto it = fds_.find(fd);
  if (it != fds_.end()) {
    fds_.erase(fd);
  }
}

void real_async_loop::set_fd_mode(const int fd, const async_fd::mode mode) {
  auto it = fds_.find(fd);
  it->second->set_mode(mode);
}

ev::dynamic_loop & real_async_loop::loop() {
  return loop_;
}

timer_id_t real_async_loop::add_timer(const timer_cb_fn_t task,
                                      const bool once, const float t)
{
  auto timer_id = next_timer_id_++;

  auto  fn = [=]()
  {
    task(id_);

    if (once) { // Move this to static fun
      auto it = timers_.find(timer_id);
      CHECK(it != timers_.end()) << "timer not found";
      timers_.erase(it);
    }
  };

  timer_ctx_t ctx{{std::make_shared<ev::timer>(loop_)},
                   std::make_shared<task_cb_fn_t>(fn),
                   *this,
                   {id_, timer_id},
                   once};

  auto it = timers_.insert({timer_id, std::make_shared<timer_ctx_t>(ctx)});
  auto ev_timer = it.first->second->timer;

  auto ctx_ptr = static_cast<void*>(it.first->second.get());
  ev_timer->set<timer_callback>(ctx_ptr);

  ev_timer->start(t, once ? 0 : t);

  return {id_, timer_id};
}

void real_async_loop::timer_callback(ev::timer & timer, int) {

  VLOG(3) << "timer_callback()";

  auto ctx_ptr = static_cast<timer_ctx_t*>(timer.data);

  (*ctx_ptr->task)();

  if (ctx_ptr->once) {
    ctx_ptr->async_loop.remove_task(ctx_ptr->id);
  }

}

timer_id_t real_async_loop::add_once_task(const timer_cb_fn_t task,
                                          const float t) {
  VLOG(3) << "add_once_task() t: " << t;

  return add_timer(task, true, t);

}

timer_id_t real_async_loop::add_periodic_task(const timer_cb_fn_t task,
                                              const float t) {
  VLOG(3) << "add_periodic_task() t: " << t;

  return add_timer(task, false, t);

}

void real_async_loop::set_task_interval(const timer_id_t id, const float t) {

  VLOG(3) << "set_task_interval() t: " << t;
  return;

  auto it = timers_.find(id.timer_id);
  CHECK(it != timers_.end()) << "timer not found";

  auto & ev_timer = it->second->timer;
  ev_timer->start(t, t);

}

bool real_async_loop::remove_task(const timer_id_t id) {

  VLOG(3) << "remove_task()";

  auto it = timers_.find(id.timer_id);

  if (it != timers_.end()) {

   it->second->timer->stop();
   timers_.erase(it);

  return true;

  } else {

    VLOG(3) << "timer not found";
    return false;
  }

}

real_async_events::real_async_events(size_t num_loops, async_cb_fn_t cb_fn) :
  async_events_interface(),
  num_loops_(num_loops),
  cb_fn_(cb_fn),
  loops_(num_loops)
{
  for (size_t i = 0; i < num_loops_; i++) {
    loops_[i].set_id(i);
    loops_[i].set_async_cb(cb_fn);
  }
}

void real_async_events::start_loop(const size_t loop_id) {
  loops_[loop_id].start();
}

void real_async_events::stop_all_loops() {
  for (auto & loop: loops_) {
    loop.stop();
  }
}

void real_async_events::signal_loop(const size_t loop_id) {
  loops_[loop_id].signal();
}

async_loop & real_async_events::loop(const size_t loop_id) {
  return loops_[loop_id];
}

std::unique_ptr<async_events_interface> make_async_events(size_t threads,
                                                          async_cb_fn_t cb)
{
  return std::move(std::unique_ptr<real_async_events>(
                    new real_async_events(threads, cb)));
}

listen_io::listen_io(const int fd, on_new_client_fn_t on_new_client)
  :
  on_new_client_(on_new_client)
{
  io_.set<listen_io, &listen_io::io_accept_cb>(this);
  io_.start(fd, ev::READ);
}

void listen_io::io_accept_cb(ev::io & io, int revents) {

  if (EV_ERROR & revents) {
    VLOG(3) << "got invalid event: " << strerror(errno);
    return;
  }

  struct sockaddr_in client_addr;

  socklen_t client_addr_len = sizeof(client_addr);
  int fd = accept(io.fd, reinterpret_cast<struct sockaddr *>(&client_addr),
                  &client_addr_len);

  if (fd < 0) {
    VLOG(3) << "accept error: " << strerror(errno);
    return;
  }


  on_new_client_(fd);
}

timer_io::timer_io(task_cb_fn_t task, float interval)
:
  timer_(),
  task_fn_(task)
{
  timer_.set<timer_io, &timer_io::timer>(this);
  timer_.start(0, interval);
}

void timer_io::timer(ev::timer &, int) {
  task_fn_();
}


real_main_async_loop::real_main_async_loop()
:
  default_loop_(),
  sig_(default_loop_),
  async_(),
  listen_ios_(),
  timer_ios_(),
  new_tasks_()
{
  sig_.set<real_main_async_loop, &real_main_async_loop::signal_cb>(this);
  sig_.start(SIGINT);

  async_.set<real_main_async_loop, &real_main_async_loop::async_cb>(this);
  async_.start();

}

void real_main_async_loop::start() {
  default_loop_.run(0);
}

void real_main_async_loop::add_tcp_listen_fd(const int fd,
                                             on_new_client_fn_t on_new_client) {
  listen_ios_.emplace_back(std::make_shared<listen_io>(fd, on_new_client));
}

void real_main_async_loop::signal_cb(ev::sig &, int) {
  default_loop_.break_loop();
}

void real_main_async_loop::add_periodic_task(task_cb_fn_t task,
                                             float interval)
{
  std::lock_guard<std::mutex> lock(mutex_);

  new_tasks_.push({task, interval});

  async_.send();
}

void real_main_async_loop::async_cb(ev::async &, int) {

  std::lock_guard<std::mutex> lock(mutex_);

  while (!new_tasks_.empty()) {
    auto p = new_tasks_.front();
    new_tasks_.pop();

    timer_ios_.push_back(std::make_shared<timer_io>(p.first, p.second));
  }

}


std::unique_ptr<main_async_loop_interface> make_main_async_loop() {

  std::unique_ptr<real_main_async_loop> real_main(new real_main_async_loop());
  return std::move(real_main);

}
