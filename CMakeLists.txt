cmake_minimum_required(VERSION 3.0.2)

# set macro-directory
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/macros")
set(CMAKE_CXX_STANDARD 14)
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

option(ACE_USE_EXTERNAL "Use external ACE"                                            0)
option(WITHOUT_GIT      "Disable the GIT testing routines"                            0)

if(ACE_USE_EXTERNAL)
    find_package(ACE)
    if(NOT ACE_FOUND)
        message(FATAL_ERROR
                "This project requires ACE installed when ACE_USE_EXTERNAL is set. Please download the ACE Micro Release Kit from http://download.dre.vanderbilt.edu/ and install it. If this script didn't find ACE and it was correctly installed please set ACE_ROOT to the correct path."
                )
    endif()
    if(EXISTS ${ACE_INCLUDE_DIR}/ace/Stack_Trace.h)
        add_definitions(-DHAVE_ACE_STACK_TRACE_H)
    endif()
else()
    include(cmake/ImportACE.cmake)
    if(NOT(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD"))
        add_definitions(-DHAVE_ACE_STACK_TRACE_H)
    endif()
endif()

#Include platform/compiler-specific definitions
include(${CMAKE_SOURCE_DIR}/cmake/SetDefinitions.cmake)
find_package(Platform REQUIRED)
find_package(MYSQL REQUIRED)

IF(NOT WITHOUT_GIT)
    find_package(Git)
ENDIF()

# Find revision ID and hash of the sourcetree
include(cmake/genrev.cmake)
add_custom_target(revision.h ALL
        COMMAND ${CMAKE_COMMAND} -DNO_GIT=${WITHOUT_GIT} -DGIT_EXEC=${GIT_EXECUTABLE} -DBUILDDIR=${CMAKE_BINARY_DIR} -P ${CMAKE_SOURCE_DIR}/cmake/genrev.cmake
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        )


include_directories(${CMAKE_BINARY_DIR}
                    ${CMAKE_SOURCE_DIR}/shared
                    ${CMAKE_SOURCE_DIR}/shared/Configuration
                    ${CMAKE_SOURCE_DIR}/shared/Database
                    ${CMAKE_SOURCE_DIR}/shared/Dynamic
                    ${CMAKE_SOURCE_DIR}/shared/Dynamic/LinkedReference
                    ${CMAKE_SOURCE_DIR}/shared/Threading
                    ${CMAKE_SOURCE_DIR}/shared/Debugging
                    ${CMAKE_SOURCE_DIR}/shared/Encryption
                    ${CMAKE_SOURCE_DIR}/shared/Lists
                    ${CMAKE_SOURCE_DIR}/shared/Logging
                    ${CMAKE_SOURCE_DIR}/shared/utf8
                    ${CMAKE_SOURCE_DIR}/shared/Server
                    ${CMAKE_SOURCE_DIR}/shared/Utilities)

if(NOT ACE_USE_EXTERNAL)
    add_subdirectory(dep/acelite)
endif()

add_subdirectory(dep/liblua)
add_subdirectory(shared)
add_subdirectory(Mononoke)
add_subdirectory(Chihiro)