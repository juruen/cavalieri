# - Try to find LibEv, Google Log Library
# Once done, this will define
#
#  LibEv_FOUND - system has LibEv
#  LibEv_INCLUDE_DIRS - the LibEv include directories
#  LibEv_LIBRARIES - link these to use LibEv

INCLUDE(LibFindMacros)

FIND_PATH(LibEv_INCLUDE_DIR
  NAMES ev++.h
  HINTS /usr/include
)

FIND_LIBRARY(LibEv_LIBRARY
  NAMES ev
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(LibEv_PROCESS_INCLUDES LibEv_INCLUDE_DIR)
set(LibEv_PROCESS_LIBS LibEv_LIBRARY)

LIBFIND_PROCESS(LibEv)
