cmake_minimum_required(VERSION 4.2)
project(dicm2deck)

set(CMAKE_CXX_STANDARD 14)

include_directories(
        inc
        thirdparty/sqlite3
        thirdparty/blake3
)

add_library(dicmlib STATIC EXCLUDE_FROM_ALL
        dicm/dicm.c
        dicm/datatypes.c
        dicm/main.c
)
add_library(sqlite3lib STATIC EXCLUDE_FROM_ALL
        thirdparty/sqlite3/sqlite3.h
        thirdparty/sqlite3/sqlite3.c
        dicm/deck_sqlite3.c
)
add_library(blake3lib STATIC EXCLUDE_FROM_ALL
        thirdparty/blake3/blake3.h
        thirdparty/blake3/blake3.c
        thirdparty/blake3/blake3_avx2_x86-64_unix.S
        thirdparty/blake3/blake3_avx512_x86-64_unix.S
        thirdparty/blake3/blake3_dispatch.c
        thirdparty/blake3/blake3_impl.h
        thirdparty/blake3/blake3_portable.c
        thirdparty/blake3/blake3_sse2_x86-64_unix.S
        thirdparty/blake3/blake3_sse41_x86-64_unix.S
)

########
# uapi #
########

add_executable(dicm2cda
        dicm/uapi/dicm2cda/uapi.c
)
target_link_libraries(dicm2cda
        dicmlib
)


add_executable(dicmstructdump
        dicm/uapi/dicmstructdump/uapi.c
)
target_link_libraries(dicmstructdump
        dicmlib
)

########
# capi #
########

add_executable(dicm2decksqlite
        dicm/capi/dicm2decksqlite3/capi.c
)
target_link_libraries(dicm2decksqlite
        dicmlib
        blake3lib
        sqlite3lib
)

