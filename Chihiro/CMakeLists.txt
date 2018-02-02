cmake_minimum_required(VERSION 3.0.2)
project(Chihiro)

file(GLOB_RECURSE SOURCE_FILES src/*.*)

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/Monster
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/Summon
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/FieldProp
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/Item
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/NPC
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/RespawnObject
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/Object
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/Player
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/Unit
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Globals
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Grids
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Functors
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Account
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Groups
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Dungeons
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Map
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Crafting
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/X2D
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Quests
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Network
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Network/GameNetwork
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Network/AuthNetwork
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Server
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/Skills
                    ${CMAKE_CURRENT_SOURCE_DIR}/src/World
                    ${CMAKE_SOURCE_DIR}/dep/sol2/single/sol
                    ${ACE_INCLUDE_DIR}
                    ${CMAKE_SOURCE_DIR}/dep/liblua
                    ${MYSQL_INCLUDE_DIR})

add_executable(Chihiro ${SOURCE_FILES})
target_link_libraries(Chihiro shared ${MYSQL_LIBRARY} lualib stdc++fs)


if(NOT ACE_USE_EXTERNAL)
    target_link_libraries(Chihiro ace)
    set_target_properties(Chihiro PROPERTIES LINK_FLAGS -lrt)
else()
    target_link_libraries(Chihiro ${ACE_LIBRARIES})
endif()

if(UNIX)
    set(EXECUTABLE_LINK_FLAGS "")
    if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
        set(EXECUTABLE_LINK_FLAGS "-Wl,--no-as-needed -pthread -lrt ${EXECUTABLE_LINK_FLAGS}")
    elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(EXECUTABLE_LINK_FLAGS "-Wl,--no-as-needed -ldl -pthread -lrt ${EXECUTABLE_LINK_FLAGS}")
    endif()
    set_target_properties(Chihiro PROPERTIES LINK_FLAGS ${EXECUTABLE_LINK_FLAGS})
endif()