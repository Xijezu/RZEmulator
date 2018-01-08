cmake_minimum_required(VERSION 3.0.2)
project(Mononoke)

#set(CMAKE_CXX_STANDARD 11)

find_package (Threads)
find_package(MYSQL REQUIRED)
find_package(ACE REQUIRED)

include_directories(src
                    src/Lists
                    src/Server
                    ${MYSQL_INCLUDE_DIR})

file(GLOB_RECURSE SOURCE_FILES src/*.*)

add_executable(Mononoke ${SOURCE_FILES} ${SHARED_SRC} )
target_link_libraries(Mononoke  ${MYSQL_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${ACE_LIBRARIES})