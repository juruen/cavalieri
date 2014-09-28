#include <glog/logging.h>
#include <unordered_set>
#include <boost/filesystem.hpp>
#include <regex>
#include <dlfcn.h>
#include <iostream>
#include <util/util.h>
#include <core/core.h>
#include <rules_loader.h>

using namespace boost::filesystem;

namespace {

const std::string dl_symbol = "rules";

using lib_data = struct {
  std::string file;
  std::time_t last_write_time;
  void * handle;
  std::shared_ptr<streams_t> stream;
};

std::vector<lib_data> so_files(std::string dir) {

  VLOG(3) << "loading rules";

  std::vector<lib_data> files;
  std::regex pattern(".*\\.so");

  for (directory_iterator iter(dir), end;
       iter != end;
       ++iter)
  {
    std::string file = iter->path().filename().string();
    if (regex_match(file, pattern))
    {
      VLOG(3) << "library found: " << file;
      files.push_back({file, last_write_time(iter->path()), nullptr, nullptr});
    }
  }

  return files;
}

bool load_library(lib_data & lib) {

  void *handle = dlopen(lib.file.c_str(), RTLD_NOW);

  if (!handle) {
    LOG(ERROR) << "error opening " << lib.file << " " << dlerror();
    return false;
  }

  lib.handle = handle;

  typedef streams_t* (*rules_t)();

  rules_t rules = (rules_t) dlsym(handle, dl_symbol.c_str());

  if (!rules) {
    LOG(ERROR) << "failed to load symbol: " << dl_symbol;
    dlclose(handle);
    return false;
  }

  VLOG(3) << "loading rules from " << lib.file;

  lib.stream = std::shared_ptr<streams_t>(rules());

  LOG(INFO) << "rules loaded succesfully from " << lib.file;

  return true;
}

using curr_map_t =
  std::unordered_map<std::string, std::vector<stream_lib>::iterator>;

curr_map_t make_curr_map(std::vector<stream_lib> & current_libs) {
  std::unordered_map<std::string, std::vector<stream_lib>::iterator> curr_map;
  for (auto it = begin(current_libs); it != end(current_libs); it++) {
    if (it->used()) {
      curr_map[it->file] = it;
    }
  }
  return curr_map;
}

bool library_used(const std::string file, const curr_map_t & curr_map) {
    return (curr_map.find(file) != end(curr_map));
}

bool close_library(stream_lib & lib) {

  lib.set_used(false);

  while (lib.ref_counter.load() != 0) {
    LOG(INFO) << "ref counter for " << lib.file << " is not zero";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
  }

  if (!dlclose(lib.handle)) {

    LOG(INFO) << lib.file << " unloaded";

    lib.handle = nullptr;
    return true;

  } else {

    LOG(ERROR) << "failed to unload " << lib.file;
    return false;

  }


}

void load_library(lib_data & lib, stream_lib & curr_lib) {

  if (!load_library(lib)) {
    LOG(ERROR) << "failed to load";
    return;
  }

  init_streams(*lib.stream);

  curr_lib.file = lib.file;
  curr_lib.handle = lib.handle;
  curr_lib.last_write_time = lib.last_write_time;
  curr_lib.stream = lib.stream;

  curr_lib.set_used(true);
}

bool available_unsused_element(const std::vector<stream_lib> & current_libs) {
  for (const auto & lib : current_libs) {
    if (!lib.used()) {
      return true;
    }
  }
  return false;
}

}

void load_rules(const std::string dir, std::vector<stream_lib> & current_libs)
{
  VLOG(3) << "load_rules()++";

  auto curr_map = make_curr_map(current_libs);

  std::unordered_set<std::string> unused;
  for (const auto & p : curr_map) {
    unused.insert(p.first);
  }

  for (auto & lib : so_files(dir)) {
    auto it = unused.find(lib.file);
    if (it != end(unused)) {
      unused.erase(it);
    }

    if (library_used(lib.file, curr_map)) {

      auto & curr_lib = curr_map[lib.file];

      if (curr_lib->last_write_time == lib.last_write_time) {
        VLOG(3) << lib.file << " didn't change";
        continue;
      }

      if (!close_library(*curr_lib)) {
        continue;
      }

      load_library(lib, *curr_lib);

    } else {

      if (!available_unsused_element(current_libs)) {
        LOG(ERROR) << "run out of lib spots";
        continue;
      }

      for (auto & clib : current_libs) {
        if (!clib.used()) {
          load_library(lib, clib);
          break;
        }
      }
    }
  }

  for (const auto & libname : unused) {
    auto & lib = curr_map[libname];
    close_library(*lib);
  }

  VLOG(3) << "load_rules()--";
}
