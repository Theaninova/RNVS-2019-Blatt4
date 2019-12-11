cmake_minimum_required(VERSION 3.15)
project(Blatt4)

set(CMAKE_CXX_STANDARD 14)

include_directories(src/generic)
include_directories(src/helper/max)
include_directories(src/helper/wulkanat)
include_directories(src/libraries)

add_executable(Blatt4
        src/generic/data_helper.c
        src/generic/data_helper.h
        src/generic/hash_helper.c
        src/generic/hash_helper.h
        src/generic/network.c
        src/generic/network.h
        src/helper/max/concurrent_task.c
        src/helper/max/concurrent_task.h
        src/helper/max/extended_math.c
        src/helper/max/extended_math.h
        src/helper/wulkanat/commander.h
        src/helper/wulkanat/debug.h
        src/helper/wulkanat/descriptive_types.h
        src/helper/wulkanat/flow_plus.h
        src/helper/wulkanat/friendly_heap.h
        src/helper/wulkanat/queue.h
        src/helper/wulkanat/smart_pointer.h
        src/libraries/uthash.h
        src/client.c
        src/peer.c
        src/server.c
        CMakeLists.txt)
