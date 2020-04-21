set(LIBRARY_NAME Mononoke)
if(WITH_PCH)
    set(mononokePCH "${CMAKE_CURRENT_SOURCE_DIR}/src/PrecompiledHeaders/mononokePCH.h")
endif()

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Lists
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Server
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Server/AuthClient
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Server/AuthGame
    ${SHARED_INCLUDE_DIR}
    ${CONAN_INCLUDE_DIRS_BOOST}
    ${MYSQL_INCLUDE_DIR}
)

# Lists
file(GLOB SRC_GRP_LISTS src/Lists/*.cpp src/Lists/*.h)
source_group("Lists" FILES ${SRC_GRP_LISTS})
# Server
file(GLOB_RECURSE SRC_GRP_MONONOKE_SERVER src/Server/*.cpp src/Server/*.h)
source_group("Mononoke-Server" FILES ${SRC_GRP_MONONOKE_SERVER})

set(MONONOKE_SRSC
    ${SRC_GRP_LISTS}
    ${SRC_GRP_MONONOKE_SERVER}
)

add_executable(${LIBRARY_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp ${MONONOKE_SRSC})
target_link_libraries(${LIBRARY_NAME} shared)

if(UNIX)
    set(EXECUTABLE_LINK_FLAGS "")
    if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
        set(EXECUTABLE_LINK_FLAGS "-Wl,--no-as-needed -pthread -lrt ${EXECUTABLE_LINK_FLAGS}")
    elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(EXECUTABLE_LINK_FLAGS "-Wl,--no-as-needed -ldl -pthread -lrt ${EXECUTABLE_LINK_FLAGS}")
    endif()
    set_target_properties(${LIBRARY_NAME} PROPERTIES LINK_FLAGS ${EXECUTABLE_LINK_FLAGS})
endif()

if(WITH_PCH)
    add_cxx_pch(${LIBRARY_NAME} ${mononokePCH})
endif()