# Enable ExternalProject CMake module
include(ExternalProject)

# Add gmock
ExternalProject_Add(
    googlemock
    URL ${CMAKE_SOURCE_DIR}/thirdparty/googlemock
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
