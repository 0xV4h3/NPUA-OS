cmake_minimum_required(VERSION 3.10)
project(lab8_ipv4_stream C)

set(CMAKE_C_STANDARD 99)

add_executable(ipv4_dg_server ipv4_dg_server.c ipv4_dg.h)
add_executable(ipv4_dg_client ipv4_dg_client.c ipv4_dg.h)
add_executable(ipv6_dg_server ipv6_dg_server.c ipv6_dg.h)
add_executable(ipv6_dg_client ipv6_dg_client.c ipv6_dg.h)

add_executable(ipv4_stream_server_echo ipv4_stream_server_echo.c ipv4_dg.h)
add_executable(ipv4_stream_client_echo ipv4_stream_client_echo.c ipv4_dg.h)
add_executable(ipv4_stream_server_chat ipv4_stream_server_chat.c ipv4_dg.h)
add_executable(ipv4_stream_client_chat ipv4_stream_client_chat.c ipv4_dg.h)