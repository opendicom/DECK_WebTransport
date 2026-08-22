cmake_minimum_required(VERSION 4.2)
project(dicm2deck)

set(CMAKE_CXX_STANDARD 14)

include_directories(
        inc
        thirdparty/sqlite3
        thirdparty/blake3
)

add_library(dicmlib STATIC EXCLUDE_FROM_ALL
        src/datatypes.c
        src/main.c
)

add_library(sqlite3lib STATIC EXCLUDE_FROM_ALL
        thirdparty/sqlite3/sqlite3.h
        thirdparty/sqlite3/sqlite3.c
        src/deck_sqlite3.c
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
        src/uapi/dicm2cda/uapi.c
)
target_link_libraries(dicm2cda
        dicmlib
)


add_executable(utf8dump
        src/uapi/utf8dump/uapi.c
)
target_link_libraries(utf8dump
        dicmlib
)


add_executable(canonical
        src/uapi/canonical/uapi.c
)
target_link_libraries(canonical
        dicmlib
)


add_executable(dicmhtj2k
        src/uapi/dicmhtj2k/uapi.c
)

target_link_libraries(dicmhtj2k
        dicmlib
)


########
# capi #
########

add_executable(dicm2decksqlite
        src/capi/memorysqlite/capi.c
        src/capi/uapi.c
)
target_link_libraries(dicm2decksqlite
        dicmlib,
        blake3lib
        sqlite3lib
)

add_executable(pcsxmi src/capi/PCSXMI/capi.c
        src/capi/PCSXMI/capi.c
        src/capi/uapi.c
)
target_link_libraries(pcsxmi
        dicmlib
)