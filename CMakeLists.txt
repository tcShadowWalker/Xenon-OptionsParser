cmake_minimum_required(VERSION 2.6)
project(optionsparser)

add_executable(optionsparser example_main.cpp XenonArgumentParser.cpp )

add_definitions ( -std=c++11 -Wall )

install(TARGETS optionsparser RUNTIME DESTINATION bin)
