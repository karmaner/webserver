cmake_minimum_required(VERSION 3.1...3.5)
project(webserver)


include (cmake/utils.cmake)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/cmake)

set(CMAKE_VERBOSE_MAKEFILE OFF)
set(CMAKE_CXX_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -ggdb -std=c++11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")
set(CMAKE_C_FLAGS "$ENV{CXXFLAGS} -rdynamic -O0 -fPIC -ggdb -std=c11 -Wall -Wno-deprecated -Werror -Wno-unused-function -Wno-builtin-macro-redefined -Wno-deprecated-declarations")

set_source_files_properties(src/ds/roaring.c COMPILE_FLAGS "-Wno-use-after-free")

include_directories(.)
include_directories(/apps/webserver/include)
link_directories(/apps/webserver/lib)

cmake_policy(SET CMP0064 NEW)
option(TEST "ON for complile test" OFF)

find_package(MySQL REQUIRED)
if(MYSQL_FOUND)
    include_directories(${MYSQL_INCLUDE_DIR})
endif()

find_package(SQLite3 REQUIRED)
if(SQLite3_FOUND)
    include_directories( ${SQLite3_INCLUDE_DIRS})
endif()

find_package(Protobuf REQUIRED)
if(Protobuf_FOUND)
    include_directories(${Protobuf_INCLUDE_DIRS})
endif()

find_package(Boost REQUIRED)
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS})
endif()

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
    include_directories(${OPENSSL_INCLUDE_DIR})
endif()

find_package(ZLIB REQUIRED)

if(ZLIB_FOUND)
    include_directories(${ZLIB_INCLUDE_DIRS})
endif()


set(LIB_SRC
    src/http/http.cpp
    src/http/http_connection.cpp
    src/http/http_parser.cpp
    src/http/http_session.cpp
    src/http/http_server.cpp
    src/http/servlet.cpp
    src/http/ws_session.cpp
    src/http/ws_servlet.cpp
    src/http/ws_server.cpp
    src/http/ws_connection.cpp

    src/http/servlets/config_servlet.cpp
    src/http/servlets/status_servlet.cpp

    src/rock/rock_server.cpp
    src/rock/rock_stream.cpp
    src/rock/rock_protocol.cpp

    src/basic/protocol.cpp
    src/basic/module.cpp
    src/basic/library.cpp
    src/basic/worker.cpp
    src/basic/daemon.cpp
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
    src/basic/stream.cpp
    src/basic/tcp_server.cpp
    src/basic/timer.cpp
    src/basic/thread.cpp
    src/basic/util.cpp
    src/basic/env.cpp
    src/basic/scheduler.cpp
    src/basic/mutex.cpp
    src/basic/zk_client.cpp

    src/util/crypto_util.cpp
    src/util/json_util.cpp
    src/util/hash_util.cpp

    src/application/application.cpp

    src/streams/load_balance.cpp
    src/streams/socket_stream.cpp
    src/streams/zlib_stream.cpp
    src/streams/async_socket_stream.cpp
    src/streams/service_discovery.cpp

    src/db/sqlite3.cpp
    src/db/mysql.cpp
    src/db/redis.cpp
    src/db/fox_thread.cpp

    src/email/email.cpp
    src/email/smtp.cpp

    src/ns/name_server_module.cpp
    src/ns/ns_client.cpp
    src/ns/ns_protocol.cpp

    src/ds/bitmap.cpp
    src/ds/roaring.c
    src/ds/roaring_bitmap.cpp
    )

ragelmaker(src/http/http11_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/http)
ragelmaker(src/http/httpclient_parser.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/http)
ragelmaker(src/basic/uri.rl LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR}/src/basic)

protobufmaker(src/ns/ns_protobuf.proto LIB_SRC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(webserver SHARED ${LIB_SRC})
force_redefine_file_macro_for_sources(webserver)


find_library(YAMLCPP yaml-cpp)
find_library(PTHREAD pthread)
find_library(JSONCPP jsoncpp)

find_library(ZOOKEEPER_LIBRARY zookeeper_mt)
find_library(HIREDIS_VIP_LIB hiredis_vip)
find_library(LIBEVENT_LIB event)


set(LIBS
        webserver
        pthread
        yaml-cpp
        jsoncpp
        tinyxml2
        event
        hiredis_vip
        ${ZOOKEEPER_LIBRARY}
        ${SQLite3_LIBRARIES}
        ${OPENSSL_LIBRARIES}
        ${ZLIB_LIBRARIES}
        ${PROTOBUF_LIBRARIES}
        ${MYSQL_LIBRARIES}
        )

#${PROTOBUF_LIBRARIES})

if(TEST)
webserver_add_executable(test_log "tests/test_log.cpp" webserver "${LIBS}")
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

webserver_add_executable(test_http_server "tests/test_http_server.cpp" webserver "${LIBS}")
webserver_add_executable(test_http_connection "tests/test_http_connection.cpp" webserver "${LIBS}")
webserver_add_executable(test_uri "tests/test_uri.cpp" webserver "${LIBS}")

webserver_add_executable(my_http_server "samples/my_http_server.cpp" webserver "${LIBS}")

webserver_add_executable(echo_server "examples/echo_server.cpp" webserver "${LIBS}")
webserver_add_executable(echo_udp_client "examples/echo_udp_client.cpp" webserver "${LIBS}")
webserver_add_executable(echo_server_upd "examples/echo_server_udp.cpp" webserver "${LIBS}")

webserver_add_executable(test_daemon "tests/test_daemon.cpp" webserver "${LIBS}")
webserver_add_executable(test_env "tests/test_env.cpp" webserver "${LIBS}")

webserver_add_executable(test_ws_client "tests/test_ws_client.cpp" webserver "${LIBS}")
webserver_add_executable(test_ws_server "tests/test_ws_server.cpp" webserver "${LIBS}")
webserver_add_executable(test_application "tests/test_application.cpp" webserver "${LIBS}")

webserver_add_executable(test_lru "tests/test_lru.cpp" webserver "${LIBS}")
webserver_add_executable(test_timed_cache "tests/test_timed_cache.cpp" webserver "${LIBS}")
webserver_add_executable(test_timed_lru_cache "tests/test_timed_lru_cache.cpp" webserver "${LIBS}")
webserver_add_executable(test_zlib_stream "tests/test_zlib_stream.cpp" webserver "${LIBS}")
webserver_add_executable(test_sqlite3 "tests/test_sqlite3.cpp" webserver "${LIBS}")
webserver_add_executable(test_rock "tests/test_rock.cpp" webserver "${LIBS}")
webserver_add_executable(test_crypto "tests/test_crypto.cpp" webserver "${LIBS}")
webserver_add_executable(test_email  "tests/test_email.cpp" webserver "${LIBS}")

webserver_add_executable(test_mysql  "tests/test_mysql.cpp" webserver "${LIBS}")
webserver_add_executable(test_zookeeper  "tests/test_zookeeper.cpp" webserver "${LIBS}")
webserver_add_executable(test_service_discovery "tests/test_service_discovery.cpp" webserver "${LIBS}")

endif()



set(ORM_SRCS
    src/orm/table.cpp
    src/orm/column.cpp
    src/orm/index.cpp
    src/orm/orm.cpp
    src/orm/util.cpp
    )
webserver_add_executable(orm "${ORM_SRCS}" webserver "${LIBS}")

webserver_add_executable(test_bitmap "tests/test_bitmap.cpp" webserver "${LIBS}")
webserver_add_executable(bin_webserver "src/basic/main.cpp" webserver "${LIBS}")
set_target_properties(bin_webserver PROPERTIES OUTPUT_NAME "webserver")

add_library(test SHARED tests/test_module.cpp)

# add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/orm_out)
# set(OLIBS ${LIBS})
# set(OLIBS ${OLIBS} orm_data)
# webserver_add_executable(test_orm "tests/test_orm.cpp" orm_data "${OLIBS}")

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)