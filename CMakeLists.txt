cmake_minimum_required(VERSION 3.5.1)
project(Blatt4)

set(CMAKE_C_STANDARD 99)

add_executable(client
        src/client.c

        src/generic/hash_helper.h
        src/generic/hash_helper.c
        src/generic/network.h
        src/generic/network.c
        src/generic/data_helper.h
        src/generic/data_helper.c

        src/libraries/uthash.h

        src/helper/wulkanat/commander.h
        src/helper/wulkanat/debug.h
        src/helper/wulkanat/queue.h
        src/helper/wulkanat/descriptive_types.h

        src/helper/max/extended_math.h
        src/helper/max/extended_math.c)
add_executable(peer
        src/peer.c
        src/generic/hash_helper.h
        src/generic/hash_helper.c
        src/generic/network.h
        src/generic/network.c
        src/generic/data_helper.h
        src/generic/data_helper.c

        src/libraries/uthash.h

        src/helper/wulkanat/commander.h
        src/helper/wulkanat/debug.h
        src/helper/wulkanat/queue.h
        src/helper/wulkanat/descriptive_types.h
        src/helper/wulkanat/flow_plus.h

        src/helper/max/extended_math.h
        src/helper/max/extended_math.c)
