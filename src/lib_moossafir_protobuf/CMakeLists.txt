include_directories(${CMAKE_CURRENT_BINARY_DIR})
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${CMAKE_CURRENT_BINARY_DIR} status.proto mini-status.proto sim-driver.proto)

set(SRC id-codec.cpp load-driver.cpp sim-driver.cpp)

add_library(moossafir_protobuf SHARED ${SRC} ${PROTO_SRCS} ${PROTO_HDRS})

target_link_libraries(moossafir_protobuf
  dccl ${GOBY_LIBRARIES} ${Boost_LIBRARIES} MOOS)

configure_file(pAcommsHandler_CGG ${IVP_EXTEND_BIN_DIR}/pAcommsHandler_CGG @ONLY)
