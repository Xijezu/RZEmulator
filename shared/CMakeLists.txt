file(GLOB_RECURSE SHARED_SOURCE_FILES *.h *.cpp)

set(SHARED_INCLUDE_DIR
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
        ${CMAKE_SOURCE_DIR}/shared/Utilities
        CACHE INTERNAL "" FORCE) # dont mind, dirty hack

include_directories(
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${SHARED_INCLUDE_DIR}
        ${ACE_INCLUDE_DIR}
        ${MYSQL_INCLUDE_DIR})

add_library(shared STATIC ${SHARED_SOURCE_FILES})
target_link_libraries(shared ${ACE_LIBRARY})
