#include <glog/logging.h>
#include <boost/filesystem.hpp>
#include <regex>
#include <dlfcn.h>
#include <iostream>
#include <util.h>
#include <core/core.h>
#include <rules_loader.h>

using namespace boost::filesystem;

namespace {

const std::string dl_symbol = "rules";

std::vector<std::string> so_files(std::string dir) {

  VLOG(3) << "loading rules";

  std::vector<std::string> files;
  std::regex pattern(".*\\.so");

  for (directory_iterator iter(dir), end;
       iter != end;
       ++iter)
  {
    std::string fn = iter->path().filename().string();
    if (regex_match(fn, pattern))
    {
      VLOG(3) << "library found: " << fn;
      files.push_back(fn);
    }
  }

  return files;
}

std::shared_ptr<streams_t> load_library(std::string lib) {

  void *handle = dlopen(lib.c_str(), RTLD_LAZY);

  if (!handle) {
    LOG(ERROR) << "error opening " << lib << " " << dlerror();
    return nullptr;
  }

  typedef streams_t* (*rules_t)();

  rules_t rules = (rules_t) dlsym(handle, dl_symbol.c_str());

  if (!rules) {
    LOG(ERROR) << "failed to load symbol: " << dl_symbol;
    dlclose(handle);
    return nullptr;
  }

  VLOG(3) << "loading rules from " << lib;

  auto stream = rules();

  LOG(INFO) << "rules loaded succesfully from " << lib;

  dlclose(handle);

  return std::shared_ptr<streams_t>(stream);
}

}

std::vector<std::shared_ptr<streams_t>> load_rules(const std::string dir) {

  std::vector<std::shared_ptr<streams_t>> rules;

  for (const auto & lib : so_files(dir)) {

    std::shared_ptr<streams_t> stream = load_library(lib);

    if (stream) {
      rules.push_back(stream);
     }

  }

  return rules;

}

