set(LIBRARY_NAME ServerMonitor)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Client
    ${CMAKE_CURRENT_SOURCE_DIR}/src/ServerMonitor
    ${SHARED_INCLUDE_DIR}
    ${CMAKE_SOURCE_DIR}/dep/json/single_include
    ${CMAKE_SOURCE_DIR}/dep/pistache/include
    ${Boost_INCLUDE_DIR}
)

find_package(ZLIB)

# Server
file(GLOB_RECURSE SRC_GRP_SERVER_MONITOR_CLIENT src/Client/*.cpp src/Client/*.h)
source_group("ServerMonitor-Client" FILES ${SRC_GRP_SERVER_MONITOR_CLIENT})

file(GLOB SRC_GRP_SERVER_MONITOR src/ServerMonitor/*.cpp src/ServerMonitor/*.h)
source_group("ServerMonitor" FILES ${SRC_GRP_SERVER_MONITOR})

add_executable(${LIBRARY_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp ${SRC_GRP_SERVER_MONITOR_CLIENT} ${SRC_GRP_SERVER_MONITOR})
target_link_libraries(${LIBRARY_NAME} shared ${ZLIB_LIBRARIES})

if(UNIX)
    set(EXECUTABLE_LINK_FLAGS "")
    if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
        set(EXECUTABLE_LINK_FLAGS "-Wl,--no-as-needed -pthread -lrt ${EXECUTABLE_LINK_FLAGS}")
    elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(EXECUTABLE_LINK_FLAGS "-Wl,--no-as-needed -ldl -pthread -lrt ${EXECUTABLE_LINK_FLAGS}")
    endif()
    set_target_properties(${LIBRARY_NAME} PROPERTIES LINK_FLAGS ${EXECUTABLE_LINK_FLAGS})
endif()