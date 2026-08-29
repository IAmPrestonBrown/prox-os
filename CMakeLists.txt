#
# ProxOS CMake
#
# Copyright 2026 Preston Brown
#

cmake_minimum_required(VERSION 3.21)

project(ProxOS
    VERSION 0.1.0
    DESCRIPTION "PRX bytecode virtual machine, PrASM assembler, and host tooling"
    LANGUAGES C CXX
)

if(CMAKE_CROSSCOMPILING)
    set(PROX_HOST_DEFAULT OFF)
else()
    set(PROX_HOST_DEFAULT ON)
endif()

if(CMAKE_CROSSCOMPILING)
    set(PROX_KERNEL_DEFAULT ON)
else()
    set(PROX_KERNEL_DEFAULT OFF)
endif()

# prox_vm still pulls in stdio.h for its host prox_printf, and prox_asm is C++
# and needs a standard library, so both are host only for now.
option(PROX_BUILD_VM     "Build the ProxVM library"                 ${PROX_HOST_DEFAULT})
option(PROX_BUILD_ASM    "Build the PrASM assembler library"        ${PROX_HOST_DEFAULT})
option(PROX_BUILD_CLI    "Build the prox host command line driver" ${PROX_HOST_DEFAULT})
option(PROX_BUILD_TESTS  "Build the ProxOS test suites"            ${PROX_HOST_DEFAULT})
option(PROX_BUILD_KERNEL "Build the ProxOS kernel"                 ${PROX_KERNEL_DEFAULT})
option(PROX_VM_DEBUG    "Compile the VM with diagnostic error output (VM_DEBUG)" ON)
option(PROX_WERROR      "Treat compiler warnings as errors"       OFF)

if(NOT PROJECT_IS_TOP_LEVEL)
    set(PROX_BUILD_TESTS OFF)
endif()

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

get_property(PROX_MULTI_CONFIG GLOBAL PROPERTY GENERATOR_IS_MULTI_CONFIG)
if(NOT PROX_MULTI_CONFIG AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS
        Debug Release RelWithDebInfo MinSizeRel)
endif()

set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib")
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin")

if(MSVC)
    set(PROX_WARNING_FLAGS /W4 $<$<BOOL:${PROX_WERROR}>:/WX>)
else()
    set(PROX_WARNING_FLAGS -Wall -Wextra -Wpedantic $<$<BOOL:${PROX_WERROR}>:-Werror>)
endif()

if(PROX_BUILD_VM)
add_library(prox_vm STATIC
    src/vm/vm.c
)
add_library(Prox::vm ALIAS prox_vm)

target_include_directories(prox_vm PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

if(PROX_VM_DEBUG)
    target_compile_definitions(prox_vm PUBLIC VM_DEBUG)
endif()

target_compile_options(prox_vm PRIVATE ${PROX_WARNING_FLAGS})

set_target_properties(prox_vm PROPERTIES
    OUTPUT_NAME proxvm
    POSITION_INDEPENDENT_CODE ON
)
endif()

if(PROX_BUILD_ASM)
add_library(prox_asm STATIC
    src/asm/assembler.cc
)
add_library(Prox::asm ALIAS prox_asm)

target_include_directories(prox_asm PUBLIC
    $<BUILD_INTERFACE:${PROJECT_SOURCE_DIR}/include>
    $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

target_link_libraries(prox_asm PRIVATE Prox::vm)
target_compile_options(prox_asm PRIVATE ${PROX_WARNING_FLAGS})

set_target_properties(prox_asm PROPERTIES
    OUTPUT_NAME proxasm
    POSITION_INDEPENDENT_CODE ON
)
endif()

if(PROX_BUILD_CLI)
    add_executable(prox
        src/cli/main.cc
    )

    target_link_libraries(prox PRIVATE Prox::asm Prox::vm)
    target_compile_options(prox PRIVATE ${PROX_WARNING_FLAGS})
endif()

if(PROX_BUILD_KERNEL)
    enable_language(ASM)

    set(PROX_KERNEL_LINKER_SCRIPT ${PROJECT_SOURCE_DIR}/src/kernel/linker.ld)

    add_executable(prox_kernel
        src/kernel/boot.S
        src/kernel/main.c
        src/kernel/uart.c
        src/kernel/printf.c
        src/kernel/alloc.c
    )

    target_include_directories(prox_kernel PRIVATE ${PROJECT_SOURCE_DIR}/include)

    # Freestanding: no libc, no host runtime, and no floating point or simd
    # since the kernel never sets those units up.
    target_compile_options(prox_kernel PRIVATE
        -ffreestanding
        -fno-builtin
        -fno-stack-protector
        -mgeneral-regs-only
        ${PROX_WARNING_FLAGS}
    )

    target_link_options(prox_kernel PRIVATE
        -nostdlib
        -static
        -Wl,-T,${PROX_KERNEL_LINKER_SCRIPT}
    )

    set_target_properties(prox_kernel PROPERTIES
        OUTPUT_NAME prox_kernel.elf
        LINK_DEPENDS ${PROX_KERNEL_LINKER_SCRIPT}
    )
endif()

if(PROX_BUILD_TESTS)
    enable_testing()

    add_executable(prox_vm_test tests/vm/test.c)
    target_link_libraries(prox_vm_test PRIVATE Prox::vm)
    target_compile_options(prox_vm_test PRIVATE ${PROX_WARNING_FLAGS})

    add_executable(prox_asm_test tests/asm/test.cc)
    target_link_libraries(prox_asm_test PRIVATE Prox::asm)
    target_compile_options(prox_asm_test PRIVATE ${PROX_WARNING_FLAGS})

    add_test(NAME vm  COMMAND prox_vm_test)
    add_test(NAME asm COMMAND prox_asm_test)

    set_tests_properties(vm asm PROPERTIES TIMEOUT 30)
endif()

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(PROX_INSTALL_TARGETS "")
if(PROX_BUILD_VM)
    list(APPEND PROX_INSTALL_TARGETS prox_vm)
endif()
if(PROX_BUILD_ASM)
    list(APPEND PROX_INSTALL_TARGETS prox_asm)
endif()

if(PROX_INSTALL_TARGETS)
    install(TARGETS ${PROX_INSTALL_TARGETS}
        EXPORT ProxOSTargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    )

    install(EXPORT ProxOSTargets
        FILE ProxOSTargets.cmake
        NAMESPACE Prox::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ProxOS
    )
endif()

if(PROX_BUILD_CLI)
    install(TARGETS prox RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
endif()

install(DIRECTORY ${PROJECT_SOURCE_DIR}/include/
    DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
)

write_basic_package_version_file(
    "${PROJECT_BINARY_DIR}/ProxOSConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)