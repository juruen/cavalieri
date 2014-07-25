PROJECT(cavalieri)

CMAKE_MINIMUM_REQUIRED(VERSION 2.8)

# Project's path for custom cmake modules
SET(CMAKE_MODULE_PATH
    ${CMAKE_MODULE_PATH}
    "${CMAKE_SOURCE_DIR}/.cmake/Modules/"
  )

ADD_SUBDIRECTORY(thirdparty)

SET(CAVALIERI_SRCS
    ${CMAKE_SOURCE_DIR}/src/cavalieri.cpp
    ${CMAKE_SOURCE_DIR}/src/core/core.cpp
    ${CMAKE_SOURCE_DIR}/src/core/real_core_helper.cpp
    ${CMAKE_SOURCE_DIR}/src/core/real_core.cpp
    ${CMAKE_SOURCE_DIR}/src/rules_loader.cpp
    ${CMAKE_SOURCE_DIR}/src/riemann_tcp_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/riemann_udp_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/websocket_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/riemann_tcp_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/os_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/config/config.cpp
    ${CMAKE_SOURCE_DIR}/src/real_os_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/pub_sub/pub_sub.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_infra.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_functions_lock.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_functions_lockfree.cpp
    ${CMAKE_SOURCE_DIR}/src/folds.cpp
    ${CMAKE_SOURCE_DIR}/src/util.cpp
    ${CMAKE_SOURCE_DIR}/src/async/real_async_loop.cpp
    ${CMAKE_SOURCE_DIR}/src/index/real_index.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/instrumentation.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/rate.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/reservoir.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/gauge.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/mem.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/tcp_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/udp_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/curl_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/tcp_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/tcp_client_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/ws_connection.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/ws_util.cpp
    ${CMAKE_SOURCE_DIR}/src/transport/listen_tcp_socket.cpp
    ${CMAKE_SOURCE_DIR}/src/pool/thread_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/query/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/query/scanner.cpp
    ${CMAKE_SOURCE_DIR}/src/query/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/query/expression.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/common.cpp
    ${CMAKE_SOURCE_DIR}/src/external/real_external.cpp
    ${CMAKE_SOURCE_DIR}/src/external/mock_external.cpp
    ${CMAKE_SOURCE_DIR}/src/external/mailer_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/external/pagerduty_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/external/graphite_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/external/graphite.cpp
    ${CMAKE_SOURCE_DIR}/src/external/riemann_tcp_client_pool.cpp
    ${CMAKE_SOURCE_DIR}/src/external/riemann_tcp_client.cpp
    ${CMAKE_SOURCE_DIR}/src/scheduler/real_scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/scheduler/mock_scheduler.cpp
  )

SET(CAVALIERI_HDRS
    ${CMAKE_SOURCE_DIR}/include/
    ${CMAKE_SOURCE_DIR}/include/query/
    ${CMAKE_SOURCE_DIR}/src/query/
    ${CMAKE_SOURCE_DIR}/include/external/
    ${CMAKE_SOURCE_DIR}/include/rules/
    /usr/include/i386-linux-gnu/c++/4.7
  )

SET(CAVALIERI_PROTOFILES
    ${CMAKE_SOURCE_DIR}/src/proto.proto
  )

SET(RULETESTER_SRCS
    ${CMAKE_SOURCE_DIR}/src/rule_tester.cpp
    ${CMAKE_SOURCE_DIR}/src/rule_tester_util.cpp
    ${CMAKE_SOURCE_DIR}/src/core/core.cpp
    ${CMAKE_SOURCE_DIR}/src/core/mock_core.cpp
    ${CMAKE_SOURCE_DIR}/src/rules_loader.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_infra.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_functions.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_functions_lock.cpp
    ${CMAKE_SOURCE_DIR}/src/streams/stream_functions_lockfree.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/instrumentation.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/rate.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/reservoir.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/gauge.cpp
    ${CMAKE_SOURCE_DIR}/src/instrumentation/mem.cpp
    ${CMAKE_SOURCE_DIR}/src/folds.cpp
    ${CMAKE_SOURCE_DIR}/src/rules/common.cpp
    ${CMAKE_SOURCE_DIR}/src/scheduler/mock_scheduler.cpp
    ${CMAKE_SOURCE_DIR}/src/index/mock_index.cpp
    ${CMAKE_SOURCE_DIR}/src/util.cpp
    ${CMAKE_SOURCE_DIR}/src/external/mock_external.cpp
  )

ADD_DEFINITIONS( -Wall -std=c++11 -g -O3 )

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

PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${CAVALIERI_PROTOFILES})

INCLUDE_DIRECTORIES(
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_BINARY_DIR}
    ${CAVALIERI_HDRS}
    ${PROTOBUF_INCLUDE_DIR}
    ${Glog_INCLUDE_DIR}
    ${LibEv_INCLUDE_DIR}
    ${PYTHON_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${OpenSSL_INCLUDE_DIRS}
    ${TBB_INCLUDE_DIRS}
    ${GFlags_INCLUDE_DIRS}
    ${JsonCpp_INCLUDE_DIRS}
    ${ThirdParty_INCLUDE_DIRS}
  )

SET(
  PYTHON_INSTALL_DIR
  ${CMAKE_INSTALL_PREFIX}/share/${CMAKE_PROJECT_NAME}/python
)

ADD_DEFINITIONS(
     -Wall -std=c++11 -g
    -DPYTHON_MODULES_PATH="${PYTHON_INSTALL_DIR}"
)

ADD_EXECUTABLE(
    cavalieri
    ${CAVALIERI_SRCS}
    ${ProtoSources}
    ${ProtoHeaders}
  )

add_dependencies(cavalieri cds)
add_dependencies(cavalieri googlemock)

TARGET_LINK_LIBRARIES(
    cavalieri
    ${CMAKE_THREAD_LIBS_INIT}
    ${PROTOBUF_LIBRARY}
    ${LibEv_LIBRARIES}
    ${Glog_LIBRARIES}
    ${GFLAGS_LIBRARIES}
    ${PYTHON_LIBRARIES}
    ${CURL_LIBRARIES}
    ${TBB_LIBRARIES}
    ${JsonCpp_LIBRARIES}
    ${ThirdParty_LIBRARIES}
    boost_filesystem
    boost_system
    crypto
    pthread
    dl
  )

ADD_EXECUTABLE(
    cavalieri_tester
    ${RULETESTER_SRCS}
    ${ProtoSources}
    ${ProtoHeaders}
  )

TARGET_LINK_LIBRARIES(
    cavalieri_tester
    boost_filesystem
    boost_system
    pthread
    dl
  )



add_dependencies(cavalieri_tester cds)
add_dependencies(cavalieri_tester googlemock)

TARGET_LINK_LIBRARIES(
    cavalieri_tester
    ${CMAKE_THREAD_LIBS_INIT}
    ${PROTOBUF_LIBRARY}
    ${Glog_LIBRARIES}
    ${GFLAGS_LIBRARIES}
    ${CURL_LIBRARIES}
    ${TBB_LIBRARIES}
    ${JsonCpp_LIBRARIES}
    ${ThirdParty_LIBRARIES}
    crypto
    pthread
    dl
  )

install(
  PROGRAMS
  ${CMAKE_BINARY_DIR}/cavalieri
  ${CMAKE_BINARY_DIR}/cavalieri_tester
  DESTINATION bin
)

install(PROGRAMS ${libcds_LIBS} DESTINATION lib)

install(
  FILES
  ${CMAKE_SOURCE_DIR}/src/python/mailer.py
  ${CMAKE_SOURCE_DIR}/src/python/pagerduty.py
  DESTINATION share/${CMAKE_PROJECT_NAME}/python
)

install(
  DIRECTORY
  ${CMAKE_SOURCE_DIR}/include/
  DESTINATION include/${CMAKE_PROJECT_NAME}
)

install(
  DIRECTORY
  ${libcds_INCLUDES}
  DESTINATION include/${CMAKE_PROJECT_NAME}
)

install(
  FILES
  ${CMAKE_BINARY_DIR}/proto.pb.h
  DESTINATION include/${CMAKE_PROJECT_NAME}
)


ENABLE_TESTING()

ADD_SUBDIRECTORY(tests)
