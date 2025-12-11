cmake_minimum_required(VERSION 3.10)
project(lab8_sockets C)

set(CMAKE_C_STANDARD 99)

add_executable(ipv4_dg_server ipv4_dg_server.c ipv4_dg.h)
add_executable(ipv4_dg_client ipv4_dg_client.c ipv4_dg.h)
add_executable(ipv6_dg_server ipv6_dg_server.c ipv6_dg.h)
add_executable(ipv6_dg_client ipv6_dg_client.c ipv6_dg.h)

add_executable(ipv4_stream_echo_server ipv4_stream_echo_server.c)
add_executable(ipv4_stream_echo_client ipv4_stream_echo_client.c)

add_executable(ipv4_stream_chat_server ipv4_stream_chat_server.c)
target_link_libraries(ipv4_stream_chat_server pthread)
add_executable(ipv4_stream_chat_client ipv4_stream_chat_client.c)
