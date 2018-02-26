cmake_minimum_required(VERSION 3.0.2)
project(Mononoke)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Lists
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Server
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Server/AuthClient
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Server/AuthGame
                    ${CMAKE_BINARY_DIR}
                    ${SHARED_INCLUDE_DIR}
                    ${ACE_INCLUDE_DIR}
                    ${MYSQL_INCLUDE_DIR})

file(GLOB_RECURSE SOURCE_FILES src/*.*)

add_executable(Mononoke ${SOURCE_FILES} )
target_link_libraries(Mononoke shared ${MYSQL_LIB})
if(ACE_USE_EXTERNAL)
    target_link_libraries(Mononoke ${ACE_LIBRARIES})
else()
    target_link_libraries(Mononoke ace)
endif()

if(UNIX)
    set(EXECUTABLE_LINK_FLAGS "")
    if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
        set(EXECUTABLE_LINK_FLAGS "-Wl,--no-as-needed -pthread -lrt ${EXECUTABLE_LINK_FLAGS}")
    elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(EXECUTABLE_LINK_FLAGS "-Wl,--no-as-needed -ldl -pthread -lrt ${EXECUTABLE_LINK_FLAGS}")
    endif()
    set_target_properties(Mononoke PROPERTIES LINK_FLAGS ${EXECUTABLE_LINK_FLAGS})
endif()