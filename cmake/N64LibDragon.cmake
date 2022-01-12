cmake_minimum_required(VERSION 3.2)

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_VERSION 1)
set(CMAKE_SYSTEM_PROCESSOR "mips")
set(CMAKE_CROSSCOMPILING 1)

set(N64_INST $ENV{N64_INST})

if(NOT IS_DIRECTORY ${N64_INST})
    message(FATAL_ERROR "Please set N64_INST in your environment")
endif()

set(N64_GCC_PREFIX ${N64_INST}/bin/mips64-elf-)
set(N64_INCLUDE    ${N64_INST}/mips64-elf/include)
set(N64_LIB        ${N64_INST}/mips64-elf/lib)

set(CMAKE_ASM_COMPILER ${N64_GCC_PREFIX}gcc    CACHE PATH "")
set(CMAKE_C_COMPILER   ${N64_GCC_PREFIX}gcc    CACHE PATH "")
set(CMAKE_CXX_COMPILER ${N64_GCC_PREFIX}g++    CACHE PATH "")
set(CMAKE_LINKER       ${N64_GCC_PREFIX}g++    CACHE PATH "")
set(CMAKE_AR           ${N64_GCC_PREFIX}ar     CACHE PATH "")
set(CMAKE_AS           ${N64_GCC_PREFIX}as     CACHE PATH "")
set(CMAKE_STRIP        ${N64_GCC_PREFIX}strip  CACHE PATH "")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BOTH)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)

SET(BUILD_SHARED_LIBS OFF CACHE INTERNAL "Shared libs not available")

add_definitions(-D_N64)

set(ARCH "-march=vr4300 -mtune=vr4300")
set(CMAKE_C_FLAGS "${ARCH} -std=gnu99" CACHE STRING "C flags")
set(CMAKE_CXX_FLAGS "${ARCH} -std=c++11" CACHE STRING "C++ flags")

include_directories("${N64_INCLUDE}")

link_libraries(
  "-L${N64_LIB}"
  "-ldragon"
  "-lc"
  "-lm"
  "-lstdc++"
  "-ldragonsys"
  "-Tn64.ld"
  "-Wl,--wrap=__do_global_ctors"
)
