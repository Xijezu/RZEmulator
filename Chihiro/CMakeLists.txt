set(LIBRARY_NAME Chihiro)
if(WITH_PCH)
    set(chihiroPCH "${CMAKE_CURRENT_SOURCE_DIR}/src/PrecompiledHeaders/chihiroPCH.h")
endif()

include_directories(
        ${CMAKE_BINARY_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/src
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/Monster
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/Summon
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/FieldProp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/Item
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/NPC
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/RespawnObject
        ${CMAKE_CURRENT_SOURCE_DIR}/src/Entities/Roamer
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
        ${CMAKE_SOURCE_DIR}/dep/liblua
        ${SHARED_INCLUDE_DIR})
if (UNIX)
    include_directories(${READLINE_INCLUDE_DIR})
endif ()

# Account
file(GLOB SRC_GRP_ACCOUNT src/Account/*.cpp src/Account/*.h)
source_group("Account" FILES ${SRC_GRP_ACCOUNT})
# Crafting
file(GLOB SRC_GRP_CRAFT src/Crafting/*.cpp src/Crafting/*.h)
source_group("Crafting" FILES ${SRC_GRP_CRAFT})
# Dungeons
file(GLOB SRC_GRP_DUNG src/Dungeons/*.cpp src/Dungeons/*.h)
source_group("Dungeons" FILES ${SRC_GRP_DUNG})
# Entities
file(GLOB_RECURSE SRC_GRP_ENTITIES src/Entities/*.cpp src/Entities/*.h)
source_group("Entities" FILES ${SRC_GRP_ENTITIES})
# Functors
file(GLOB SRC_GRP_FUNCTORS src/Functors/*.cpp src/Functors/*.h)
source_group("Functors" FILES ${SRC_GRP_FUNCTORS})
# Globals
file(GLOB SRC_GRP_GLOBALS src/Globals/*.cpp src/Globals/*.h)
source_group("Globals" FILES ${SRC_GRP_GLOBALS})
# Groups
file(GLOB SRC_GRP_GROUP src/Groups/*.cpp src/Groups/*.h)
source_group("Groups" FILES ${SRC_GRP_GROUP})
# Map
file(GLOB SRC_GRP_MAP src/Map/*.cpp src/Map/*.h)
source_group("Map" FILES ${SRC_GRP_MAP})
# Network
file(GLOB_RECURSE SRC_GRP_NETWORK src/Network/*.cpp src/Network/*.h)
source_group("Network" FILES ${SRC_GRP_NETWORK})
# Quests
file(GLOB SRC_GRP_QUESTS src/Quests/*.cpp src/Quests/*.h)
source_group("Quests" FILES ${SRC_GRP_QUESTS})
# Scripting
file(GLOB SRC_GRP_SCRIPTING src/Scripting/*.cpp src/Scripting/*.h)
source_group("Scripting" FILES ${SRC_GRP_SCRIPTING})
# Skills
file(GLOB SRC_GRP_SKILLS src/Skills/*.cpp src/Skills/*.h)
source_group("Skills" FILES ${SRC_GRP_SKILLS})
# World
file(GLOB SRC_GRP_WORLD src/World/*.cpp src/World/*.h)
source_group("World" FILES ${SRC_GRP_WORLD})
# X2D
file(GLOB SRC_GRP_X2D src/X2D/*.cpp src/X2D/*.h)
source_group("X2D" FILES ${SRC_GRP_X2D})

set(CHIHIRO_SRSC
    ${SRC_GRP_ACCOUNT}
    ${SRC_GRP_CRAFT}
    ${SRC_GRP_DUNG}
    ${SRC_GRP_ENTITIES}
    ${SRC_GRP_FUNCTORS}
    ${SRC_GRP_GLOBALS}
    ${SRC_GRP_GROUP}
    ${SRC_GRP_MAP}
    ${SRC_GRP_NETWORK}
    ${SRC_GRP_QUESTS}
    ${SRC_GRP_SCRIPTING}
    ${SRC_GRP_SKILLS}
    ${SRC_GRP_WORLD}
    ${SRC_GRP_X2D}
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp 
)

add_executable(${LIBRARY_NAME} ${CHIHIRO_SRSC})
target_link_libraries(${LIBRARY_NAME} shared lualib)

if(UNIX)
    if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
        set(EXECUTABLE_LINK_FLAGS "-Wl,--no-as-needed -pthread -lrt ${EXECUTABLE_LINK_FLAGS}")
    elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(EXECUTABLE_LINK_FLAGS "-Wl,--no-as-needed -ldl -pthread -lrt ${EXECUTABLE_LINK_FLAGS}")
    endif()
    target_link_libraries(${LIBRARY_NAME}  ${READLINE_LIBRARY})
    set_target_properties(${LIBRARY_NAME}  PROPERTIES LINK_FLAGS ${EXECUTABLE_LINK_FLAGS})
endif()

if(WITH_PCH)
    add_cxx_pch(${LIBRARY_NAME} ${chihiroPCH})
endif()