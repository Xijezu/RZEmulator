#pragma once
/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 * Copyright (C) 2005-2009 MaNGOS <http://getmangos.com/>
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
#include <chrono>

#include "Define.h"

inline uint32_t getMSTime()
{
    using namespace std::chrono;

    static const steady_clock::time_point ApplicationStartTime = steady_clock::now();

    return uint32_t(duration_cast<milliseconds>(steady_clock::now() - ApplicationStartTime).count());
}

inline uint32_t getMSTimeDiff(uint32_t oldMSTime, uint32_t newMSTime)
{
    // getMSTime() have limited data range and this is case when it overflow in this tick
    if (oldMSTime > newMSTime)
        return (0xFFFFFFFF - oldMSTime) + newMSTime;
    else
        return newMSTime - oldMSTime;
}

inline uint32_t GetMSTimeDiffToNow(uint32_t oldMSTime)
{
    return getMSTimeDiff(oldMSTime, getMSTime());
}

struct IntervalTimer {
public:
    IntervalTimer()
        : _interval(0)
        , _current(0)
    {
    }

    void Update(time_t diff)
    {
        _current += diff;
        if (_current < 0)
            _current = 0;
    }

    bool Passed() { return _current >= _interval; }

    void Reset()
    {
        if (_current >= _interval)
            _current %= _interval;
    }

    void SetCurrent(time_t current) { _current = current; }

    void SetInterval(time_t interval) { _interval = interval; }

    time_t GetInterval() const { return _interval; }

    time_t GetCurrent() const { return _current; }

private:
    time_t _interval;
    time_t _current;
};

struct TimeTracker {
public:
    TimeTracker(time_t expiry)
        : i_expiryTime(expiry)
    {
    }

    void Update(time_t diff) { i_expiryTime -= diff; }

    bool Passed() const { return i_expiryTime <= 0; }

    void Reset(time_t interval) { i_expiryTime = interval; }

    time_t GetExpiry() const { return i_expiryTime; }

private:
    time_t i_expiryTime;
};

struct TimeTrackerSmall {
public:
    TimeTrackerSmall(uint32_t expiry = 0)
        : i_expiryTime(expiry)
    {
    }

    void Update(int32_t diff) { i_expiryTime -= diff; }

    bool Passed() const { return i_expiryTime <= 0; }

    void Reset(uint32_t interval) { i_expiryTime = interval; }

    int32_t GetExpiry() const { return i_expiryTime; }

private:
    int32_t i_expiryTime;
};

struct PeriodicTimer {
public:
    PeriodicTimer(int32_t period, int32_t start_time)
        : i_period(period)
        , i_expireTime(start_time)
    {
    }

    bool Update(const uint32_t diff)
    {
        if ((i_expireTime -= diff) > 0)
            return false;

        i_expireTime += i_period > int32_t(diff) ? i_period : diff;
        return true;
    }

    void SetPeriodic(int32_t period, int32_t start_time)
    {
        i_expireTime = start_time;
        i_period = period;
    }

    // Tracker interface
    void TUpdate(int32_t diff) { i_expireTime -= diff; }

    bool TPassed() const { return i_expireTime <= 0; }

    void TReset(int32_t diff, int32_t period) { i_expireTime += period > diff ? period : diff; }

private:
    int32_t i_period;
    int32_t i_expireTime;
};