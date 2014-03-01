FIND_PACKAGE(GTest REQUIRED)
FIND_PACKAGE(Threads REQUIRED)

SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
# Code coverage options
#ADD_DEFINITIONS( -fprofile-arcs -ftest-coverage -g -O0)

SET(MINIRIEMANN_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/src/scheduler/mock_scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/scheduler/scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/mock_os_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/os_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/tcp_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/riemann_tcp_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/ws_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/ws_util.cpp
    ${CMAKE_SOURCE_DIR}/src/streams.cpp
    ${CMAKE_SOURCE_DIR}/src/folds.cpp
    ${CMAKE_SOURCE_DIR}/src/index/index.cpp
    ${CMAKE_SOURCE_DIR}/src/pubsub.cpp
    ${CMAKE_SOURCE_DIR}/src/util.cpp
    ${CMAKE_SOURCE_DIR}/src/query/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/query/scanner.cpp
    ${CMAKE_SOURCE_DIR}/src/query/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/query/expression.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/common.cpp
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
    ${CMAKE_SOURCE_DIR}/tests/basic_test_case.hpp
    ${CMAKE_SOURCE_DIR}/tests/query_grammar_test_case.hpp
    ${CMAKE_SOURCE_DIR}/tests/mock_scheduler_test_case.hpp
    ${CMAKE_SOURCE_DIR}/tests/streams_test_case.hpp
    ${CMAKE_SOURCE_DIR}/tests/folds_test_case.hpp
    ${CMAKE_SOURCE_DIR}/tests/atom_test_case.hpp
    ${CMAKE_SOURCE_DIR}/tests/rules_common_test_case.hpp
    ${CMAKE_SOURCE_DIR}/tests/tcp_connection_test_case.hpp
    ${CMAKE_SOURCE_DIR}/tests/ws_connection_test_case.hpp
    ${CMAKE_SOURCE_DIR}/tests/riemann_tcp_connection_test_case.hpp
  )

TARGET_LINK_LIBRARIES(
    test_plan
    gcov
    gmock
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
    tbb
    cds
  )

INCLUDE(CodeCoverage)

SETUP_TARGET_FOR_COVERAGE(
    test_plan_coverage
    test_plan
    coverage
)
