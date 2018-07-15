/*
 * Copyright (C) 2011-2017 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2017 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2017 MaNGOS <https://www.getmangos.eu/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 3 of the License, or (at your
 * option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef NGEMITY_DEFINE_H
#define NGEMITY_DEFINE_H

#include "CompilerDefs.h"
#include <boost/detail/endian.hpp>

#define EPIC 4
#define NGEMITY_LITTLEENDIAN 0
#define NGEMITY_BIGENDIAN    1

#if !defined(NGEMITY_ENDIAN)
#  if defined (BOOST_BIG_ENDIAN)
#    define NGEMITY_ENDIAN NGEMITY_BIGENDIAN
#  else
#    define NGEMITY_ENDIAN NGEMITY_LITTLEENDIAN
#  endif
#endif

#include <cstddef>
#include <cinttypes>
#include <climits>

#if PLATFORM == PLATFORM_WINDOWS
#  define NGEMITY_PATH_MAX 260
#  ifndef DECLSPEC_NORETURN
#    define DECLSPEC_NORETURN __declspec(noreturn)
#  endif //DECLSPEC_NORETURN
#  ifndef DECLSPEC_DEPRECATED
#    define DECLSPEC_DEPRECATED __declspec(deprecated)
#  endif //DECLSPEC_DEPRECATED
#else //PLATFORM != PLATFORM_WINDOWS
#  define NGEMITY_PATH_MAX PATH_MAX
#  define DECLSPEC_NORETURN
#  define DECLSPEC_DEPRECATED
#endif //PLATFORM

#if COMPILER == COMPILER_GNU
#  define ATTR_NORETURN __attribute__((noreturn))
#  define ATTR_PRINTF(F, V) __attribute__ ((format (printf, F, V)))
#  define ATTR_DEPRECATED __attribute__((deprecated))
#else //COMPILER != COMPILER_GNU
#  define ATTR_NORETURN
#  define ATTR_PRINTF(F, V)
#  define ATTR_DEPRECATED
#endif //COMPILER == COMPILER_GNU

typedef unsigned long  DWORD;
typedef unsigned short WORD;

#ifndef LOWORD
# define LOWORD(a) ((WORD)(a))
#endif // #ifndef LOWORD
#ifndef HIWORD
# define HIWORD(a) ((WORD)(((DWORD)(a) >> 16) & 0xFFFF))
#endif // #ifndef HIWORD


#define UI64FMTD "%" PRIu64
#define UI64LIT(N) UINT64_C(N)

#define SI64FMTD "%" PRId64
#define SI64LIT(N) INT64_C(N)

#define SZFMTD "%" PRIuPTR

typedef int64_t int64;
typedef int32_t int32;
typedef int16_t int16;
typedef int8_t int8;
typedef uint64_t uint64;
typedef uint32_t uint32;
typedef uint16_t uint16;
typedef uint8_t uint8;

typedef uint32 uint;
typedef uint16 ushort;
#endif //NGEMITY_DEFINE_H