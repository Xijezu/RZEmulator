/*
 * Copyright (C) 2011-2014 Project SkyFire <http://www.projectskyfire.org/>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 * Copyright (C) 2005-2014 MaNGOS <http://getmangos.com/>
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

#ifndef SKYFIRE_COMMON_H
#define SKYFIRE_COMMON_H

#include "Define.h"
#include <ace/Stack_Trace.h>
#include <ace/OS_NS_unistd.h>
#include <ace/Null_Mutex.h>
#include <ace/Singleton.h>
#include <cstdlib>
#include <unordered_map>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <math.h>
#include <errno.h>
#include <signal.h>
#include <assert.h>

#include "Log.h"
#include "Config.h"

#if PLATFORM == PLATFORM_WINDOWS
#define STRCASECMP stricmp
#else
#define STRCASECMP strcasecmp
#endif

#include <set>
#include <list>
#include <string>
#include <map>
#include <queue>
#include <sstream>
#include <algorithm>

#include "Threading/LockedQueue.h"
#include "Threading/Threading.h"

#include <ace/Basic_Types.h>
#include <ace/Guard_T.h>
#include <ace/RW_Thread_Mutex.h>
#include <ace/Thread_Mutex.h>

#if PLATFORM == PLATFORM_WINDOWS
#  include <ace/config-all.h>
// XP winver - needed to compile with standard leak check in MemoryLeaks.h
// uncomment later if needed
//#define _WIN32_WINNT 0x0501
#  include <ws2tcpip.h>
//#undef WIN32_WINNT
#else
#  include <sys/types.h>
#  include <sys/ioctl.h>
#  include <sys/socket.h>
#  include <netinet/in.h>
#  include <unistd.h>
#  include <netdb.h>
#endif

#if COMPILER == COMPILER_MICROSOFT

#include <float.h>

#define I32FMT "%08I32X"
#define I64FMT "%016I64X"
//#define snprintf _snprintf
#define atoll _atoi64
#define vsnprintf _vsnprintf
#define finite(X) _finite(X)
#define llabs _abs64

#else

#define stricmp strcasecmp
#define strnicmp strncasecmp
#define I32FMT "%08X"
#define I64FMT "%016llX"

#endif

#define UI64FMTD ACE_UINT64_FORMAT_SPECIFIER
#define UI64LIT(N) ACE_UINT64_LITERAL(N)

#define SI64FMTD ACE_INT64_FORMAT_SPECIFIER
#define SI64LIT(N) ACE_INT64_LITERAL(N)

#define SIZEFMTD ACE_SIZE_T_FORMAT_SPECIFIER

inline float finiteAlways(float f) { return finite(f) ? f : 0.0f; }

#define atol(a) strtoul( a, NULL, 10)

#define STRINGIZE(a) #a

using namespace std::literals::string_literals;

enum TimeConstants
{
    MINUTE          = 60,
    HOUR            = MINUTE*60,
    DAY             = HOUR*24,
    WEEK            = DAY*7,
    MONTH           = DAY*30,
    YEAR            = MONTH*12,
    IN_MILLISECONDS = 1000
};

enum LocaleConstant
{
    LOCALE_enUS = 0,
    LOCALE_koKR = 1,
    LOCALE_frFR = 2,
    LOCALE_deDE = 3,
    LOCALE_zhCN = 4,
    LOCALE_zhTW = 5,
    LOCALE_esES = 6,
    LOCALE_esMX = 7,
    LOCALE_ruRU = 8
};

const uint8 TOTAL_LOCALES = 9;
const LocaleConstant DEFAULT_LOCALE = LOCALE_enUS;

#define MAX_LOCALES 8
#define MAX_ACCOUNT_TUTORIAL_VALUES 8

extern char const* localeNames[TOTAL_LOCALES];

LocaleConstant GetLocaleByName(const std::string& name);

typedef std::vector<std::string> StringVector;

// we always use stdlibc++ std::max/std::min, undefine some not C++ standard defines (Win API and some other platforms)
#ifdef max
#undef max
#endif

#ifdef min
#undef min
#endif

#ifndef M_PI
#define M_PI            3.14159265358979323846f
#endif

#ifndef M_PI_F
#define M_PI_F        float(M_PI)
#endif

#define MAX_QUERY_LEN 32*1024

#define SKYFIRE_GUARD(MUTEX, LOCK) \
  ACE_Guard< MUTEX > SKYFIRE_GUARD_OBJECT (LOCK); \
    if (SKYFIRE_GUARD_OBJECT.locked() == 0) ASSERT(false);

//! For proper implementation of multiple-read, single-write pattern, use
//! ACE_RW_Mutex as underlying @MUTEX
# define SKYFIRE_WRITE_GUARD(MUTEX, LOCK) \
  ACE_Write_Guard< MUTEX > SKYFIRE_GUARD_OBJECT (LOCK); \
    if (SKYFIRE_GUARD_OBJECT.locked() == 0) ASSERT(false);

//! For proper implementation of multiple-read, single-write pattern, use
//! ACE_RW_Mutex as underlying @MUTEX
# define SKYFIRE_READ_GUARD(MUTEX, LOCK) \
  ACE_Read_Guard< MUTEX > SKYFIRE_GUARD_OBJECT (LOCK); \
    if (SKYFIRE_GUARD_OBJECT.locked() == 0) ASSERT(false);

#endif