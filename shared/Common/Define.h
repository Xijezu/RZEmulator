#pragma once
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
#include <boost/predef/other/endian.h>

#include "CompilerDefs.h"
#include "PacketEpics.h"

#define EPIC EPIC_4_1_1
#define NGEMITY_LITTLEENDIAN 0
#define NGEMITY_BIGENDIAN 1

#if !defined(NGEMITY_ENDIAN)
#if defined(BOOST_ENDIAN_BIG_BYTE) && BOOST_ENDIAN_BIG_BYTE
#define NGEMITY_ENDIAN NGEMITY_BIGENDIAN
#else
#define NGEMITY_ENDIAN NGEMITY_LITTLEENDIAN
#endif
#endif

#include <cinttypes>
#include <climits>
#include <cstddef>

#if PLATFORM == PLATFORM_WINDOWS
#define NGEMITY_PATH_MAX 260
#ifndef DECLSPEC_NORETURN
#define DECLSPEC_NORETURN __declspec(noreturn)
#endif // DECLSPEC_NORETURN
#ifndef DECLSPEC_DEPRECATED
#define DECLSPEC_DEPRECATED __declspec(deprecated)
#endif // DECLSPEC_DEPRECATED
#else // PLATFORM != PLATFORM_WINDOWS
#define NGEMITY_PATH_MAX PATH_MAX
#define DECLSPEC_NORETURN
#define DECLSPEC_DEPRECATED
#endif // PLATFORM

#if defined(_MSC_VER) && _MSC_VER < 1500 // VC++ 8.0 and below
#define snprintf _snprintf
#endif

#if COMPILER == COMPILER_GNU
#define ATTR_NORETURN __attribute__((noreturn))
#define ATTR_PRINTF(F, V) __attribute__((format(printf, F, V)))
#define ATTR_DEPRECATED __attribute__((deprecated))
#else // COMPILER != COMPILER_GNU
#define ATTR_NORETURN
#define ATTR_PRINTF(F, V)
#define ATTR_DEPRECATED
#endif // COMPILER == COMPILER_GNU

#define UI64FMTD "%" PRIu64
#define UI64LIT(N) UINT64_C(N)

#define SI64FMTD "%" PRId64
#define SI64LIT(N) INT64_C(N)

#define SZFMTD "%" PRIuPTR

#define NG_MARIADB