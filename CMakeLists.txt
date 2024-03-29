cmake_minimum_required(VERSION 3.10)
project(peer)

set(CMAKE_CXX_STANDARD 11)

FIND_PACKAGE (Threads)

include_directories(src/generic)
include_directories(src/helper/max)
include_directories(src/helper/wulkanat)
include_directories(src/libraries)

add_executable(peer
        src/generic/data_helper.c
        src/generic/data_helper.h
        src/generic/hash_helper.c
        src/generic/hash_helper.h
        src/generic/network.c
        src/generic/network.h
        src/helper/max/concurrent_task.c
        src/helper/max/concurrent_task.h
        src/helper/max/extended_math.h
        src/helper/wulkanat/commander.h
        src/helper/wulkanat/debug.h
        src/helper/wulkanat/descriptive_types.h
        src/helper/wulkanat/queue.h
        src/libraries/uthash.h
        src/peer.c
        CMakeLists.txt)

SET(CMAKE_C_FLAGS ${CMAKE_C_FLAGS} "-pthread")