PROJECT(miniriemann)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Project's path for custom cmake modules
SET(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/.cmake/Modules/"
  )

SET(MINIRIEMANN_SRCS
    ${CMAKE_SOURCE_DIR}/src/miniriemann.cpp
    ${CMAKE_SOURCE_DIR}/src/core.cpp
    ${CMAKE_SOURCE_DIR}/src/riemann_tcp_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/riemann_udp_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/websocket_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/riemann_tcp_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/os_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/real_os_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/pub_sub/pub_sub.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_infra.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/folds.cpp
    ${CMAKE_SOURCE_DIR}/src/incoming_events.cpp
    ${CMAKE_SOURCE_DIR}/src/util.cpp
    ${CMAKE_SOURCE_DIR}/src/async/real_async_loop.cpp
    ${CMAKE_SOURCE_DIR}/src/index/index.cpp
    ${CMAKE_SOURCE_DIR}/src/index/real_index.cpp
    ${CMAKE_SOURCE_DIR}/src/async/async_loop.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/tcp_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/udp_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/tcp_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/ws_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/ws_util.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/listen_tcp_socket.cpp
    ${CMAKE_SOURCE_DIR}/src/pool/thread_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/query/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/query/scanner.cpp
    ${CMAKE_SOURCE_DIR}/src/query/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/query/expression.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/common.cpp
    ${CMAKE_SOURCE_DIR}/src/python_interpreter.cpp
    ${CMAKE_SOURCE_DIR}/src/external/pagerduty.cpp
    ${CMAKE_SOURCE_DIR}/src/external/email.cpp
    ${CMAKE_SOURCE_DIR}/src/scheduler/real_scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/scheduler/mock_scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/scheduler/scheduler.cpp
  )

SET(MINIRIEMANN_HDRS
    ${CMAKE_SOURCE_DIR}/src/
    ${CMAKE_SOURCE_DIR}/src/query/
    ${CMAKE_SOURCE_DIR}/src/external/
    ${CMAKE_SOURCE_DIR}/src/rules/
    /usr/include/i386-linux-gnu/c++/4.7
  )

SET(MINIRIEMANN_PROTOFILES
    ${CMAKE_SOURCE_DIR}/src/proto.proto
  )

SET(RULETESTER_SRCS
    ${CMAKE_SOURCE_DIR}/src/rule_tester.cpp
    ${CMAKE_SOURCE_DIR}/src/rule_tester_util.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_infra.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/folds.cpp
    ${CMAKE_SOURCE_DIR}/src/index/index.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/common.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/rules.cpp
    ${CMAKE_SOURCE_DIR}/src/scheduler/scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/scheduler/mock_scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/index/mock_index.cpp
    ${CMAKE_SOURCE_DIR}/src/index/index.cpp
    ${CMAKE_SOURCE_DIR}/src/util.cpp
    ${CMAKE_SOURCE_DIR}/src/external/external_mocks.cpp
  )

ADD_DEFINITIONS( -Wall -std=c++11 -g )

INCLUDE(FindProtobuf)

FIND_PACKAGE(Protobuf REQUIRED)
FIND_PACKAGE(Glog REQUIRED)
FIND_PACKAGE(GFlags REQUIRED)
FIND_PACKAGE(LibEv REQUIRED)
FIND_PACKAGE(PythonLibs 2.7 REQUIRED)
FIND_PACKAGE(CURL REQUIRED)
FIND_PACKAGE(OpenSSL REQUIRED)
FIND_PACKAGE(TBB REQUIRED)
FIND_PACKAGE(JsonCpp REQUIRED)

PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${MINIRIEMANN_PROTOFILES})

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}
    ${MINIRIEMANN_HDRS}
    ${PROTOBUF_INCLUDE_DIR}
    ${Glog_INCLUDE_DIR}
    ${LibEv_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${OpenSSL_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIRS}
    ${GFlags_INCLUDE_DIRS}
    ${JsonCpp_INCLUDE_DIRS}
  )

ADD_EXECUTABLE(
    miniriemann
    ${MINIRIEMANN_SRCS}
    ${ProtoSources}
    ${ProtoHeaders}
  )


TARGET_LINK_LIBRARIES(
    miniriemann
    ${CMAKE_THREAD_LIBS_INIT}
    ${PROTOBUF_LIBRARY}
    ${LibEv_LIBRARIES}
    ${Glog_LIBRARIES}
    ${GFLAGS_LIBRARIES}
    ${PYTHON_LIBRARIES}
    ${CURL_LIBRARIES}
    ${TBB_LIBRARIES}
    ${JsonCpp_LIBRARIES}
    crypto
    pthread
    cds
    dl
  )

ADD_EXECUTABLE(
    rule_tester
    ${RULETESTER_SRCS}
    ${ProtoSources}
    ${ProtoHeaders}
  )

TARGET_LINK_LIBRARIES(
    rule_tester
    ${CMAKE_THREAD_LIBS_INIT}
    ${PROTOBUF_LIBRARY}
    ${Glog_LIBRARIES}
    ${GFLAGS_LIBRARIES}
    ${CURL_LIBRARIES}
    ${TBB_LIBRARIES}
    ${JsonCpp_LIBRARIES}
    crypto
    pthread
    cds
    dl
  )

#add_library(rules SHARED src/rules.cpp)

ENABLE_TESTING()

ADD_SUBDIRECTORY(tests)

