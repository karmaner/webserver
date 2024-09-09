cmake_minimum_required(VERSION 3.1)
project(webserver)


include (cmake/utils.cmake)

set(CMAKE_VERBOSE_MAKEFILE ON)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined")

include_directories(.)
include_directories(/apps/webserver/include)
link_directories(/apps/webserver/lib)

set(LIB_SRC

    src/http/http.cpp
    # src/http/http_connection.cpp
    src/http/http_parser.cpp
    # src/http/http_session.cpp
    # src/http/http_server.cpp
    # src/http/servlet.cpp
    src/basic/address.cpp
    src/basic/bytearray.cpp
    src/basic/config.cpp
    src/basic/fd_manager.cpp
    src/basic/fiber.cpp
    src/basic/hook.cpp
    src/basic/iomanager.cpp
    src/basic/log.cpp
    src/basic/scheduler.cpp
    src/basic/socket.cpp
    src/basic/socket_stream.cpp
    src/basic/stream.cpp
    src/basic/tcp_server.cpp
    src/basic/timer.cpp
    src/basic/thread.cpp
    src/basic/util.cpp
    src/basic/env.cpp
    src/basic/scheduler.cpp
    src/basic/mutex.cpp
    )

ragelmaker(src/http/http11_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/http)
ragelmaker(src/http/httpclient_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/http)

add_library(webserver SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(webserver) #__FILE__
#add_library(webserver_static STATIC ${LIB_SRC})
#SET_TARGET_PROPERTIES (webserver_static PROPERTIES OUTPUT_NAME "webserver")

find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)
#set(LIBS ${LIBS} webserver)
#set(LIBS ${LIBS} dl)
#set(LIBS ${LIBS} ${YAMLCPP})
#set(LIBS ${LIBS} ${PTHREAD})

set(LIBS
        webserver
        pthread
        yaml-cpp)

# webserver_add_executable(test1 "tests/test.cpp" webserver "${LIBS}")
webserver_add_executable(test_config "tests/test_config.cpp" webserver "${LIBS}")
webserver_add_executable(test_thread "tests/test_thread.cpp" webserver "${LIBS}")
webserver_add_executable(test_util "tests/test_util.cpp" webserver "${LIBS}")
webserver_add_executable(test_fiber "tests/test_fiber.cpp" webserver "${LIBS}")
webserver_add_executable(test_scheduler "tests/test_scheduler.cpp" webserver "${LIBS}")
webserver_add_executable(test_iomanager "tests/test_iomanager.cpp" webserver "${LIBS}")
webserver_add_executable(test_hook "tests/test_hook.cpp" webserver "${LIBS}")
webserver_add_executable(test_address "tests/test_address.cpp" webserver "${LIBS}")
webserver_add_executable(test_socket "tests/test_socket.cpp" webserver "${LIBS}")
webserver_add_executable(test_bytearray "tests/test_bytearray.cpp" webserver "${LIBS}")
webserver_add_executable(test_http "tests/test_http.cpp" webserver "${LIBS}")
webserver_add_executable(test_http_parser "tests/test_http_parser.cpp" webserver "${LIBS}")
webserver_add_executable(test_tcp_server "tests/test_tcp_server.cpp" webserver "${LIBS}")
# webserver_add_executable(echo_server "examples/echo_server.cpp" webserver "${LIBS}")
# webserver_add_executable(test_http_server "tests/test_http_server.cpp" webserver "${LIBS}")
# webserver_add_executable(test_http_connection "tests/test_http_connection.cpp" webserver "${LIBS}")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
