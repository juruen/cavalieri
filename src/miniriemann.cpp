#include <glog/logging.h>
#include <atom/atom.h>
#include <core.h>

std::shared_ptr<core> g_core;

int main(int, char **argv)
{
  atom_initialize();
  {
    ATOM_GC;

    atom_attach_thread();

    google::InitGoogleLogging(argv[0]);

    g_core = std::make_shared<core>();

    g_core->start();

    g_core.reset();
  }
  atom_terminate();

  return 0;
}
