cmake_minimum_required(VERSION 3.5)

# set macro-directory
set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/macros")
set (CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# Features a C++ compiler must have to be used to compile sol2
# This list is not *complete* as CMake does not support features for
# all of the advanced features utilized.
set(CXX_FEATURES
        cxx_auto_type
        cxx_constexpr
        cxx_decltype
        cxx_decltype_auto
        cxx_default_function_template_args
        cxx_final
        cxx_lambdas
        cxx_noexcept
        cxx_nullptr
        cxx_override
        cxx_range_for
        cxx_return_type_deduction
        cxx_right_angle_brackets
        cxx_static_assert
        cxx_strong_enums
        cxx_variadic_macros
        cxx_variadic_templates)

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

file(GLOB_RECURSE SHARED_SRC shared/*.cpp ${ace_STAT_SRCS})

add_subdirectory(Mononoke)
add_subdirectory(Chihiro)