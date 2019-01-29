cmake_minimum_required (VERSION 2.8)

project (cThreadPool)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ./output/)

aux_source_directory(./src DIR_SRCS)
aux_source_directory(./test DIR_TEST)

add_compile_options(-g -Wall -std=c99)
link_libraries(pthread)

add_executable(tpool ${DIR_SRCS} ${DIR_TEST} test/main.c)
