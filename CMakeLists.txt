cmake_minimum_required(VERSION 3.10)

set(CMAKE_TOOLCHAIN_FILE "${CMAKE_CURRENT_SOURCE_DIR}/vcpkg/scripts/buildsystems/vcpkg.cmake"
  CACHE STRING "Vcpkg toolchain file")

project(NGemity)
########## CONAN ##########
# if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
#   message(STATUS "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
#   file(DOWNLOAD "https://raw.githubusercontent.com/conan-io/cmake-conan/0.18.1/conan.cmake"
#                 "${CMAKE_BINARY_DIR}/conan.cmake"
#                 TLS_VERIFY ON)
# endif()

# include(${CMAKE_BINARY_DIR}/conan.cmake)
# conan_cmake_run(CONANFILE conanfile.txt BASIC_SETUP CMAKE_TARGETS BUILD missing) # run conan setup
######## END CONAN ########

# set macro-directory
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/macros")

if(CMAKE_COMPILER_IS_GNUCXX AND NOT MINGW)
    add_definitions(-fno-delete-null-pointer-checks)
endif()

if(CMAKE_BUILD_TYPE MATCHES "Debug")
    set(DEBUG 1)
endif()

option(WITHOUT_GIT          "Disable the GIT testing routines"               OFF)
option(WITH_PCH             "Use Precompiled Headers"                        OFF)
option(WITH_TOOLS           "Also compile additional tools"                  ON)
option(NG_USE_CLITHREAD     "Use CLI Thread (readline on Linux)"             OFF)

find_package(Platform REQUIRED)
find_package(PCHSupport REQUIRED)
IF(NG_USE_CLITHREAD)
    find_package(Readline REQUIRED)
    add_definitions(-DNG_USE_CLITHREAD)
endif()

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