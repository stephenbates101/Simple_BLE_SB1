cmake_minimum_required(VERSION 3.21)

project(INDRA_BT_V!)

message("-- [INFO] Building Example")

add_executable(Indra_BT_v1
    Indra_BT_v1.cpp
    ../common/utils.cpp
)

target_link_libraries(Indra_BT_v1 simpleble::simpleble)