cmake_minimum_required(VERSION 3.21)

project(INDRA_BT_APP)

message("-- [INFO] Building Example")
add_executable(Indra_BT_App Indra_BT_App.cpp)
target_link_libraries(Indra_BT_App simplebluez::simplebluez pthread)
