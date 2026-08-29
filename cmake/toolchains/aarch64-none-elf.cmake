#
# ProxOS kernel toolchain, aarch64 bare metal for the QEMU virt machine
#
# cmake -S . -B build-kernel -G Ninja \
#     -DCMAKE_TOOLCHAIN_FILE=cmake/toolchains/aarch64-none-elf.cmake
#
# Copyright 2026 Preston Brown
#

set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(PROX_KERNEL_TRIPLE aarch64-unknown-none-elf CACHE STRING "Kernel target triple")

# clang cross compiles from one install, no separate binutils needed
set(CMAKE_C_COMPILER   clang)
set(CMAKE_CXX_COMPILER clang++)
set(CMAKE_ASM_COMPILER clang)

set(CMAKE_C_COMPILER_TARGET   ${PROX_KERNEL_TRIPLE})
set(CMAKE_CXX_COMPILER_TARGET ${PROX_KERNEL_TRIPLE})
set(CMAKE_ASM_COMPILER_TARGET ${PROX_KERNEL_TRIPLE})

# There is no libc to link against, so the compiler check cannot make an
# executable. Check with a static library instead.
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

set(CMAKE_EXE_LINKER_FLAGS_INIT "-fuse-ld=lld -nostdlib -static")

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM BEFORE)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_PACKAGE ONLY)
