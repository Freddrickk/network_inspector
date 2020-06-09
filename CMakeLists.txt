cmake_minimum_required(VERSION 3.12)
project("network_inspector")

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${PROJECT_BINARY_DIR}/bin)
set(CMAKE_CXX_STANDARD 17)

add_library("network_inspector" STATIC "${CMAKE_CURRENT_SOURCE_DIR}/src/manager.cpp"
                                       "${CMAKE_CURRENT_SOURCE_DIR}/src/network_inspector.cpp"
                                       "${CMAKE_CURRENT_SOURCE_DIR}/src/utils.cpp"
                                       "${CMAKE_CURRENT_SOURCE_DIR}/src/hooks.cpp"
                                       "${CMAKE_CURRENT_SOURCE_DIR}/src/socket.cpp")

if(NOT TARGET htp)
    add_subdirectory("external/htp")
endif()

target_include_directories("network_inspector" PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/src" PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")

target_link_libraries("network_inspector" PUBLIC "htp")
target_link_libraries("network_inspector" PRIVATE ws2_32)

add_executable("client_tcp" "${CMAKE_CURRENT_SOURCE_DIR}/tests/client_tcp.cpp")
target_link_libraries("client_tcp" PUBLIC "network_inspector")

add_executable("server_tcp" "${CMAKE_CURRENT_SOURCE_DIR}/tests/server_tcp.cpp")
target_link_libraries("server_tcp" PUBLIC "network_inspector")

add_executable("server_udp" "${CMAKE_CURRENT_SOURCE_DIR}/tests/server_udp.cpp")
target_link_libraries("server_udp" PUBLIC "network_inspector")

add_executable("client_udp" "${CMAKE_CURRENT_SOURCE_DIR}/tests/client_udp.cpp")
target_link_libraries("client_udp" PUBLIC "network_inspector")
