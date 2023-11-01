set(LIBRARY_NAME NGRDB)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${SHARED_INCLUDE_DIR}
)

# Server
#file(GLOB_RECURSE SRC_GRP_SHIZUKE_SERVER src/Session/*.cpp src/Session/*.h)
#source_group("Shizuke-Server" FILES ${SRC_GRP_SHIZUKE_SERVER})

add_executable(${LIBRARY_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp)
target_link_libraries(${LIBRARY_NAME} shared)
add_definitions(-D_RDBGENTEST)
if(UNIX)
    set(EXECUTABLE_LINK_FLAGS "")
    if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
        set(EXECUTABLE_LINK_FLAGS "-Wl,--no-as-needed -pthread -lrt ${EXECUTABLE_LINK_FLAGS}")
    elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(EXECUTABLE_LINK_FLAGS "-Wl,--no-as-needed -ldl -pthread -lrt ${EXECUTABLE_LINK_FLAGS}")
    endif()
    set_target_properties(${LIBRARY_NAME} PROPERTIES LINK_FLAGS ${EXECUTABLE_LINK_FLAGS})
endif()