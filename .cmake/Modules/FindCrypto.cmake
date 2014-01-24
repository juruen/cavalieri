# - Try to find Glog, Google Log Library
# Once done, this will define
#
#  Glog_FOUND - system has Glog
#  Glog_INCLUDE_DIRS - the Glog include directories
#  Glog_LIBRARIES - link these to use Glog

INCLUDE(LibFindMacros)

FIND_LIBRARY(Crypto_LIBRARY
  NAMES
    crypto
)


# Set the include dir variables and the libraries and let libfind_process do the rest.
# NOTE: Singular variables for this library, plural for libraries this this lib depends on.
set(Glog_PROCESS_LIBS Crypto_LIBRARY)

LIBFIND_PROCESS(Crypto)
