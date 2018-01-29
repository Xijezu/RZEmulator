cmake_minimum_required(VERSION 3.0.2)
project(Mononoke)

add_definitions(-DIS_MONONOKE)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Lists
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Server
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Server/AuthClient
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Server/AuthGame
                    ${MYSQL_INCLUDE_DIR})

file(GLOB_RECURSE SOURCE_FILES src/*.*)

add_executable(Mononoke ${SOURCE_FILES} )
target_link_libraries(Mononoke shared ${MYSQL_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${ACE_LIBRARIES})