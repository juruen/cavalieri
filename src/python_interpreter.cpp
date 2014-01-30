#include <Python.h>
#include <glog/logging.h>
#include <python_interpreter.h>

static void del_pyobjs(std::vector<PyObject*> objects) {
  for (auto o: objects) {
    if (o == 0) {
      Py_DECREF(o);
    }
  }
}

python_interpreter::python_interpreter() {
  Py_Initialize();
}

bool python_interpreter::run_function(
    const std::string& module_name,
    const std::string& function_name,
    const std::string& arg)
{
  PyObject *p_module_name = PyString_FromString(module_name.c_str());
  PyObject *p_module = PyImport_Import(p_module_name);

  if (p_module == 0) {
    LOG(ERROR) << "failed to load python module: " << module_name;
    del_pyobjs({p_module_name});
    return false;
  }

  PyObject *p_dict = PyModule_GetDict(p_module);

  if (p_dict == 0) {
    LOG(ERROR) << "failed to PyModule_GetDict(p_module)";
    del_pyobjs({p_module_name, p_module});
    return false;
  }

  PyObject *p_func = PyDict_GetItemString(p_dict, function_name.c_str());

  if (!PyCallable_Check(p_func)) {
    LOG(ERROR) << "function: " << function_name << " in module " << module_name
               << " is not callable";
    del_pyobjs({p_module_name, p_module, p_func});
    PyErr_Print();
    return false;
  }

  PyObject *p_args = PyTuple_New(1);
  PyObject *p_value = PyString_FromString(arg.c_str());
  PyTuple_SetItem(p_args, 0, p_value);

  p_value = PyObject_CallObject(p_func, p_args);

  if (p_value == 0) {
    LOG(ERROR) << "function: " << function_name << " in module " << module_name
               << " failed";
    PyErr_Print();
    del_pyobjs({p_module_name, p_module, p_func, p_args});
    return false;
  }

  LOG(ERROR) << "function: " << function_name << " in module " << module_name
             << " succeded";

  del_pyobjs({p_module_name, p_module, p_func, p_args});

  return true;
}

python_interpreter::~python_interpreter() {
  Py_Finalize();
}

void python_runner::run_function(const std::string& module_name,
                                 const std::string& function_name,
                                 const std::string& arg)
{
  mutex_.lock();
  interpreter_.run_function(module_name, function_name, arg);
  mutex_.unlock();
}

python_runner g_python_runner{};
