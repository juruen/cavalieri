#ifndef ATOM_TEST_CASE
#define ATOM_TEST_CASE

#include <thread>
#include <atomic>
#include <atom.h>


const int atom_test_case_iterations = 10000;
const int atom_test_case_threads = 4;

TEST(atom_update_test_case, test)
{
  atom<int> atom_int(new int(0));
  auto update_fn = [](const int  i) { return i + 1; };

  auto fn = [&]() mutable
  {
    atom<bool>::attach_thread();
    for (int i = 0; i < atom_test_case_iterations; i++) {
      atom_int.update(update_fn);
    }
    atom<bool>::detach_thread();
  };

  std::vector<std::thread> v;
  for (int i = 0; i < atom_test_case_threads; i++) {
    v.push_back(std::move(std::thread(fn)));
  }

  for (int i = 0; i < atom_test_case_threads; i++) {
    v[i].join();
  }

  int final_count;
  atom_int.safe_read([&](const int & i) { final_count = i; });
  ASSERT_EQ(atom_test_case_iterations * atom_test_case_threads, final_count);
}

TEST(atom_update_success_test_case, test)
{
  atom<int> atom_int(new int(0));
  std::atomic<int> five_count(0);

  auto update_fn = [](const int  i) { return i + 1; };
  auto success_fn = [&](const int i) { if (i % 5 == 0) five_count++;  };

  auto fn = [&]() mutable
  {
    atom<bool>::attach_thread();
    for (int i = 0; i < atom_test_case_iterations; i++) {
      atom_int.update(update_fn, success_fn);
    }
    atom<bool>::detach_thread();
  };

  std::vector<std::thread> v;
  for (int i = 0; i < atom_test_case_threads; i++) {
    v.push_back(std::move(std::thread(fn)));
  }

  for (int i = 0; i < atom_test_case_threads; i++) {
    v[i].join();
  }

  int final_count;
  atom_int.safe_read([&](const int & i) { final_count = i; });
  ASSERT_EQ(atom_test_case_iterations * atom_test_case_threads, final_count);
  ASSERT_EQ( atom_test_case_iterations * atom_test_case_threads / 5, five_count);
}
#endif
