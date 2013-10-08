# - Try to find Glog, Google Log Library
# Once done, this will define
#
#  Glog_FOUND - system has Glog
#  Glog_INCLUDE_DIRS - the Glog include directories
#  Glog_LIBRARIES - link these to use Glog

INCLUDE(LibFindMacros)

FIND_PATH(Glog_INCLUDE_DIR
  NAMES glog/logging.h
  HINTS /usr/include
)

FIND_LIBRARY(Glog_LIBRARY
  NAMES glog
)

# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Glog_PROCESS_INCLUDES Glog_INCLUDE_DIR)
set(Glog_PROCESS_LIBS Glog_LIBRARY)

LIBFIND_PROCESS(Glog)
