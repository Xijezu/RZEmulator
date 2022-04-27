set(LIBRARY_NAME shared)
if(WITH_PCH)
        set(sharedPCH "${CMAKE_CURRENT_SOURCE_DIR}/PrecompiledHeaders/sharedPCH.h")
endif()

file(GLOB SRC_GRP_ASIO Asio/*.cpp Asio/*.h)
source_group("ASIO" FILES ${SRC_GRP_ASIO})

file(GLOB SRC_GRP_COMMON Common/*.cpp Common/*.h)
source_group("Common" FILES ${SRC_GRP_COMMON})

file(GLOB SRC_GRP_CONFIG Configuration/*.cpp Configuration/*.h)
source_group("Config" FILES ${SRC_GRP_CONFIG})

file(GLOB_RECURSE SRC_GRP_DATABASE Database/*.cpp Database/*.h)
source_group("Database" FILES ${SRC_GRP_DATABASE})

file(GLOB SRC_GRP_DEBUG Debugging/*.cpp Debugging/*.h)
source_group("Debugging" FILES ${SRC_GRP_DEBUG})

file(GLOB SRC_GRP_DYNAMIC Dynamic/*.cpp Dynamic/*.h)
source_group("Dynamic" FILES ${SRC_GRP_DYNAMIC})

file(GLOB SRC_GRP_ENCRPYTION Encryption/*.cpp Encryption/*.h)
source_group("Encryption" FILES ${SRC_GRP_ENCRPYTION})

file(GLOB SRC_GRP_LOG Logging/*.cpp Logging/*.h)
source_group("Logging" FILES ${SRC_GRP_LOG})

file(GLOB_RECURSE SRC_GRP_SERVER Server/*.cpp Server/*.h)
source_group("Server" FILES ${SRC_GRP_SERVER})

file(GLOB SRC_GRP_THREAD Threading/*.cpp Threading/*.h)
source_group("Threading" FILES ${SRC_GRP_THREAD})

file(GLOB SRC_GRP_UTIL Utilities/*.cpp Utilities/*.h)
source_group("Utilities" FILES ${SRC_GRP_UTIL})

set(LIBRARY_SRCS
        ${SRC_GRP_ASIO}
        ${SRC_GRP_COMMON}
        ${SRC_GRP_CONFIG}
        ${SRC_GRP_DATABASE}
        ${SRC_GRP_DEBUG}
        ${SRC_GRP_DYNAMIC}
        ${SRC_GRP_ENCRPYTION}
        ${SRC_GRP_LOG}
        ${SRC_GRP_SERVER}
        ${SRC_GRP_THREAD}
        ${SRC_GRP_UTIL}
)

set(SHARED_INCLUDE_DIR
        ${CMAKE_SOURCE_DIR}/shared
        ${CMAKE_SOURCE_DIR}/shared/Common
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
        CACHE INTERNAL "shared_includes" FORCE
)

include_directories(
        ${SHARED_INCLUDE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_BINARY_DIR}
        ${CONAN_INCLUDE_DIRS_MYSQL-CONNECTOR-C}
        ${CONAN_INCLUDE_DIRS_BOOST}
        ${CONAN_INCLUDE_DIRS_OPENSSL}
        ${CONAN_INCLUDE_DIRS_FMT}
)
link_directories(${CONAN_LIB_DIRS_OPENSSL})

add_library(${LIBRARY_NAME} STATIC ${LIBRARY_SRCS})
target_include_directories(${LIBRARY_NAME} SYSTEM PRIVATE $ENV{INCLUDE})
target_link_libraries(${LIBRARY_NAME} CONAN_PKG::mysql-connector-c CONAN_PKG::fmt CONAN_PKG::boost CONAN_PKG::openssl CONAN_PKG::nlohmann_json)
if(WITH_PCH)
        add_cxx_pch(${LIBRARY_NAME} ${sharedPCH})
endif()
