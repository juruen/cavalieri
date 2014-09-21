# Enable ExternalProject CMake module
include(ExternalProject)

# Add gmock
ExternalProject_Add(
    googlemock
    SVN_REPOSITORY http://googlemock.googlecode.com/svn/tags/release-1.7.0
    UPDATE_COMMAND ""
    TIMEOUT 10
    CMAKE_ARGS -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)

set (ThirdParty_INCLUDE_DIRS
     ${CMAKE_BINARY_DIR}/thirdparty/googlemock-prefix/src/googlemock/gtest/include
     ${CMAKE_BINARY_DIR}/thirdparty/googlemock-prefix/src/googlemock/include
     PARENT_SCOPE)

set (ThirdParty_LIBRARIES
     ${CMAKE_BINARY_DIR}/thirdparty/googlemock-prefix/src/googlemock-build/libgmock.a
     PARENT_SCOPE)
