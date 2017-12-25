/*
  *  Copyright (C) 2017 Xijezu <http://xijezu.com/>
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

#include "Summon.h"
#include "MemPool.h"
#include "ObjectMgr.h"
#include "DatabaseEnv.h"
#include "Messages.h"
#include "World.h"

// static
void Summon::EnterPacket(XPacket &pEnterPct, Summon *pSummon)
{
    Unit::EnterPacket(pEnterPct, pSummon);
    pEnterPct << pSummon->m_pMaster->GetHandle();
    Messages::GetEncodedInt(pEnterPct, pSummon->GetSummonCode());
    pEnterPct.fill(pSummon->GetName(), 19);
};


Summon::Summon(uint pHandle, uint pIdx) : Unit(true)
{
#ifdef _MSC_VER
#   pragma warning(default:4355)
#endif
    _mainType = MT_NPC; // dont question it :^)
    _subType  = ST_Summon;
    _objType  = OBJ_MOVABLE;
    _valuesCount = UNIT_END;

    _InitValues();
    SetInt32Value(UNIT_FIELD_HANDLE, pHandle);
    SetSummonInfo(pIdx);
}

Summon* Summon::AllocSummon(Player * pMaster, uint pCode)
{
    Summon* summon = sMemoryPool->AllocSummon(pCode);
    summon->m_pMaster = pMaster;
}

void Summon::SetSummonInfo(int idx)
{
    m_tSummonBase = sObjectMgr->GetSummonBase(idx);
    SetCurrentJob(idx);
}

int Summon::GetSummonCode()
{
    return m_tSummonBase.id;
}

uint32_t Summon::GetCardHandle()
{
    ACE_ASSERT(m_pItem);

    return m_pItem->m_nHandle;
}

void Summon::DB_InsertSummon(Player *pMaster, Summon *pSummon)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_SUMMON);
    stmt->setInt32(0, pSummon->GetUInt32Value(UNIT_FIELD_UID));         // handle
    stmt->setInt32(1, 0);                                               // account_id
    stmt->setInt32(2, pMaster->GetUInt32Value(UNIT_FIELD_UID));         // owner_id
    stmt->setInt64(3, pSummon->GetSummonCode());                        // summon_id
    stmt->setInt64(4, pSummon->m_pItem->m_Instance.UID);                // card_uid
    stmt->setUInt64(5, pSummon->GetEXP());
    stmt->setInt32(6, pSummon->GetJP());
    stmt->setUInt64(7, 0);                                              // Last Decreased EXP
    stmt->setString(8, pSummon->GetName());
    stmt->setInt32(9, 1);                                               // transform
    stmt->setInt32(10, pSummon->getLevel());
    stmt->setInt32(11, pSummon->GetCurrentJLv());
    stmt->setInt32(12, 1);                                              // max lvl
    stmt->setInt32(13, 0);                                              // fp
    stmt->setInt32(14, 0);
    stmt->setInt32(15, 0);
    stmt->setInt32(16, 0);
    stmt->setInt32(17, 0);                                              // prev_...stuff
    stmt->setInt32(18, 0);                                              // sp
    stmt->setInt32(19, pSummon->GetMaxHealth());
    stmt->setInt32(20, pSummon->GetMaxMana());
    CharacterDatabase.Execute(stmt);
}

void Summon::OnUpdate()
{
    uint ct = sWorld->GetArTime();
    if(IsInWorld()) {
        if(GetHealth() == 0) {
            // Unsummon after some time
        }

        if(bIsMoving && IsInWorld()) {
            processWalk(ct);
            lastProcessTime = ct;
            return;
        }

        if(HasFlag(UNIT_FIELD_STATUS, StatusFlags::MovePending)) {
            processPendingMove();
        }
        lastProcessTime = ct;
    }
    Unit::OnUpdate();
}

void Summon::processWalk(uint t)
{
    // Do Ride check here
    ArMoveVector tmp_mv(this);
    tmp_mv.Step(t);
    if((tmp_mv.GetPositionX() / g_nRegionSize) != (GetPositionX() / g_nRegionSize) ||
            (tmp_mv.GetPositionY() / g_nRegionSize) != (GetPositionY() / g_nRegionSize) ||
            !tmp_mv.bIsMoving)
    {
        if(bIsMoving && IsInWorld()) {
            sWorld->onRegionChange(this, t - lastStepTime, !tmp_mv.bIsMoving);
        }
    }
}
