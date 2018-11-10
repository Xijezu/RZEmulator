project(NGemity)
cmake_minimum_required(VERSION 3.0.2)

# set macro-directory
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/macros")
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set RPATH-handing (CMake parameters)

set(CMAKE_SKIP_BUILD_RPATH FALSE)
set(CMAKE_BUILD_WITH_INSTALL_RPATH TRUE)
set(CMAKE_INSTALL_RPATH "${LIBS_DIR}")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH TRUE)

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

#Include platform/compiler-specific definitions
include(${CMAKE_SOURCE_DIR}/cmake/SetDefinitions.cmake)

find_package(Platform REQUIRED)
find_package(PCHSupport REQUIRED)
find_package(MYSQL REQUIRED)
find_package(Readline REQUIRED)

IF(NOT WITHOUT_GIT)
    find_package(Git)
ENDIF()

# Find revision ID and hash of the sourcetree
include(cmake/genrev.cmake)
add_custom_target(revision.h ALL
        COMMAND ${CMAKE_COMMAND} -DNO_GIT=${WITHOUT_GIT} -DGIT_EXEC=${GIT_EXECUTABLE} -DBUILDDIR=${CMAKE_BINARY_DIR} -P ${CMAKE_SOURCE_DIR}/cmake/genrev.cmake
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR})

add_subdirectory(dep/boost)
add_subdirectory(dep/liblua)
add_subdirectory(dep/fmt)
add_subdirectory(shared)
add_subdirectory(Mononoke)
add_subdirectory(Chihiro)