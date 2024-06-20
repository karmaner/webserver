cmake_minimum_required(VERSION 3.1)
project(webserver)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

set(LIB_SRC
    src/basic/log.cpp
    src/basic/util.cpp
    )

add_library(webserver SHARED ${LIB_SRC})

add_executable(test tests/test.cpp)
add_dependencies(test webserver)
target_link_libraries(test webserver)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)