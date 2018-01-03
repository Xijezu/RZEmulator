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
    SetFlag(UNIT_FIELD_STATUS, MonsterStatus::MS_Dead);

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
            //procDropItem(pos, pFrom, Priority, vPartyContribute, fDropRatePenalty, fItemDropRateBonus);
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

    vPartyContribute.resize(m_vDamageList.size());
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
