#include <atom/atom.h>
#include <core.h>

int main(int argc, char **argv)
{
  atom_initialize();
  {
    ATOM_GC;

    atom_attach_thread();

    start_core(argc, argv);
  }
  atom_terminate();

  return 0;
}
