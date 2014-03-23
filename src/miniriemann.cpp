#include <glog/logging.h>
#include <atom/atom.h>
#include <core.h>

int main(int argc, char **argv)
{
  atom_initialize();
  {
    ATOM_GC;

    atom_attach_thread();

    google::ParseCommandLineFlags(&argc, &argv, true);

    google::InitGoogleLogging(argv[0]);

    start_core();
  }
  atom_terminate();

  return 0;
}
