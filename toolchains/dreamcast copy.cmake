# Using standard kos provided toolchain.cmake
include($ENV{KOS_BASE}/utils/cmake/dreamcast.toolchain.cmake)

message(STATUS "CMAKE_SYSROOT is set to: ${CMAKE_SYSROOT}")


# Set the system root directory
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --sysroot=/opt/toolchains/dc/sh-elf/sh-elf -isystem /opt/toolchains/dc/sh-elf/sh-elf/include -isystem /opt/toolchains/dc/kos/include  -isystem /opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1/ -isystem /opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1/sh-elf/ -isystem /opt/toolchains/dc/kos-ports/include")

# Make sure it is using the right architecture
add_compile_definitions(_arch_dreamcast)


# Import the KOS_BASE environment variable
set(KOS_BASE $ENV{KOS_BASE})

# Add KOS include paths to the compiler flags
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} \
  -I${KOS_BASE}/include \
  -I${KOS_BASE}/arch/dreamcast/include \
  -I${KOS_BASE}/../kos-ports/include"
)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} \
  -isystem /opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1/sh-elf \
  -isystem /opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1 \
  -I${KOS_BASE}/include \
  -I${KOS_BASE}/arch/dreamcast/include \
  -I${KOS_BASE}/../kos-ports/include \
  -isystem /opt/toolchains/dc/sh-elf/sh-elf/include \
  -isystem      /opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1/experimental"
)

include_directories(
    ${KOS_BASE}/include
    /opt/toolchains/dc/sh-elf/sh-elf/include/c++/14.2.1
    /opt/toolchains/dc/kos-ports/include
    /opt/toolchains/dc/sh-elf/sh-elf/include/
)



set(MKDCDISC "/usr/local/bin/mkdcdisc")