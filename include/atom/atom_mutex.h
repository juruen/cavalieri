#ifndef CAVALIERI_ATOM_MUTEX_H
#define CAVALIERI_ATOM_MUTEX_H

#include <unordered_map>
#include <memory>
#include <atomic>
#include <mutex>

template <class T>
class atom_mutex {
public:

  atom_mutex(T* ptr) : ptr_(ptr) {};
  atom_mutex() : ptr_(new T()) {};

  void update(std::function<T(const T&)> update_fn) {
    update_(update_fn, {});
  }

  void update(const T& t, std::function<void(const T&, const T&)> success_fn) {
    update_([&](const T&) { return t; }, success_fn);
  }

  void update(
    std::function<T(const T&)> update_fn,
    std::function<void(const T&, const T&)> success_fn
  )
  {
    update_(update_fn, success_fn);
  }

  void update_(
    std::function<T(const T&)> update_fn,
    std::function<void(const T&, const T&)> success_fn
  )
  {
    std::lock_guard<std::mutex> lock(mutex_);

    T old(*ptr_);
    ptr_.reset(new T(update_fn(*ptr_)));

    if (success_fn) {
      success_fn(old, *ptr_);
    }

  }

  void safe_read(std::function<void(const T&)> fn) {
    std::lock_guard<std::mutex> lock(mutex_);
    fn(*ptr_);
  }

  std::pair<std::shared_ptr<T>, std::mutex&> ptr_mutex() {
    return {ptr_, mutex_};
  }

private:

  std::shared_ptr<T> ptr_;
  std::mutex mutex_;
};

inline void atom_attach_thread_mutex() { }

inline void atom_detach_thread_mutex() { }

inline void atom_initialize_mutex() { }

inline void atom_terminate_mutex() { }

#define ATOM_GC_MUTEX (void)(0)

template <class K, class V>
void map_on_sync_insert_mutex(
  atom_mutex<std::unordered_map<K,V>> & m,
  K k,
  std::function<V()> fn_value,
  std::function<void(V&)> fn_inserted
)
{
  auto ptr_mutex = m.ptr_mutex();

  std::lock_guard<std::mutex> lock(ptr_mutex.second);

  const auto it = ptr_mutex.first->find(k);
  if (it == ptr_mutex.first->end()) {

    const auto new_it = ptr_mutex.first->insert({k, fn_value()});
    fn_inserted(new_it.first->second);

  } else {

    fn_inserted(it->second);

  }

}

#endif
