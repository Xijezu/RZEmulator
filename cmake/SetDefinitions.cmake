#
# This code is part of MaNGOS. Contributor & Copyright details are in AUTHORS/THANKS.
#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#

# Set build-directive (used in core to tell which buildtype we used)
add_definitions(-D_BUILD_DIRECTIVE="'${CMAKE_BUILD_TYPE}'")

#Set config install path correctly from given path
string(FIND "${CONF_DIR}" ":" CONF_DIR_ABSOLUTE)
if(${CONF_DIR_ABSOLUTE} EQUAL -1)
    #Path was not absolute
    set(CONF_INSTALL_DIR "${CMAKE_INSTALL_PREFIX}/${CONF_DIR}")
    if(MSVC)
        set(CONF_COPY_DIR "${CMAKE_BINARY_DIR}/bin/$(ConfigurationName)/${CONF_DIR}")
    endif()
else()
    #Path was absolute
    set(CONF_INSTALL_DIR "${CONF_DIR}")
    if(MSVC)
        set(CONF_COPY_DIR "${CONF_DIR}")
    endif()
endif()

if(WIN32)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
    set(BIN_DIR ${CMAKE_INSTALL_PREFIX}/)
    set(LIBS_DIR ${CMAKE_INSTALL_PREFIX}/)

    if(MSVC)
        if(PLATFORM EQUAL 64)
            # This definition is necessary to work around a bug with Intellisense described
            # here: http://tinyurl.com/2cb428.  Syntax highlighting is important for proper
            # debugger functionality.
            add_definitions("-D_WIN64")

            # Enable extended object support for debug compiles on X64 (not required on X86)
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /bigobj")
        else()
            # Mark 32 bit executables large address aware so they can use > 2GB address space
            set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /LARGEADDRESSAWARE")

        endif()

        if(NOT CMAKE_GENERATOR MATCHES "Visual Studio 7")
            set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} /wd4996 /wd4355 /wd4244 /wd4985 /wd4267 /MP")
            set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /wd4996 /wd4355 /wd4244 /wd4267 /MP")
            set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} /wd4996 /wd4355 /wd4244 /wd4985 /wd4267 /MP")
            set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /wd4996 /wd4355 /wd4244 /wd4985 /wd4267 /MP")
        endif()

        # Specify the maximum PreCompiled Header memory allocation limit
        # Fixes a compiler-problem when using PCH - the /Ym flag is adjusted by the compiler in MSVC2012, hence we need to set an upper limit with /Zm to avoid discrepancies)
        # (And yes, this is a verified , unresolved bug with MSVC... *sigh*)
        string(REGEX REPLACE "/Zm[0-9]+ *" "" CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS})
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zm600" CACHE STRING "" FORCE)
    elseif(MINGW)
        if(NOT DEBUG)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --no-warnings")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --no-warnings")
        else()
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wextra -Winit-self -Winvalid-pch -Wfatal-errors -g3")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Winit-self -Winvalid-pch -Wfatal-errors -Woverloaded-virtual -g3")
        endif()
    endif()
elseif(UNIX)
    set(BIN_DIR ${CMAKE_INSTALL_PREFIX}/bin)
    set(LIBS_DIR ${CMAKE_INSTALL_PREFIX}/lib)

    # For Unix systems set the rpath so that libraries are found
    set(CMAKE_INSTALL_RPATH "${LIBS_DIR}")
    set(CMAKE_INSTALL_NAME_DIR "${LIBS_DIR}")
    # Run out of build tree
    set(CMAKE_BUILD_WITH_INSTALL_RPATH OFF)

    if(CMAKE_C_COMPILER MATCHES "gcc" OR CMAKE_C_COMPILER_ID STREQUAL "GNU")
        set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=gnu99")
        if(NOT DEBUG)
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} --no-warnings")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} --no-warnings -std=c++11 -Wno-narrowing -Wno-deprecated-register")
        else()
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -W -Wall -Wextra -Winit-self -Winvalid-pch -Wfatal-errors -g3")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -W -Wall -Wextra -Winit-self -Winvalid-pch -Wfatal-errors -Woverloaded-virtual -g3 -std=c++11 -Wno-narrowing -Wno-deprecated-register")
        endif()

    elseif(CMAKE_C_COMPILER MATCHES "icc")
        if(DEBUG)
          add_definitions(-w1)
          add_definitions(-g)
        endif()
    elseif(CMAKE_C_COMPILER MATCHES "clang" OR CMAKE_C_COMPILER_ID STREQUAL "Clang")
        if(DEBUG)
            set(WARNING_FLAGS "-W -Wall -Wextra -Winit-self -Wfatal-errors")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${WARNING_FLAGS}")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${WARNING_FLAGS} -Woverloaded-virtual")
            set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g3")
            set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g3")
        endif()

        # -Wno-narrowing needed to suppress a warning in g3d
        # -Wno-deprecated-register is needed to suppress 185 gsoap warnings on Unix systems.
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14 -Wno-narrowing -Wno-deprecated-register -Wno-ignored-attributes -Wno-deprecated-declarations")
    endif()
endif()