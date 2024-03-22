get_filename_component(CMAKELISTS_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
get_filename_component(PROTO_NAME "${CMAKELISTS_DIR}" NAME)
message(STATUS "Proto name: ${PROTO_NAME}")

cmake_minimum_required(VERSION 3.10)
project(${PROTO_NAME}_client)

# 设置C++标准
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# 查找需要的包
find_package(OpenCV REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)

# Protobuf 和 gRPC 的生成代码路径
set(PROTO_SRC_DIR ${CMAKE_SOURCE_DIR}/resources/protos)
set(GENERATED_OUT_PATH ${CMAKE_BINARY_DIR}/generated/protos/${PROTO_NAME})
file(MAKE_DIRECTORY ${GENERATED_OUT_PATH})

# 设置.proto文件名
set(PROTO_FILE "${PROTO_SRC_DIR}/${PROTO_NAME}.proto")

set(GENERATED_PROTO "${GENERATED_OUT_PATH}/${PROTO_NAME}.pb.cc")
set(GENERATED_HEADER "${GENERATED_OUT_PATH}/${PROTO_NAME}.pb.h")
set(GENERATED_GRPC "${GENERATED_OUT_PATH}/${PROTO_NAME}.grpc.pb.cc")
set(GENERATED_GRPC_HEADER "${GENERATED_OUT_PATH}/${PROTO_NAME}.grpc.pb.h")

add_custom_command(
  OUTPUT "${GENERATED_PROTO}" "${GENERATED_HEADER}" "${GENERATED_GRPC}" "${GENERATED_GRPC_HEADER}"
  COMMAND ${Protobuf_PROTOC_EXECUTABLE}
  ARGS --grpc_out "${GENERATED_OUT_PATH}"
       --cpp_out "${GENERATED_OUT_PATH}"
       -I "${PROTO_SRC_DIR}"
       --plugin=protoc-gen-grpc=`which grpc_cpp_plugin`
       "${PROTO_FILE}"
  DEPENDS "${PROTO_FILE}"
)

include_directories(${GENERATED_OUT_PATH})
include_directories(${OpenCV_INCLUDE_DIRS})
include_directories(${gRPC_INCLUDE_DIRS})
include_directories(${Protobuf_INCLUDE_DIRS})

# 编译${PROTO_NAME}_client库
add_library(${PROTO_NAME}_client
    ${PROTO_NAME}_client.h
    ${PROTO_NAME}_client.cpp
    ${GENERATED_PROTO}
    ${GENERATED_GRPC}
)

# 设置目标的包含路径
target_include_directories(${PROTO_NAME}_client PRIVATE
    ${OpenCV_INCLUDE_DIRS}
    ${gRPC_INCLUDE_DIRS}
    ${Protobuf_INCLUDE_DIRS}
    ${GENERATED_OUT_PATH}
    ${CMAKE_SOURCE_DIR}/src
)

# 链接到目标库
target_link_libraries(${PROTO_NAME}_client PRIVATE
    ${OpenCV_LIBS}
    gRPC::grpc++_reflection
    gRPC::grpc++
    protobuf::libprotobuf
)
