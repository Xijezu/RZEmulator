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
#include "ClientPackets.h"
#include "ArRegion.h"
// static
void Summon::EnterPacket(XPacket &pEnterPct, Summon *pSummon, Player* pPlayer)
{
    Unit::EnterPacket(pEnterPct, pSummon, pPlayer);
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
    _valuesCount = BATTLE_FIELD_END;

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
    if(m_tSummonBase == nullptr)
        ASSERT(false);
    SetCurrentJob(idx);
    this->m_nTransform = m_tSummonBase->form;
}

int Summon::GetSummonCode()
{
    return m_tSummonBase->id;
}

uint32_t Summon::GetCardHandle()
{
    if(m_pItem != nullptr)
        return m_pItem->m_nHandle;
    else
        return 0;
}

void Summon::DB_UpdateSummon(Player *pMaster, Summon *pSummon)
{
    // PrepareStatement(CHARACTER_UPD_SUMMON, "UPDATE Summon SET account_id = ?, owner_id = ?, code = ?,
    // exp = ?, jp = ?, last_decreased_exp = ?, name = ?, transform = ?, lv = ?, jlv = ?, max_level = ?,
    // prev_level_01 = ?, prev_level_02 = ?, prev_id_01 = ?, prev_id_02 = ?, hp = ?, mp = ? WHERE sid = ?;", CONNECTION_ASYNC);
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_UPD_SUMMON);

    uint8_t i = 0;
    stmt->setInt32(i++, 0); // account id
    stmt->setInt32(i++, pMaster->GetInt32Value(UNIT_FIELD_UID));
    stmt->setInt32(i++, pSummon->GetSummonCode());
    stmt->setUInt64(i++, pSummon->GetEXP());
    stmt->setInt32(i++, pSummon->GetJP());
    stmt->setUInt64(i++, 0); // Last decreased exp
    stmt->setString(i++, pSummon->GetName());
    stmt->setInt32(i++, pSummon->m_nTransform);
    stmt->setInt32(i++, pSummon->GetLevel());
    stmt->setInt32(i++, pSummon->GetLevel()); // jlv
    stmt->setInt32(i++, pSummon->GetLevel()); // Max lvl
    stmt->setInt32(i++, pSummon->GetPrevJobLv(0));
    stmt->setInt32(i++, pSummon->GetPrevJobLv(1));
    stmt->setInt32(i++, pSummon->GetPrevJobId(0));
    stmt->setInt32(i++, pSummon->GetPrevJobId(1));
    stmt->setInt32(i++, pSummon->GetHealth());
    stmt->setInt32(i++, pSummon->GetMana());
    stmt->setInt32(i, pSummon->GetUInt32Value(UNIT_FIELD_UID));
    CharacterDatabase.Execute(stmt);
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
    stmt->setInt32(9, pSummon->m_nTransform);                           // transform
    stmt->setInt32(10, pSummon->GetLevel());
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

void Summon::OnAfterReadSummon()
{

}

void Summon::onExpChange()
{
    int level = 1;
    int lvl   = 0;
    int oblv  = 0;
    int jp    = 0;
    switch (m_tSummonBase->form) {
        case 1:
            lvl  = 50;
            oblv = 60;
            break;
        case 2:
            lvl  = 100;
            oblv = 115;
            break;
        case 3:
            lvl  = 170;
            oblv = 170;
            break;
        default:
            return;
    }
    if (lvl > 1) {
        do {
            auto need = sObjectMgr->GetNeedSummonExp(level);
            if (need == 0 || need > GetEXP())
                break;
            ++level;
            if (level > oblv)  /// @todo add max level reached
                ++jp;
        } while (level < oblv);
    }
    if (m_pMaster != nullptr)
        Messages::SendEXPMessage(m_pMaster, this);

    if (level != 0) {
        if (level != GetLevel()) {
            long uid = 0;
            if (m_pItem != nullptr)
                uid = m_pItem->m_Instance.UID;
            int ljp = 0;
            if (level <= GetLevel())
                ljp = 0;
            else
                ljp = jp;

            int levelchange = level - GetLevel();
            SetCurrentJLv(level);
            SetInt32Value(UNIT_FIELD_LEVEL, level);
            if (levelchange <= 0) {
                CalculateStat();
            } else {
                auto old_hp = GetHealth();
                auto old_mp = GetMana();
                SetJP(GetJP() + jp);
                CalculateStat();
                if (GetHealth() != 0) {
                    SetHealth(GetMaxHealth());
                    SetMana(GetMaxMana());
                }
                if (IsInWorld()) {
                    Messages::BroadcastHPMPMessage(this, GetHealth() - old_hp, GetMana() - old_mp, false);
                } else {
                    if (m_pMaster != nullptr) {
                        Messages::SendHPMPMessage(m_pMaster, this, GetHealth() - old_hp, GetMana() - old_mp, false);
                    }
                }
                if (m_pMaster != nullptr)
                    Messages::SendPropertyMessage(m_pMaster, this, "jp", GetJP());
            }
            DB_UpdateSummon(m_pMaster, this);
            if (m_pItem != nullptr && m_pMaster != nullptr)
                Messages::SendItemMessage(m_pMaster, m_pItem);
            if (IsInWorld())
                Messages::BroadcastLevelMsg(this);
            if (m_pMaster != nullptr)
                Messages::SendLevelMessage(m_pMaster, this);
        }
    }
}

bool Summon::DoEvolution()
{
     auto prev_hp = GetHealth();
    auto prev_mp = GetMana();

    if(this->m_tSummonBase->form < 3) {
        // @TODO Ride
        if(false) {
            return false;
        } else {
            auto nTargetCode = m_tSummonBase->evolve_target;
            SetSummonInfo(nTargetCode);
            CalculateStat();
            m_pMaster->Save(false);

            XPacket evoPct(TS_SC_SUMMON_EVOLUTION);
            evoPct << m_pItem->m_nHandle;
            evoPct << GetHandle();
            evoPct.fill(GetName(), 19);
            evoPct << m_tSummonBase->id;
            if(IsInWorld()) {
                sWorld->Broadcast((uint)(GetPositionX() / g_nRegionSize), (uint)(GetPositionY()/g_nRegionSize), GetLayer(), evoPct);
            } else {
                if(m_pMaster != nullptr)
                    m_pMaster->SendPacket(evoPct);
            }

            if(sArRegion->IsVisibleRegion(
                    (uint)(GetPositionX() / g_nRegionSize),
                    (uint)(GetPositionY() / g_nRegionSize),
                    (uint)(m_pMaster->GetPositionX() / g_nRegionSize),
                    (uint)(m_pMaster->GetPositionY() / g_nRegionSize)) == 0) {
                m_pMaster->SendPacket(evoPct);
            }
            Messages::SendStatInfo(m_pMaster, this);
            Messages::SendHPMPMessage(m_pMaster, this, GetHealth() - prev_hp, GetMana() - prev_mp, false);
            Messages::SendLevelMessage(m_pMaster, this);
            Messages::SendEXPMessage(m_pMaster, this);

            if(m_pItem != nullptr)  {
                /*int i = 0;
                for( i = 0; i < m_tSummonBase.form - 1; ++i) {
                    m_pItem->m_Instance.Socket[i+1] = GetPrevJobLv(i);
                }
                m_pItem->m_Instance.Socket[i + 1] = GetLevel();*/
                if(m_pMaster != nullptr)
                    Messages::SendItemMessage(m_pMaster, m_pItem);
            }
            return true;
        }
    }
    return false;
}

bool Summon::TranslateWearPosition(ItemWearType &pos, Item *item, std::vector<int> &ItemList)
{
    return Unit::TranslateWearPosition(pos, item, ItemList);
}

CreatureStat *Summon::GetBaseStat() const
{
    return sObjectMgr->GetStatInfo((uint)m_tSummonBase->stat_id);
}

Summon::~Summon()
{
    if(IsInWorld())
    {
        sWorld->RemoveObjectFromWorld(this);
    }
}
