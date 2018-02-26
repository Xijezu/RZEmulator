#
#  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
#
#  This program is free software; you can redistribute it and/or modify it
#  under the terms of the GNU General Public License as published by the
#  Free Software Foundation; either version 3 of the License, or (at your
#  option) any later version.
#
#  This program is distributed in the hope that it will be useful, but WITHOUT
#  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
#  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
#  more details.
#
#  You should have received a copy of the GNU General Public License along
#  with this program. If not, see <http://www.gnu.org/licenses/>.
#

function(IncludeACE)
    add_subdirectory(${CMAKE_SOURCE_DIR}/dep/acelite)
    if(NOT(${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD"))
        add_definitions(-DHAVE_ACE_STACK_TRACE_H)
    endif()

    set(ACE_INCLUDE_DIR ${CMAKE_SOURCE_DIR}/dep/acelite CACHE INTERNAL "" FORCE)
    set(ACE_LIBRARIES_DIR ${CMAKE_BINARY_DIR}/dep/acelite/ace CACHE INTERNAL "" FORCE)
    set(ACE_LIBRARIES ${CMAKE_BINARY_DIR}/dep/acelite/ace/libace.so CACHE INTERNAL "" FORCE)
endfunction()

set(ACE_USE_EXTERNAL 0)
if(ACE_FORCE_INTERNAL)
    message(STATUS "Using distributed version of ACE")
    IncludeACE()
else()
    message(STATUS "Trying to use external version of ACE")
    find_package(ACE)
    if(ACE_FOUND)
        message(STATUS "Found external ACE, using it !")
        set(ACE_USE_EXTERNAL 1)
        if(EXISTS ${ACE_INCLUDE_DIR}/ace/Stack_Trace.h)
            add_definitions(-DHAVE_ACE_STACK_TRACE_H)
        endif()
    else()
        message(STATUS "External ACE not found, using distributed version instead")
        IncludeACE()
    endif()
endif()