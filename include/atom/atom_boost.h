#ifndef ATOM_BOOST_H
#define ATOM_BOOST_H

#include <functional>
#include <boost/shared_ptr.hpp>
#include <boost/make_shared.hpp>

template <class T>
class atom_boost {
public:

  atom_boost(T* ptr) : ptr_(ptr) {};
  atom_boost() : ptr_(new T()) {};

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

    boost::shared_ptr<T> old_val, new_val;

    do {

      old_val = atomic_load(&ptr_);
      new_val = boost::make_shared<T>(update_fn(*old_val));

    } while(!boost::atomic_compare_exchange<T>(&ptr_, &old_val, new_val));

    if (success_fn) {
      success_fn(*old_val, *new_val);
    }

  }

  void safe_read(std::function<void(const T&)> fn) {
    auto curr = atomic_load(&ptr_);
    fn(*curr);
  }

  boost::shared_ptr<T> & ref_boost() {
    return ptr_;
  }

private:

  boost::shared_ptr<T> ptr_;
};

inline void atom_attach_thread_boost() { }

inline void atom_detach_thread_boost() { }

inline void atom_initialize_boost() { }

inline void atom_terminate_boost() { }

#define ATOM_GC_BOOST (void)(0)

template <class K, class V>
void map_on_sync_insert_boost(
  atom_boost<std::unordered_map<K,V>> & m,
  K k,
  std::function<V()> fn_value,
  std::function<void(V&)> fn_inserted
)
{

  using T = std::unordered_map<K, V>;

  boost::shared_ptr<T> nptr, optr;
  auto & oref = m.ref_boost();

  do {

    oref = m.ref_boost();
    optr = atomic_load(&oref);

    auto it = optr->find(k);

    if (it != optr->end()) {
      fn_inserted(it->second);
      return;
    }

    nptr = boost::make_shared<T>(*optr);
    nptr->insert({k, fn_value()});

  } while (!boost::atomic_compare_exchange<T>(&oref, &optr, nptr));

  auto it = nptr->find(k);
  fn_inserted(it->second);

}

#endif
