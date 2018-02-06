file(GLOB_RECURSE SHARED_SOURCE_FILES *.h *.cpp)

include_directories(
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/Configuration
        ${CMAKE_CURRENT_SOURCE_DIR}/Encryption
        ${CMAKE_CURRENT_SOURCE_DIR}/Database
        ${CMAKE_CURRENT_SOURCE_DIR}/Database/Implementation
        ${CMAKE_CURRENT_SOURCE_DIR}/Debugging
        ${CMAKE_CURRENT_SOURCE_DIR}/Dynamic
        ${CMAKE_CURRENT_SOURCE_DIR}/Dynamic/LinkedReference
        ${CMAKE_CURRENT_SOURCE_DIR}/Logging
        ${CMAKE_CURRENT_SOURCE_DIR}/Server
        ${CMAKE_CURRENT_SOURCE_DIR}/Threading
        ${CMAKE_CURRENT_SOURCE_DIR}/utf8
        ${CMAKE_CURRENT_SOURCE_DIR}/Utilities
        ${ACE_INCLUDE_DIR}
        ${MYSQL_INCLUDE_DIR}
)

add_library(shared STATIC ${SHARED_SOURCE_FILES})

target_link_libraries(shared ${ACE_LIBRARY})
