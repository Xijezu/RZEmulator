#pragma once
/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
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
#include <ctime>
#include <string>

#include "Define.h"
#include "LogCommon.h"

struct LogMessage
{
    LogMessage(LogLevel _level, std::string const &_type, std::string &&_text);
    LogMessage(LogLevel _level, std::string const &_type, std::string &&_text, std::string &&_param1);

    LogMessage(LogMessage const & /*other*/) = delete;
    LogMessage &operator=(LogMessage const & /*other*/) = delete;

    static std::string getTimeStr(time_t time);
    std::string getTimeStr();

    LogLevel const level;
    std::string const type;
    std::string const text;
    std::string prefix;
    std::string param1;
    time_t mtime;

    ///@ Returns size of the log message content in bytes
    uint32_t Size() const { return static_cast<uint32_t>(prefix.size() + text.size()); }
};