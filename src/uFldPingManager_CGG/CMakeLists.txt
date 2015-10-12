#--------------------------------------------------------
# The CMakeLists.txt for:             uFldPingManager_CGG
# Author(s):                                  Simon Rohou
#--------------------------------------------------------

set(PROTOS fld_ping_manager_config.proto)  
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} ${PROTOS})

SET(SRC
  FldPingManager_CGG.cpp
  FldPingManager_CGG_Info.cpp
  FldPingManager_CGG_Test.cpp
  main.cpp
)

ADD_EXECUTABLE(uFldPingManager_CGG ${SRC} ${PROTO_HDRS} ${PROTO_SRCS})

TARGET_LINK_LIBRARIES(uFldPingManager_CGG
  ${MOOS_LIBRARIES}
  m
  pthread
  apputil
  geometry 
  contacts
  ufield
  mbutil
  # MOOSSafir libraries:
  CoordinateSystem
  Acoustic
  Tests
  Ping
  ${SYSTEM_LIBS}
  ${GOBY_LIBRARIES}
  ${Boost_LIBRARIES}
)
