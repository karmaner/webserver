cmake_minimum_required(VERSION 3.1)
project(webserver)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

include_directories(.)
include_directories(webserver/include)
link_directories(webserver/lib)

find_library(YAMLCPP yaml-cpp)
message("*** YAMLCPP: ${YAMLCPP}")

set(LIB_SRC
    src/basic/log.cpp
    src/basic/util.cpp
    src/basic/config.cpp
    src/basic/thread.cpp
    )

set(LIBS
    webserver
    pthread
    ${YAMLCPP})
message("*** LIBS: ${LIBS}")

add_library(webserver SHARED ${LIB_SRC})

add_executable(test_log tests/test_log.cpp)
add_dependencies(test_log webserver)
target_link_libraries(test_log webserver ${LIBS})

add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config webserver)
target_link_libraries(test_config webserver ${LIBS})

add_executable(test_thread tests/test_thread.cpp)
add_dependencies(test_thread webserver)
target_link_libraries(test_thread webserver ${LIBS})


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)