include(LibFindMacros)

libfind_pkg_check_modules(JsonCpp_PKGCONF jsoncpp)

find_path(JsonCpp_INCLUDE_DIR
    NAMES json/features.h
    PATH_SUFFIXES jsoncpp
    PATHS ${JsonCpp_PKGCONF_INCLUDE_DIRS} # /usr/include/jsoncpp/json
  )

find_library(JsonCpp_LIBRARY
    NAMES jsoncpp
    PATHS ${JsonCpp_PKGCONF_LIBRARY_DIRS}
  )

set(JsonCpp_PROCESS_INCLUDES JsonCpp_INCLUDE_DIR)
set(JsonCpp_PROCESS_LIBS JsonCpp_LIBRARY)
libfind_process(JsonCpp)
