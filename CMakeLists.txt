cmake_minimum_required(VERSION 3.1)
project(sylar)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function")

include_directories(.)
include_directories(/apps/sylar/include)
link_directories(/apps/sylar/lib)

find_library(YAMLCPP yaml-cpp)
message("***", ${YAMLCPP})

set(LIB_SRC
    src/basic/log.cpp
    src/basic/util.cpp
    src/basic/config.cpp
    )

add_library(webserver SHARED ${LIB_SRC})
#add_library(sylar_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES (sylar_static PROPERTIES OUTPUT_NAME "sylar")

# add_executable(test tests/test_log.cpp)
# add_dependencies(test webserver)
# target_link_libraries(test webserver ${YAMLCPP})

add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config webserver)
target_link_libraries(test_config webserver ${YAMLCPP})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)