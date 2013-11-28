FIND_PACKAGE(GTest REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

# Code coverage options
#ADD_DEFINITIONS( -fprofile-arcs -ftest-coverage -g -O0)

SET(MINIRIEMANN_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/src/util.cpp
  )

SET(MINIRIEMANN_PROTOFILES
    ${CMAKE_SOURCE_DIR}/src/proto.proto
  )

PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${MINIRIEMANN_PROTOFILES})

INCLUDE_DIRECTORIES(
    ${GTEST_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}
    ${MINIRIEMANN_HDRS}
    ${PROTOBUF_INCLUDE_DIR}
    ${Glog_INCLUDE_DIR}
    ${LibEv_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS}
  )

ADD_EXECUTABLE(
    test_plan
    ${MINIRIEMANN_TEST_SRCS}
    ${ProtoSources}
    ${ProtoHeaders}
    ${CMAKE_SOURCE_DIR}/tests/test_plan.cpp
  )

GTEST_ADD_TESTS(
    test_plan
    NONE
    ${CMAKE_SOURCE_DIR}/tests/basic_test_case.hpp
  )

TARGET_LINK_LIBRARIES(
    test_plan
    gcov
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${PROTOBUF_LIBRARY}
    ${LibEv_LIBRARIES}
    ${Glog_LIBRARIES}
    ${PYTHON_LIBRARIES}
    ssl
    crypto
    curl
    pthread
  )
