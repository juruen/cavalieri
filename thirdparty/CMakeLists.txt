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

# Detect arch to build libcds
INCLUDE (CheckTypeSize)
check_type_size("void *" SIZEOF_VOID_P)
IF( ${SIZEOF_VOID_P} STREQUAL  "8" )
    set (CDS_SYSTEM_PROCESSOR "amd64")
    #set (CDS_LIB_DIR "gcc-amd64-linux-0")
    set (CDS_LIB_DIR "gcc-x86_64-linux-0")
    MESSAGE( STATUS "64 bit architecture detected size of void * is " ${SIZEOF_VOID_P})
ELSE()
    set (CDS_SYSTEM_PROCESSOR "x86")
    set (CDS_LIB_DIR "gcc-x86-linux-0")
    MESSAGE( STATUS "32 bit architecture detected size of void * is " ${SIZEOF_VOID_P})
ENDIF()

# Add cds
ExternalProject_Add(
    cds
    SVN_REPOSITORY  http://svn.code.sf.net/p/libcds/code/tags/Release_1.5.0
    TIMEOUT 10
    UPDATE_COMMAND ""
    PATCH_COMMAND COMMAND svn patch ${CMAKE_SOURCE_DIR}/thirdparty/cds-no-test.patch ${CMAKE_BINARY_DIR}/thirdparty/cds-prefix/src/cds
    CONFIGURE_COMMAND ""
    CMAKE_COMMAND ""
    BUILD_COMMAND /bin/bash -c "cd ${CMAKE_BINARY_DIR}/thirdparty/cds-prefix/src/cds/build && chmod +x build.sh && EXTRA_CXXFLAGS=-fPIC ./build.sh -p ${CDS_SYSTEM_PROCESSOR}"
    TIMEOUT 10
    INSTALL_COMMAND ""
    LOG_DOWNLOAD ON
    LOG_CONFIGURE ON
    LOG_BUILD ON)

set (ThirdParty_INCLUDE_DIRS
     ${CMAKE_BINARY_DIR}/thirdparty/googlemock-prefix/src/googlemock/gtest/include
     ${CMAKE_BINARY_DIR}/thirdparty/googlemock-prefix/src/googlemock/include
     ${CMAKE_BINARY_DIR}/thirdparty/cds-prefix/src/cds
     PARENT_SCOPE)

set (ThirdParty_LIBRARIES
     ${CMAKE_BINARY_DIR}/thirdparty/cds-prefix/src/cds/bin/${CDS_LIB_DIR}/libcds.so
     ${CMAKE_BINARY_DIR}/thirdparty/googlemock-prefix/src/googlemock-build/libgmock.a
     PARENT_SCOPE)

set (libcds_LIBS
     ${CMAKE_BINARY_DIR}/thirdparty/cds-prefix/src/cds/bin/${CDS_LIB_DIR}/libcds.so
     ${CMAKE_BINARY_DIR}/thirdparty/cds-prefix/src/cds/bin/${CDS_LIB_DIR}/libcds.so.1.5.0
     PARENT_SCOPE)

set (libcds_INCLUDES
     ${CMAKE_BINARY_DIR}/thirdparty/cds-prefix/src/cds/cds
     PARENT_SCOPE)

