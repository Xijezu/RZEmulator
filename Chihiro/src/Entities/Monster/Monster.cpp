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

#include "Monster.h"
#include "Messages.h"
#include "XPacket.h"
#include "World.h"
#include "Summon.h"
#include "Player.h"
#include "MemPool.h"
#include "ObjectMgr.h"

Monster::Monster(uint handle, MonsterBase* mb) : Unit(true)
{
    _mainType = MT_NPC;
    _subType = ST_Mob;
    _objType = OBJ_MOVABLE;
    _valuesCount = BATTLE_FIELD_END;

    _InitValues();
    SetUInt32Value(UNIT_FIELD_HANDLE, handle);
    m_Base = mb;
    SetInt32Value(UNIT_FIELD_RACE, m_Base->race);
    SetMaxHealth(mb->hp);
    SetMaxMana(mb->mp);
    SetHealth(GetMaxHealth());
    SetMana(GetMaxMana());
    SetLevel(mb->level);
}

void Monster::EnterPacket(XPacket &pEnterPct, Monster *monster)
{
    Unit::EnterPacket(pEnterPct, monster);
    //pEnterPct << (uint32_t)0;
    Messages::GetEncodedInt(pEnterPct, bits_scramble(monster->m_Base->id));
    // pEnterPct << (uint32_t)0;
    pEnterPct << (uint8_t)0;
}

int Monster::onDamage(Unit *pFrom, ElementalType elementalType, DamageType damageType, int nDamage, bool bCritical)
{
    Player* player{nullptr};
    Summon* summon{nullptr};

    if(GetHealth() - nDamage <= 0)
        nDamage = GetHealth();

    if(nDamage == 0)
        return Unit::onDamage(pFrom, elementalType, damageType, nDamage, bCritical);

    uint ct = sWorld->GetArTime();

    if(pFrom->GetSubType() == ST_Summon) {
        summon = dynamic_cast<Summon*>(pFrom);
        if(summon->GetMaster() != nullptr) {
            player = summon->GetMaster();
        }
    } else {
        player = dynamic_cast<Player*>(pFrom);
    }

    if(m_hFirstAttacker == 0) {
        if(player != nullptr)
            m_hFirstAttacker = player->GetHandle();
        m_nFirstAttackTime = ct;
    }
    this->addDamage(pFrom->GetHandle(), nDamage);
    if(player != nullptr && m_hTamer == player->GetHandle())
        m_nTamedTime += (int)(ct + 30000);
    return Unit::onDamage(pFrom, elementalType, damageType, nDamage, bCritical);
}

void Monster::onDead(Unit *pFrom, bool decreaseEXPOnDead)
{
    Unit::onDead(pFrom, decreaseEXPOnDead);
    //SetFlag(UNIT_FIELD_STATUS, MonsterStatus::MS_Dead);
    SetStatus(MonsterStatus::MS_Dead);

    std::vector<VirtualParty> vPartyContribute{};
    takePriority Priority{};

    // TODO: Do tame

    calcPartyContribute(pFrom, vPartyContribute);
    procEXP(pFrom, vPartyContribute);
    //if(!m_bTamedSuccess) {
    if(true) {
        uint ct = sWorld->GetArTime();
        auto pos = GetCurrentPosition(ct);

        int i = 0;
        for(auto& vp : vPartyContribute) {
            if(m_Base->monster_type >= 31 /* && isDungeonRaidMonster */) {
                Priority.PickupOrder.hPlayer[i]  = 0;
                Priority.PickupOrder.nPartyID[i] = 0;
            } else if(vp.hPlayer != 0) {
                Priority.PickupOrder.hPlayer[i] = vp.hPlayer;
                Priority.PickupOrder.nPartyID[i] = 0;
            } else {
                Priority.PickupOrder.hPlayer[i] = 0;
                Priority.PickupOrder.nPartyID[i] = vp.nPartyID;
            }

            i++;
            if(i >= 3)
                break;
        }

        int cl;
        if(!vPartyContribute.empty())
            cl = vPartyContribute[0].nLevel;
        else
            cl = 0;

        auto fDropRatePenalty = 1.0f;
        cl -= GetLevel();
        if(cl >= 5) {
            fDropRatePenalty     = 1.0f - (float) pow(cl - 3, 2.0f) * 0.02f;
            if (fDropRatePenalty < 0.0f)
                fDropRatePenalty = 0.0f;
        }

        float fChaosDropRateBonus = 1.0f;
        float fItemDropRateBonus = 1.0f;
        float fGoldDropRateBonus = 1.0f;

        auto player = dynamic_cast<Player*>(pFrom);
        //procDropChaos(pFrom, Priority, vPartyContribute, fDropRatePenalty, fChaosDropRateBonus);
        //if(pFrom->GetSubType() == ST_Player && player->GetInt32Value(UNIT_FIELD_CHAOS) < 1)
            //sWorld->addChaos(this, player, 1.0f);

        if(m_Base->monster_type < 31 /* || !IsDungeonRaidMonster*/) {
            //procDropGold(pos, pFrom, Priority, vPartyContribute, fDropRatePenalty, fGoldDropRateBonus);
            procDropItem(pos, pFrom, Priority, vPartyContribute, fDropRatePenalty);
        }

        // TODO: OnDeath script
    }
}

void Monster::calcPartyContribute(Unit *pKiller, std::vector<VirtualParty> &vPartyContribute)
{
    if(m_vDamageList.empty())
        return;

    uint t = sWorld->GetArTime();
    uint hKiller = 0;
    float fMaxDamageAdd = 0.1f;
    int nLastAttackPartyID = 0;
    Player* player{nullptr};

    if(m_nFirstAttackTime + 6000 < t) {
        m_hFirstAttacker = 0;
        fMaxDamageAdd = 0.4f;
    }

    if(pKiller->GetSubType() == ST_Summon) {
        player = dynamic_cast<Summon*>(pKiller)->GetMaster();
    } else if(pKiller->GetSubType() == ST_Player) {
        player = dynamic_cast<Player *>(pKiller);
        if (player != nullptr) {
            hKiller            = player->GetHandle();
            nLastAttackPartyID = player->GetInt32Value(UNIT_FIELD_PARTY_ID);
        }
    }

    for(auto& dt : m_vDamageList) {
        /////// HACK
        auto p = dynamic_cast<Player*>(sMemoryPool->getPtrFromId(dt.uid));
        if(p != nullptr) {
            VirtualParty vp{dt.uid, dt.nDamage, (int)p->GetLevel()};
            vp.fContribute = 1.0f;
            vPartyContribute.emplace_back(vp);
        }
    }


    ///// TODO: Do whole file
}

DamageTag* Monster::addDamage(uint handle, int nDamage)
{
    uint ct = sWorld->GetArTime();
    auto tag = getDamageTag(handle, ct);
    if(tag == nullptr) {
        m_vDamageList.emplace_back(DamageTag{handle, ct, nDamage});
        tag = &m_vDamageList.back();
    }
    m_nTotalDamage += nDamage;
    tag->nDamage += nDamage;
    return tag;
}

DamageTag* Monster::getDamageTag(uint handle, uint t)
{
    if(m_vDamageList.empty())
        return nullptr;
    for(unsigned long i = m_vDamageList.size() - 1; i >= 0; --i)
    {
        DamageTag* dt = &m_vDamageList[i];
        if (dt->uid  == handle)
        {
            if(t != 0)
                dt->nTime = t;
            return dt;
        }
        if (dt->nTime + 30000 < t)
        {
            m_nTotalDamage -= dt->nDamage;
            m_vDamageList.erase(m_vDamageList.begin() + i);
        }
    }
    return nullptr;
}

void Monster::procEXP(Unit *pKiller, std::vector<VirtualParty> &vPartyContribute)
{
    if(m_vDamageList.empty())
        return;

    float fSharedJP = 0.0f;
    float fSharedEXP = 0.0f;

    for(auto& vp : vPartyContribute) {
        fSharedEXP = vp.fContribute * m_Base->exp[0];
        fSharedJP = vp.fContribute * m_Base->jp[0];
        if(fSharedEXP < 1)
            fSharedEXP = 1;

        if(vp.bTamer || (vp.hPlayer != 0 && vp.hPlayer == m_hTamer)) {
            fSharedEXP = fSharedEXP * m_Base->taming_exp_mod;
            fSharedJP = fSharedJP * m_Base->taming_exp_mod;
        }

        if(vp.hPlayer != 0) {
            auto player2 = dynamic_cast<Player *>(sMemoryPool->getPlayerPtrFromId(vp.hPlayer));
            if (player2 != nullptr)
                sWorld->addEXP(this, player2, fSharedEXP, fSharedJP);
        } else {
            //sWorld->addEXP(this, vp.nPartyID, (int)fSharedEXP, (int)fSharedJP);
        }
    }
}

void Monster::Update(uint diff)
{
    uint ct = sWorld->GetArTime();
    MonsterStatus  ms = GetStatus();
    if(ms != MonsterStatus::MS_Normal) {
        if((int)ms > 0) {
            if((int)ms <= 3) {
                if(m_nLastUpdatedTime + 50 < ct)
                    OnUpdate();

                if(true/*IsActable*/) {
                    if(GetHealth() == 0)
                        return;
                }
            } else {
                if(ms == MonsterStatus::MS_Dead) {
                    processDead(ct);
                    return;
                }
            }
        }
    } else {
        if(m_nLastUpdatedTime + 3000 < ct || HasFlag(UNIT_FIELD_STATUS, StatusFlags::MovePending)) {
            OnUpdate();

            if(GetHealth() == 0)
                return;
            //if(GetHealth() == GetMaxHealth())
                //clearHateList();
        }

    }

    if(GetHealth() != 0) {
        if(HasFlag(UNIT_FIELD_STATUS, StatusFlags::MovePending)) {
            if(IsInWorld())
                processPendingMove();
        }
        if(m_nTamedTime + 30000 < ct) {
            m_hTamer = 0;
            m_nTamedTime = -1;
        }

        if(bIsMoving && IsInWorld()) {
            //processWalk
        }
    }

    Unit::Update(diff);
}

void Monster::processDead(uint t)
{
    if(m_pDeleteHandler != nullptr) {
        m_pDeleteHandler->onMonsterDelete(this);
    }

    if(GetUInt32Value(UNIT_FIELD_DEAD_TIME) + 1200 < t) {
        if(IsInWorld())
            sWorld->RemoveObjectFromWorld(this);
        sMemoryPool->DeleteMonster(GetHandle(), true);
    }
}

void Monster::SetStatus(MonsterStatus status)
{
    if(m_nStatus != MonsterStatus::MS_Dead) {
        //if((int)status != m_nStatus && (int)status != 4 && (int)status != 0 && m_nStatus == 0)
            //ResetTriggerCondition();
        if(m_nStatus != status) {
            m_nStatus = status;
            //Messages::BroadcastStatusMessage(this);
        }
    }
}

void Monster::procDropItem(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty>& vPartyContribute, float fDropRatePenalty)
{
    long item_count;
    for(int i = 0; i < 10; ++i) {
        if(m_Base->drop_item_id[i] != 0 /*&& checkDrop */) {
            item_count = irand(m_Base->drop_min_count[i], m_Base->drop_max_count[i]);
            if(item_count < m_Base->drop_min_count[i]) {
                MX_LOG_WARN("entities.monster", "Monster::procDropItem: Min/Max Count error!");
            } else {
                int level = irand(m_Base->drop_min_level[i], m_Base->drop_max_level[i]);
                int code = m_Base->drop_item_id[i];
                if(code >= 0)
                    dropItem(pos, pKiller, pPriority, vPartyContribute, code, item_count, level, false, -1);
                else
                    dropItemGroup(pos, pKiller, pPriority, vPartyContribute, code, item_count, level, -1);
            }
        }
    }
}

void Monster::dropItem(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, int code, long count, int level, bool bIsEventItem, int nFlagIndex)
{
    if(count == 0) {
        MX_LOG_ERROR("entities.monster", "Monster::dropItem: count was 0. (x: %u, y: %u, code: %u, Killer: %s", pos.GetPositionX(), pos.GetPositionY(), code, pKiller->GetName());
        return;
    }

    Unit* cr{nullptr};
    Player* player{nullptr};

    for(auto& ht : m_vHateList) {
        if(ht.uid != 0) {
            cr = dynamic_cast<Unit*>(sMemoryPool->getPtrFromId(ht.uid));

            if(cr != nullptr) {
                if(cr->GetSubType() == ST_Player)
                    player = dynamic_cast<Player*>(cr);

                if(cr->GetSubType() == ST_Summon)
                    player = dynamic_cast<Summon*>(cr)->GetMaster();

                if(player != nullptr && player->IsInWorld())
                    break;
            }
        }
    }

    if(player == nullptr) {
        if(pKiller == nullptr)
            return;
        if(pKiller->GetSubType() == ST_Player)
            player = dynamic_cast<Player*>(pKiller);
        if(pKiller->GetSubType() == ST_Summon)
            player = dynamic_cast<Summon*>(pKiller)->GetMaster();
    }

    if(player != nullptr && player->IsInWorld()) {
        auto ni = Item::AllocItem(0, code, count, GenerateCode::ByMonster, level, 0, 0, 0, 0, 0, 0, 0);
        if(ni == nullptr)
            return;
        ni->SetPickupOrder(pPriority.PickupOrder);
        ni->SetCurrentXY(pos.GetPositionX(), pos.GetPositionY());
        ni->SetLayer(GetLayer());
        //ni->AddNoise(rand32(), rand32(), 18);

        if((uint)nFlagIndex <= 0x1F)
            ni->m_Instance.Flag |= (uint)(1 << (nFlagIndex & 0x1F));
        if(bIsEventItem) {
            ni->m_Instance.Flag |= 0x10;
        }
        sWorld->MonsterDropItemToWorld(this, ni);
    }
}

void Monster::dropItemGroup(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, int nDropGroupID, long count, int level, int nFlagIndex)
{
    std::map<int, long> mapDropItem{};
    long nItemCount;
    int nItemID;
    for(int i = 0; i < count; ++i) {
        nItemID = nDropGroupID;
        nItemCount = 1;

        do
            sObjectMgr->SelectItemIDFromDropGroup(nItemID, nItemID, nItemCount);
        while(nItemID < 0);
        if(nItemID > 0) {
            if(mapDropItem.count(nItemID) > 0) {
                mapDropItem[nItemID] = mapDropItem[nItemID] + nItemCount;
            } else {
                mapDropItem.emplace(nItemID, nItemCount);
            }
        }
    }

    for(auto& kvp : mapDropItem) {
        dropItem(pos, pKiller, pPriority, vPartyContribute, kvp.first, kvp.second, level, false, nFlagIndex);
    }
}
