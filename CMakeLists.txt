cmake_minimum_required(VERSION 3.0.2)

# set macro-directory
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/macros")
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set RPATH-handing (CMake parameters)
set(CMAKE_SKIP_BUILD_RPATH 0)
set(CMAKE_BUILD_WITH_INSTALL_RPATH 0)
set(CMAKE_INSTALL_RPATH "${CMAKE_INSTALL_PREFIX}/lib")
set(CMAKE_INSTALL_RPATH_USE_LINK_PATH 1)

# build in Release-mode by default if not explicitly set
if( NOT CMAKE_BUILD_TYPE )
    set(CMAKE_BUILD_TYPE "Release")
endif()

find_package(ACE REQUIRED)
find_package(Threads REQUIRED)
find_package(MYSQL REQUIRED)
find_package(Lua REQUIRED)

include_directories(${CMAKE_SOURCE_DIR}/shared
                    ${CMAKE_SOURCE_DIR}/dep/sol2/single/sol
                    ${CMAKE_SOURCE_DIR}/shared/Configuration
                    ${CMAKE_SOURCE_DIR}/shared/Database
                    ${CMAKE_SOURCE_DIR}/shared/Dynamic
                    ${CMAKE_SOURCE_DIR}/shared/Threading
                    ${CMAKE_SOURCE_DIR}/shared/Debugging
                    ${CMAKE_SOURCE_DIR}/shared/Encryption
                    ${CMAKE_SOURCE_DIR}/shared/Lists
                    ${CMAKE_SOURCE_DIR}/shared/Logging
                    ${CMAKE_SOURCE_DIR}/shared/utf8
                    ${CMAKE_SOURCE_DIR}/shared/Server
                    ${CMAKE_SOURCE_DIR}/shared/Utilities
                    ${ACE_INCLUDE_DIR}
                    ${LUA_INCLUDE_DIR}
                    ${MYSQL_INCLUDE_DIR})

file(GLOB_RECURSE SHARED_SRC shared/*.cpp ${ace_STAT_SRCS} )

add_subdirectory(Mononoke)
add_subdirectory(Chihiro)