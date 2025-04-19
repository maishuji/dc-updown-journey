# Using standard kos provided toolchain.cmake


##### Configure Include Directories #####
set(CMAKE_SYSTEM_INCLUDE_PATH "${CMAKE_SYSTEM_INCLUDE_PATH} /opt/toolchains/dc/sh-elf/sh-elf/include/ ${KOS_BASE}/include ${KOS_BASE}/kernel/arch/dreamcast/include ${KOS_BASE}/addons/include $ENV{KOS_BASE}/../kos-ports/include")

include_directories(
    /opt/toolchains/dc/sh-elf/sh-elf/include/
    $ENV{KOS_BASE}/include
    $ENV{KOS_BASE}/kernel/arch/dreamcast/include
    $ENV{KOS_BASE}/addons/include
    $ENV{KOS_BASE}/../kos-ports/include
)

include($ENV{KOS_BASE}/utils/cmake/dreamcast.toolchain.cmake)

message(STATUS "CMAKE_SYSROOT is set to: ${CMAKE_SYSROOT}")


# Set the system root directory
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=/opt/toolchains/dc/sh-elf/sh-elf -isystem /opt/toolchains/dc/sh-elf/sh-elf/include -isystem /opt/toolchains/dc/kos/include  -isystem /opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1/ -isystem /opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1/sh-elf/ -isystem /opt/toolchains/dc/kos-ports/include")


# Import the KOS_BASE environment variable
set(KOS_BASE $ENV{KOS_BASE})

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
  -I${KOS_BASE}/include"
)

include_directories(
    ${KOS_BASE}/include
    /opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1
    /opt/toolchains/dc/kos-ports/include
    /opt/toolchains/dc/sh-elf/sh-elf/include/
)



set(MKDCDISC "/usr/local/bin/mkdcdisc")