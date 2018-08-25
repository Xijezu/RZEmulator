file(GLOB_RECURSE SHARED_SOURCE_FILES *.h *.cpp)

set(SHARED_INCLUDE_DIR
        ${CMAKE_SOURCE_DIR}/shared
        ${CMAKE_SOURCE_DIR}/shared/Asio
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
        ${CMAKE_SOURCE_DIR}/shared/Server/Packets
        ${CMAKE_SOURCE_DIR}/shared/Server/Packets/GameClient
        ${CMAKE_SOURCE_DIR}/shared/Server/Packets/AuthClient
        ${CMAKE_SOURCE_DIR}/shared/Server/Packets/AuthGame
        ${CMAKE_SOURCE_DIR}/shared/Server/ASIO
        ${CMAKE_SOURCE_DIR}/shared/Utilities
        ${CMAKE_SOURCE_DIR}/shared/Updater
        ${CMAKE_SOURCE_DIR}/dep/fmt
        ${Boost_INCLUDE_DIR}
        CACHE INTERNAL "" FORCE) # dont mind, dirty hack

include_directories(
        ${CMAKE_BINARY_DIR}
        ${Boost_INCLUDE_DIRS}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SHARED_INCLUDE_DIR}
        ${MYSQL_INCLUDE_DIR})


add_library(shared STATIC ${SHARED_SOURCE_FILES})

add_cxx_pch(shared ${CMAKE_SOURCE_DIR}/shared/PrecompiledHeaders/sharedPCH.h)