cmake_minimum_required(VERSION 3.6)

SET( PROJECT_VERSION 0.0.8 )

project(electricui-embedded)

add_library(electricui STATIC src/electricui.c src/eui_utilities.c src/transports/eui_binary_transport.c)

target_include_directories(electricui PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/src ${CMAKE_CURRENT_SOURCE_DIR}/src/transports)
