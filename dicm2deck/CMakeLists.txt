cmake_minimum_required(VERSION 4.2)
project(dicm2deck)

set(CMAKE_CXX_STANDARD 14)

include_directories(inc)

add_library(dicmlib STATIC EXCLUDE_FROM_ALL
        dicm/dicm.c
        dicm/datatypes.c
        dicm/main.c)

add_executable(dicm2cda
        dicm/uapi/dicm2cda/uapi.c
)
target_link_libraries(dicm2cda
        dicmlib
)
add_executable(dicmstructdump
        dicm/uapi/dicmstructdump/uapi.c
        dicm/dicm.c
        dicm/datatypes.c
        dicm/main.c)

#add_library(dicm2cdaPlugin MODULE src/uapi/dicm2cda/uapi.c)


#add_library(dicmstructdump MODULE src/uapi/dicmstructdump/uapi.c)

