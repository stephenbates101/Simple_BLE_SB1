cmake_minimum_required(VERSION 3.21)

project(INDRA_CONNECT)

message("-- [INFO] Building Example")
add_executable(Indra_Connect Indra_Connect.cpp)
target_link_libraries(Indra_Connect simplebluez::simplebluez pthread)
