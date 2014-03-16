#ifndef ATOM_H
#define ATOM_H

#include <unordered_map>
#include <memory>

#define USE_LIBCDS

#ifdef USE_LIBCDS
#include <atom_cds.h>
#else
#include <atom_cds.h>
#endif

template <class T>
class atom {
public:

  atom(T* ptr) : atom_(ptr) {};
  atom() : atom_(new T()) {};

  void update(std::function<T(const T&)> update_fn) {
    atom_.update(update_fn);
  }

  void update(const T& t, std::function<void(const T&, const T&)> success_fn) {
    atom_.update(t, success_fn);
  }

  void update(
      std::function<T(const T&)> update_fn,
      std::function<void(const T&, const T&)> success_fn
  )
  {
    atom_.update(update_fn, success_fn);
  }

#ifdef USE_LIBCDS
  atom_cds<T> & atom_impl() {
    return atom_;
  }
#else
  atom_cds<T> & atom_impl() {
    return atom_;
  }
#endif
private:

#ifdef USE_LIBCDS
  atom_cds<T> atom_;
#else
  atom_cds<T> atom_;
#endif

};

inline void atom_attach_thread() {
#ifdef USE_LIBCDS
    atom_attach_thread_cds();
#else
    atom_attach_thread_cds();
#endif
}

inline void atom_detach_thread() {
#ifdef USE_LIBCDS
    atom_detach_thread_cds();
#else
    atom_detach_thread_cds();
#endif
}


template <class T>
std::shared_ptr<atom<T>> make_shared_atom() {
  std::shared_ptr<atom<T>> s(new atom<T>(new T()));
  return s;
}

template <class T>
std::shared_ptr<atom<T>> make_shared_atom(const T t) {
  std::shared_ptr<atom<T>> s(new atom<T>(new T(t)));
  return s;
}

template <class K, class V>
void map_on_sync_insert(
  std::shared_ptr<atom<std::unordered_map<K,V>>> & m,
  K k,
  std::function<V()> fn_value,
  std::function<void(V&)> fn_inserted
)
{
#ifdef USE_LIBCDS
  map_on_sync_insert_cds<K,V>(m->atom_impl(), k, fn_value, fn_inserted);
#else
  map_on_sync_insert_cds<K,V>(m->atom_impl(), k, fn_value, fn_inserted);
#endif
}

#endif
