cmake_minimum_required(version 3.15)

project(server)

LINK_LIBRARIES(pthread dl)

add_executable(server server.c buffer.c conf.c event.c io.c log.c thread.c)
