# Using standard kos provided toolchain.cmake


message(STATUS "(1)CMAKE_SYSTEM_INCLUDE_PATH is set to: ${CMAKE_SYSTEM_INCLUDE_PATH}")

##### Configure Include Directories #####
set(CMAKE_SYSTEM_INCLUDE_PATH "${CMAKE_SYSTEM_INCLUDE_PATH} \
  /opt/toolchains/dc/sh-elf/sh-elf/include/ \
  $ENV{KOS_BASE}/include \
  $ENV{KOS_BASE}/kernel/arch/dreamcast/include \
  $ENV{KOS_BASE}/addons/include \
  /opt/toolchains/dc/kos-ports/include"
)

message(STATUS "(2)CMAKE_SYSTEM_INCLUDE_PATH is set to: ${CMAKE_SYSTEM_INCLUDE_PATH}")

include_directories(
    /opt/toolchains/dc/sh-elf/sh-elf/include/
    $ENV{KOS_BASE}/include
    $ENV{KOS_BASE}/kernel/arch/dreamcast/include
    $ENV{KOS_BASE}/addons/include
    /opt/toolchains/dc/kos-ports/include
)

include($ENV{KOS_BASE}/utils/cmake/dreamcast.toolchain.cmake)

message(STATUS "CMAKE_SYSROOT is set to: ${CMAKE_SYSROOT}")

message(STATUS "(3)CMAKE_SYSTEM_INCLUDE_PATH is set to: ${CMAKE_SYSTEM_INCLUDE_PATH}")



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

message(STATUS "(3.5)CMAKE_SYSTEM_INCLUDE_PATH is set to: ${CMAKE_SYSTEM_INCLUDE_PATH}")

# Retrieve the include directories
get_property(INCLUDE_DIRS GLOBAL PROPERTY INCLUDE_DIRECTORIES)

# Print out the directories
message(STATUS "Include directories in include_directories: ${INCLUDE_DIRS}")

# Defining how CMake formats system includes.
# It will generate ith system include paths with -isystem in the compiler command line.
set(CMAKE_INCLUDE_SYSTEM_FLAG_CXX "-isystem ")

set(MKDCDISC "/usr/local/bin/mkdcdisc")