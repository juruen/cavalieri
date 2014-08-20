#ifndef CAVALIERI_WEBSOCKET_WORKER_THREADS_H
#define CAVALIERI_WEBSOCKET_WORKER_THREADS_H

#include <vector>
#include <functional>
#include <thread>

class worker_threads {
public:
  using task_t = std::function<void()>;

  worker_threads(const size_t threads, const task_t task);
  void stop();

  void run_tasks(const int i);

private:

  task_t task_;
  std::vector<int> finished_threads_;
  std::vector<std::thread> threads_;

};

#endif
