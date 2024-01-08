cmake_minimum_required(VERSION 3.21)

include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/prelude.cmake)
project(simplebluez_projects)
include(${CMAKE_CURRENT_SOURCE_DIR}/../cmake/epilogue.cmake)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_EXTENSIONS OFF)

option(SIMPLEBLUEZ_LOCAL "Use local SimpleBluez" ON)

if (SIMPLEBLUEZ_LOCAL)
    add_subdirectory(${CMAKE_CURRENT_LIST_DIR}/../simplebluez ${CMAKE_BINARY_DIR}/simplebluez)
else()
    find_package(simplebluez CONFIG REQUIRED)
endif()

# C++
add_subdirectory(Indra_Connect)
add_subdirectory(Indra_BT_App)