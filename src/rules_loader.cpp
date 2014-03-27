#include <glog/logging.h>
#include <boost/filesystem.hpp>
#include <regex>
#include <dlfcn.h>
#include <iostream>
#include <core.h>
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

  VLOG(3) << "rules loaded succesfully";

  dlclose(handle);

  std::shared_ptr<streams_t> sh_stream;
  if (stream) {
    sh_stream = std::make_shared<streams_t>(*stream);
    delete stream;
  }

  return sh_stream;
}

}

void load_rules(const std::string dir) {

  for (const auto & lib : so_files(dir)) {

    g_core->add_stream(load_library(lib));

  }

}

