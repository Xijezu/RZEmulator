project(NGemity)
cmake_minimum_required(VERSION 3.10)

# Enable conan
include(${CMAKE_BINARY_DIR}/conanbuildinfo.cmake)
conan_basic_setup(TARGETS)

# set macro-directory
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/macros")

if(CMAKE_COMPILER_IS_GNUCXX AND NOT MINGW)
    add_definitions(-fno-delete-null-pointer-checks)
endif()

# build in Release-mode by default if not explicitly set
if( NOT CMAKE_BUILD_TYPE )
    set(CMAKE_BUILD_TYPE "Release")
endif()
if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(DEBUG 1)
endif()

option(WITHOUT_GIT        "Disable the GIT testing routines"             0)
option(WITH_PCH               "Use Precompiled Headers"                       0)
option(WITH_TOOLS           "Also compile additional tools"                 1)

find_package(Platform REQUIRED)
find_package(PCHSupport REQUIRED)
find_package(MYSQL REQUIRED)
find_package(Readline REQUIRED)
#Include platform/compiler-specific definitions
include(${CMAKE_SOURCE_DIR}/cmake/SetDefinitions.cmake)

IF(NOT WITHOUT_GIT)
    find_package(Git)
ENDIF()

if(WITH_PCH)
    message("Compiling with PrecompiledHeaders")
else()
    message("Compiling without PrecompiledHeaders")
endif()

# Find revision ID and hash of the sourcetree
include(cmake/genrev.cmake)
add_custom_target(revision.h ALL
        COMMAND ${CMAKE_COMMAND} -DNO_GIT=${WITHOUT_GIT} -DGIT_EXEC=${GIT_EXECUTABLE} -DBUILDDIR=${CMAKE_BINARY_DIR} -P ${CMAKE_SOURCE_DIR}/cmake/genrev.cmake
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_subdirectory(dep/liblua)
add_subdirectory(shared)
add_subdirectory(Mononoke)
add_subdirectory(Chihiro)
if(WITH_TOOLS)
    add_subdirectory(Tools)
endif()