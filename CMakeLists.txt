PROJECT(miniriemann)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Project's path for custom cmake modules
SET(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/.cmake/Modules/"
  )

SET(MINIRIEMANN_SRCS
    ${CMAKE_SOURCE_DIR}/src/miniriemann.cpp
    ${CMAKE_SOURCE_DIR}/src/index.cpp
    ${CMAKE_SOURCE_DIR}/src/pubsub.cpp
    ${CMAKE_SOURCE_DIR}/src/streams.cpp
    ${CMAKE_SOURCE_DIR}/src/folds.cpp
    ${CMAKE_SOURCE_DIR}/src/tcpserver.cpp
    ${CMAKE_SOURCE_DIR}/src/tcpconnection.cpp
    ${CMAKE_SOURCE_DIR}/src/riemanntcpconnection.cpp
    ${CMAKE_SOURCE_DIR}/src/util.cpp
    ${CMAKE_SOURCE_DIR}/src/websocket.cpp
    ${CMAKE_SOURCE_DIR}/src/query/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/query/scanner.cpp
    ${CMAKE_SOURCE_DIR}/src/query/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/query/expression.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/common.cpp
    ${CMAKE_SOURCE_DIR}/src/pythoninterpreter.cpp
    ${CMAKE_SOURCE_DIR}/src/external/pagerduty.cpp
    ${CMAKE_SOURCE_DIR}/src/wsutil.cpp
    ${CMAKE_SOURCE_DIR}/src/incomingevents.cpp
    ${CMAKE_SOURCE_DIR}/src/thread_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/tcp_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/riemann_tcp_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/real_scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/mock_scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/real_index.cpp
    ${CMAKE_SOURCE_DIR}/src/scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/websocket_pool.cpp
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
    ${CMAKE_SOURCE_DIR}/src/streams.cpp
    ${CMAKE_SOURCE_DIR}/src/folds.cpp
    ${CMAKE_SOURCE_DIR}/src/index.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/common.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/rules.cpp
    ${CMAKE_SOURCE_DIR}/src/scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/mock_scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/mock_index.cpp
    ${CMAKE_SOURCE_DIR}/src/index.cpp
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

