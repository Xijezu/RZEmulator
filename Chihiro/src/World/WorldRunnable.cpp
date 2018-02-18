/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the
 *  Free Software Foundation; either version 3 of the License, or (at your
 *  option) any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#include "Common.h"
#include "World.h"
#include "WorldRunnable.h"
#include "AuthNetwork.h"
#include "Timer.h"
#include "MemPool.h"
#include "ObjectMgr.h"
#include "Maploader.h"

#define WORLD_SLEEP_CONST 50

void WorldRunnable::run()
{
    uint32 realCurrTime = 0;
    uint32 realPrevTime = getMSTime();

    uint32 prevSleepTime = 0;
    World::m_worldLoopCounter = 0;

    // is Stopped event
    while (!World::IsStopped())
    {
        ++World::m_worldLoopCounter;

        realCurrTime = getMSTime();
        uint32 diff = getMSTimeDiff(realPrevTime, realCurrTime);

        sWorld->Update(diff);
        realPrevTime = realCurrTime;

        if (diff <= WORLD_SLEEP_CONST + prevSleepTime)
        {
            prevSleepTime = WORLD_SLEEP_CONST + prevSleepTime - diff;
            ACE_Based::Thread::Sleep(prevSleepTime);
        }
        else
            prevSleepTime = 0;
    }

    sWorld->KickAll();
    sAuthNetwork->close();
    sWorldSocketMgr->StopNetwork();

    sMemoryPool->Destroy();

    sObjectMgr->UnloadAll();
    sMapContent->UnloadAll();
}