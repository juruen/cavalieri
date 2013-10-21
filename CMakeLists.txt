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
    ${CMAKE_SOURCE_DIR}/src/tcpserver.cpp
    ${CMAKE_SOURCE_DIR}/src/util.cpp
    ${CMAKE_SOURCE_DIR}/src/websocket.cpp
    ${CMAKE_SOURCE_DIR}/src/parser.cpp
    ${CMAKE_SOURCE_DIR}/src/scanner.cpp
    ${CMAKE_SOURCE_DIR}/src/driver.cpp
    ${CMAKE_SOURCE_DIR}/src/expression.cpp
  )

SET(MINIRIEMANN_HDRS
    ${CMAKE_SOURCE_DIR}/src/
  )

SET(MINIRIEMANN_PROTOFILES
    ${CMAKE_SOURCE_DIR}/src/proto.proto
  )

ADD_DEFINITIONS( -Wall -std=c++11 -g )

INCLUDE(FindProtobuf)

FIND_PACKAGE(Protobuf REQUIRED)
FIND_PACKAGE(Glog REQUIRED)
FIND_PACKAGE(LibEv REQUIRED)
FIND_PACKAGE(PythonLibs 2.7 REQUIRED)


IF(PROTOBUF_FOUND)
  PROTOBUF_GENERATE_CPP(ProtoSources ProtoHeaders ${MINIRIEMANN_PROTOFILES})

  INCLUDE_DIRECTORIES(
      ${CMAKE_CURRENT_BINARY_DIR}
      ${CMAKE_BINARY_DIR}
      ${MINIRIEMANN_HDRS}
      ${PROTOBUF_INCLUDE_DIR}
      ${Glog_INCLUDE_DIR}
      ${LibEv_INCLUDE_DIR}
      ${PYTHON_INCLUDE_DIRS}
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
      ${PYTHON_LIBRARIES}
      ssl
      crypto
      curl
    )

ENDIF(PROTOBUF_FOUND)
