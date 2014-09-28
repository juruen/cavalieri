FIND_PACKAGE(Threads REQUIRED)

SET(CMAKE_CXX_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
SET(CMAKE_C_FLAGS "-g -O0 -fprofile-arcs -ftest-coverage")
# Code coverage options
#ADD_DEFINITIONS( -fprofile-arcs -ftest-coverage -g -O0)

SET(CAVALIERI_TEST_SRCS
    ${CMAKE_SOURCE_DIR}/src/scheduler/mock_scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/common/event.cpp
    ${CMAKE_SOURCE_DIR}/src/core/core.cpp
    ${CMAKE_SOURCE_DIR}/src/core/mock_core.cpp
    ${CMAKE_SOURCE_DIR}/src/index/mock_index.cpp
    ${CMAKE_SOURCE_DIR}/src/external/mock_external.cpp
    ${CMAKE_SOURCE_DIR}/src/os/mock_os_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/os/os_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/tcp_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/riemann_tcp_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/ws_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/ws_util.cpp
    ${CMAKE_SOURCE_DIR}/src/predicates/predicates.cpp
    ${CMAKE_SOURCE_DIR}/src/rules_loader.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/lib.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_infra.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_functions_lock.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/instrumentation.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/rate.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/reservoir.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/gauge.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/mem.cpp
    ${CMAKE_SOURCE_DIR}/src/pub_sub/pub_sub.cpp
    ${CMAKE_SOURCE_DIR}/src/folds/folds.cpp
    ${CMAKE_SOURCE_DIR}/src/index/real_index.cpp
    ${CMAKE_SOURCE_DIR}/src/pub_sub/pub_sub.cpp
    ${CMAKE_SOURCE_DIR}/src/util/util.cpp
    ${CMAKE_SOURCE_DIR}/src/query/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/query/scanner.cpp
    ${CMAKE_SOURCE_DIR}/src/query/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/query/expression.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/common.cpp
  )

SET(CAVALIERI_PROTOFILES
    ${CMAKE_SOURCE_DIR}/src/proto.proto
  )

PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${CAVALIERI_PROTOFILES})

INCLUDE_DIRECTORIES(
    ${GTEST_INCLUDE_DIRS}
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}
    ${CAVALIERI_HDRS}
    ${PROTOBUF_INCLUDE_DIR}
    ${Glog_INCLUDE_DIR}
    ${LibEv_INCLUDE_DIR}
    ${ThirdParty_INCLUDE_DIRS}
    ${JsonCpp_INCLUDE_DIRS}
  )

ADD_EXECUTABLE(
    test_plan
    ${CAVALIERI_TEST_SRCS}
    ${ProtoSources}
    ${ProtoHeaders}
    ${CMAKE_SOURCE_DIR}/tests/test_plan.cpp
  )

#GTEST_ADD_TESTS(
#    test_plan
#    ${CMAKE_SOURCE_DIR}/tests/basic_test_case.hpp
#    ${CMAKE_SOURCE_DIR}/tests/query_grammar_test_case.hpp
#    ${CMAKE_SOURCE_DIR}/tests/mock_scheduler_test_case.hpp
#    ${CMAKE_SOURCE_DIR}/tests/streams_test_case.hpp
#    ${CMAKE_SOURCE_DIR}/tests/folds_test_case.hpp
#    ${CMAKE_SOURCE_DIR}/tests/atom_test_case.hpp
#    ${CMAKE_SOURCE_DIR}/tests/rules_common_test_case.hpp
#    ${CMAKE_SOURCE_DIR}/tests/tcp_connection_test_case.hpp
#    ${CMAKE_SOURCE_DIR}/tests/ws_connection_test_case.hpp
#    ${CMAKE_SOURCE_DIR}/tests/riemann_tcp_connection_test_case.hpp
#    ${CMAKE_SOURCE_DIR}/tests/pubsub_test_case.hpp
#    ${CMAKE_SOURCE_DIR}/tests/index_test_case.hpp
#  )

TARGET_LINK_LIBRARIES(
    test_plan
    gcov
    ${GTEST_BOTH_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    ${PROTOBUF_LIBRARY}
    ${LibEv_LIBRARIES}
    ${Glog_LIBRARIES}
    ${JsonCpp_LIBRARIES}
    ssl
    crypto
    curl
    pthread
    tbb
    ${ThirdParty_LIBRARIES}
    dl
    boost_filesystem
    boost_system
  )

add_dependencies(test_plan cds)
add_dependencies(test_plan googlemock)


INCLUDE(CodeCoverage)

SETUP_TARGET_FOR_COVERAGE(
    test_plan_coverage
    test_plan
    coverage
)
