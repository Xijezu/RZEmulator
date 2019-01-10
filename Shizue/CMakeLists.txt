set(LIBRARY_NAME Shizue)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Session
    ${SHARED_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${MYSQL_INCLUDE_DIR}
)

# Server
file(GLOB_RECURSE SRC_GRP_SHIZUKE_SERVER src/Session/*.cpp src/Session/*.h)
source_group("Shizuke-Server" FILES ${SRC_GRP_SHIZUKE_SERVER})

add_executable(${LIBRARY_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp ${SRC_GRP_SHIZUKE_SERVER})
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