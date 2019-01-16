set(LIBRARY_NAME Luna)

find_package(OpenSSL)

include_directories(
    ${CMAKE_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/src
    ${CMAKE_CURRENT_SOURCE_DIR}/src/Client
    ${SHARED_INCLUDE_DIR}
    ${Boost_INCLUDE_DIR}
    ${OPENSSL_INCLUDE_DIR}
)

# Server
file(GLOB_RECURSE SRC_GRP_LUNA_CLIENT src/Client/*.cpp src/Client/*.h)
source_group("Luna-Client" FILES ${SRC_GRP_LUNA_CLIENT})

add_executable(${LIBRARY_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/src/Main.cpp ${SRC_GRP_LUNA_CLIENT})
target_link_libraries(${LIBRARY_NAME} shared OpenSSL::SSL OpenSSL::Crypto)

if(UNIX)
    set(EXECUTABLE_LINK_FLAGS "")
    if(CMAKE_SYSTEM_NAME MATCHES "FreeBSD")
        set(EXECUTABLE_LINK_FLAGS "-Wl,--no-as-needed -pthread -lrt ${EXECUTABLE_LINK_FLAGS}")
    elseif(CMAKE_SYSTEM_NAME MATCHES "Linux")
        set(EXECUTABLE_LINK_FLAGS "-Wl,--no-as-needed -ldl -pthread -lrt ${EXECUTABLE_LINK_FLAGS}")
    endif()
    set_target_properties(${LIBRARY_NAME} PROPERTIES LINK_FLAGS ${EXECUTABLE_LINK_FLAGS})
endif()