#ifndef PYTHON_INTERPRETER_H
#define PYTHON_INTERPRETER_H

#include <string>

struct python_interpreter {
  python_interpreter();
  bool run_function(
      const std::string& module_name,
      const std::string& function_name,
      const std::string& arg
  );
  virtual ~python_interpreter();
};

#endif
