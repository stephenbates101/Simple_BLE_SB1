cmake_minimum_required(VERSION 3.21)

project(INDRAAPP_V1)

message("-- [INFO] Building Example")

add_executable(IndraApp_v1
    IndraApp_v1.cpp
    ../common/utils.cpp
)

target_link_libraries(IndraApp_v1 simpleble::simpleble)
