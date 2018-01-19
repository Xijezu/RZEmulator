cmake_minimum_required(VERSION 3.0.2)
project(Chihiro)

file(GLOB_RECURSE sources_Entities src/Entities/*.cpp src/Entities/*.h)
file(GLOB_RECURSE sources_Skills src/Skills/*.cpp src/Skills/*.h)
file(GLOB_RECURSE sources_Scripting src/Scripting/*.cpp src/Scripting/*.h)
file(GLOB_RECURSE sources_X2D src/X2D/*.cpp src/X2D/*.h)
file(GLOB_RECURSE sources_Crafting src/Crafting/*.cpp src/Crafting/*.h)
file(GLOB_RECURSE sources_Quests src/Quests/*.cpp src/Quests/*.h)
file(GLOB_RECURSE sources_Dungeons src/Dungeons/*.cpp src/Dungeons/*.h)
file(GLOB_RECURSE sources_Globals src/Globals/*.cpp src/Globals/*.h)
file(GLOB_RECURSE sources_Groups src/Groups/*.cpp src/Groups/*.h)
file(GLOB_RECURSE sources_Grids src/Grids/*.cpp src/Grids/*.h)
file(GLOB_RECURSE sources_Map src/Map/*.cpp src/Map/*.h)
file(GLOB_RECURSE sources_Network src/Network/*.cpp src/Network/*.h)
file(GLOB_RECURSE sources_World src/World/*.cpp src/World/*.h)
file(GLOB sources_localdir src/*.cpp src/*.h)

set(SOURCE_FILES ${sources_Entities}
                 ${sources_Scripting}
                 ${sources_X2D}
                 ${sources_Quests}
                 ${sources_Crafting}
                 ${sources_Globals}
                 ${sources_Groups}
                 ${sources_Grids}
                 ${sources_Dungeons}
                 ${sources_Map}
                 ${sources_Skills}
                 ${sources_Network}
                 ${sources_localdir}
                 ${sources_World})

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
                    ${MYSQL_INCLUDE_DIR}
                    ${ACE_INCLUDE_DIR})

add_executable(Chihiro ${SOURCE_FILES} ${SHARED_SRC})
target_link_libraries(Chihiro  ${MYSQL_LIBRARY} ${CMAKE_THREAD_LIBS_INIT} ${ACE_LIBRARIES} ${LUA_LIBRARIES} stdc++fs)