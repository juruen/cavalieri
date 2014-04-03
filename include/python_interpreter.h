#ifndef PYTHON_INTERPRETER_H
#define PYTHON_INTERPRETER_H

#include <string>
#include <vector>
#include <mutex>

struct python_interpreter {
  python_interpreter();
  bool run_function(const std::string& module_name,
                    const std::string& function_name,
                    const std::string& arg);
  virtual ~python_interpreter();
};

class python_runner {
public:
  void run_function(const std::string& module_name,
                    const std::string& function_name,
                    const std::string& arg);
private:
  python_interpreter interpreter_;
  std::mutex mutex_;
};

extern python_runner g_python_runner;
#endif
