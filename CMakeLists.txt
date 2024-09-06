cmake_minimum_required(VERSION 3.1)
project(webserver)

include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(.)
include_directories(webserver/include)
link_directories(webserver/lib)

set(LIB_SRC
    src/basic/log.cpp
    src/basic/util.cpp
    src/basic/config.cpp
    src/basic/thread.cpp
    src/basic/fiber.cpp
    src/basic/mutex.cpp
    src/basic/scheduler.cpp
    src/basic/env.cpp
    )

find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)
find_package(Boost REQUIRED)

set(LIBS
    webserver
    pthread
    yaml-cpp)
message("*** LIBS: ${LIBS}")

add_library(webserver SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(webserver) #__FILE__

add_executable(test_log tests/test_log.cpp)
add_dependencies(test_log webserver)
force_redefine_file_macro_for_sources(test_log) #__FILE__
target_link_libraries(test_log webserver ${LIBS})

add_executable(test_config tests/test_config.cpp)
add_dependencies(test_config webserver)
force_redefine_file_macro_for_sources(test_config) #__FILE__
target_link_libraries(test_config webserver ${LIBS})

add_executable(test_thread tests/test_thread.cpp)
add_dependencies(test_thread webserver)
force_redefine_file_macro_for_sources(test_thread) #__FILE__
target_link_libraries(test_thread webserver ${LIBS})

add_executable(test_util tests/test_util.cpp)
add_dependencies(test_util webserver)
force_redefine_file_macro_for_sources(test_util) #__FILE__
target_link_libraries(test_util webserver ${LIBS})

add_executable(test_fiber tests/test_fiber.cpp)
add_dependencies(test_fiber webserver)
force_redefine_file_macro_for_sources(test_fiber) #__FILE__
target_link_libraries(test_fiber webserver ${LIBS})

add_executable(test_scheduler tests/test_scheduler.cpp)
add_dependencies(test_scheduler webserver)
force_redefine_file_macro_for_sources(test_scheduler) #__FILE__
target_link_libraries(test_scheduler webserver ${LIBS})

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)