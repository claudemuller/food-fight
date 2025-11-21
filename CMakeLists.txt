cmake_minimum_required(VERSION 3.10)
project(foodfight C)

set(CMAKE_C_STANDARD 11)

# Output directory for the binary
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Source files
file(GLOB SRC_FILES "${CMAKE_SOURCE_DIR}/src/*.c")

# Base warnings
add_compile_options(
    -Wall -pedantic -Wextra -Wmissing-declarations
    -Wfloat-conversion -Wsign-conversion -Wconversion
)

# Include directories
include_directories(
    ${CMAKE_SOURCE_DIR}/lib
    $ENV{HOME}/repos/3rd-party/nativefiledialog-extended/src/include
    $ENV{HOME}/repos/3rd-party/raylib/src
)

# GTK flags
find_package(PkgConfig REQUIRED)
pkg_check_modules(GTK3 REQUIRED gtk+-3.0)
include_directories(${GTK3_INCLUDE_DIRS})

# Link directories
link_directories(
    $ENV{HOME}/repos/3rd-party/raylib/build
    $ENV{HOME}/repos/3rd-party/nativefiledialog-extended/build/src
)

# Executable
add_executable(foodfight ${SRC_FILES})

# Link libraries
target_link_libraries(foodfight
    ${GTK3_LIBRARIES}
    raylib
    m
    pthread
    dl
    nfd
)

#----------- Address Sanitizer Build -------------

option(ASAN "Enable AddressSanitizer" OFF)

if(ASAN)
    message(STATUS "Building with AddressSanitizer")
    target_compile_options(foodfight PRIVATE -fsanitize=address -fno-common -fno-omit-frame-pointer)
    target_link_options(foodfight PRIVATE -fsanitize=address)
endif()

#----------- Debug Build (ASAN + -g) -------------

option(DEBUG "Enable debug mode" OFF)

if(DEBUG)
    message(STATUS "Debug mode enabled")
    target_compile_options(foodfight PRIVATE -g)
endif()

#----------- Custom run target --------------------

add_custom_target(run
    COMMAND foodfight ${ARGS}
    DEPENDS foodfight
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

#----------- MangoHud run target ------------------

add_custom_target(run-hud
    COMMAND LD_PRELOAD=/usr/lib/mangohud/libMangoHud_dlsym.so mangohud ./foodfight ${ARGS}
    DEPENDS foodfight
    WORKING_DIRECTORY ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}
)

#----------- Cleaning ------------------------------

set_property(DIRECTORY APPEND PROPERTY ADDITIONAL_MAKE_CLEAN_FILES
    "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}"
)

#----------- Compilation DB ------------------------

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

