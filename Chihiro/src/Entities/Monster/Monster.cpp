/*
 *  Copyright (C) 2017-2020 NGemity <https://ngemity.org/>
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
#include "EncodingScrambled.h"
#include "GameContent.h"
#include "GameRule.h"
#include "GroupManager.h"
#include "Log.h"
#include "MemPool.h"
#include "Messages.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "RegionContainer.h"
#include "Skill.h"
#include "Summon.h"
#include "World.h"
#include "XPacket.h"
#include <algorithm>

Monster::Monster(uint32_t handle, MonsterBase *mb) : Unit(true)
{
    _mainType = MT_NPC;
    _subType = ST_Mob;
    _objType = OBJ_MOVABLE;
    _valuesCount = BATTLE_FIELD_END;

    _InitValues();
    _InitTimerFieldsAndStatus();
    SetUInt32Value(UNIT_FIELD_HANDLE, handle);
    SetInt32Value(UNIT_FIELD_UID, mb->id);
    m_Base = mb;
    SetInt32Value(UNIT_FIELD_RACE, m_Base->race);
    m_nStatus = STATUS_NORMAL;
    SetLevel((uint8_t)mb->level);
    CalculateStat();
    SetHealth(GetMaxHealth());
    SetMana(GetMaxMana());
    SetOrientation((float)(rand32() / 100));

    m_bNearClient = false;
    m_nLastEnemyDistance = 0.0f;
    m_nLastTrackTime = 0;
    m_bComeBackHome = false;
    m_nLastHateUpdateTime = 0;
    m_bNeedToFindEnemy = false;
    m_hFirstAttacker = 0;
    m_nFirstAttackTime = 0;
    m_nTotalDamage = 0;
    m_nMaxHate = 0;

    m_nTamingSkillLevel = 0;
    m_hTamer = 0;
    m_nTamedTime = 0;
    m_bTamedSuccess = false;
    m_bIsWandering = false;
    m_pWayPointInfo = nullptr;
    m_nWayPointIdx = 0;
}

void Monster::EnterPacket(XPacket &pEnterPct, Monster *monster, Player *pPlayer)
{
    Unit::EnterPacket(pEnterPct, monster, pPlayer);
    Messages::GetEncodedInt(pEnterPct, EncodingScrambled::Scramble(static_cast<uint32_t>(monster->m_Base->id)));
    pEnterPct << (uint8_t)0;
}

int32_t Monster::onDamage(Unit *pFrom, ElementalType elementalType, DamageType damageType, int32_t nDamage, bool bCritical)
{
    Player *player{nullptr};
    Summon *summon{nullptr};

    if (GetHealth() - nDamage <= 0)
        nDamage = GetHealth();

    if (nDamage == 0)
        return Unit::onDamage(pFrom, elementalType, damageType, nDamage, bCritical);

    uint32_t ct = sWorld.GetArTime();

    if (pFrom->IsSummon())
    {
        summon = dynamic_cast<Summon *>(pFrom);
        if (summon->GetMaster() != nullptr)
        {
            player = summon->GetMaster();
        }
    }
    else
    {
        player = dynamic_cast<Player *>(pFrom);
    }

    if (m_hFirstAttacker == 0)
    {
        if (player != nullptr)
            m_hFirstAttacker = player->GetHandle();
        m_nFirstAttackTime = ct;
    }
    this->addDamage(pFrom->GetHandle(), nDamage);
    if (player != nullptr && m_hTamer == player->GetHandle())
        m_nTamedTime += (int32_t)(ct + 30000);
    return Unit::onDamage(pFrom, elementalType, damageType, nDamage, bCritical);
}

void Monster::onDead(Unit *pKiller, bool decreaseEXPOnDead)
{
    Unit::onDead(pKiller, decreaseEXPOnDead);
    SetStatus(STATUS_DEAD);

    std::vector<VirtualParty> vPartyContribute{};
    takePriority Priority{};
    m_bTamedSuccess = false;

    if (GetTamer() != 0)
    {
        auto player = sMemoryPool.GetObjectInWorld<Player>(m_hTamer);
        if (player != nullptr)
            m_bTamedSuccess = sWorld.ProcTame(this);
    }

    calcPartyContribute(pKiller, vPartyContribute);
    if (m_Base->exp[0] > 0)
        procEXP(pKiller, vPartyContribute);

    if (!m_bTamedSuccess)
    {
        std::sort(vPartyContribute.begin(), vPartyContribute.end(), [](const auto &a, const auto &b) -> bool { return a.fContribute > b.fContribute; });
        auto pos = GetCurrentPosition(sWorld.GetArTime());

        takePriority Priority{};

        int32_t nCount{0};

        for (auto &vp : vPartyContribute)
        {
            if (m_Base->monster_type >= 31 /* && isDungeonRaidMonster*/)
            {
                Priority.PickupOrder.hPlayer[nCount] = 0;
                Priority.PickupOrder.nPartyID[nCount] = 0;
            }
            else if (vp.hPlayer != 0)
            {
                Priority.PickupOrder.hPlayer[nCount] = vp.hPlayer;
                Priority.PickupOrder.nPartyID[nCount] = 0;
            }
            else
            {
                Priority.PickupOrder.hPlayer[nCount] = 0;
                Priority.PickupOrder.nPartyID[nCount] = vp.nPartyID;
            }

            if (++nCount >= 3)
                break;
        }

        auto firstContrib = vPartyContribute.front();
        int32_t nMaxPriorityLevel = (vPartyContribute.empty()) ? 0 : firstContrib.nLevel;
        float fMaxContribute = (vPartyContribute.empty()) ? 0.0f : firstContrib.fContribute;

        float fDropRatePenalty{1.0f};
        int32_t nLevelDiff = nMaxPriorityLevel - GetLevel();

        if (nLevelDiff >= 10)
            fDropRatePenalty = std::max(1 - (nLevelDiff - 10) * 0.2f, 0.0f);

        float fChaosDropRateBonus{1.0f};
        float fItemDropRateBonus{1.0f};
        float fGoldDropRateBonus{1.0f};

        procDropChaos(pKiller, vPartyContribute, fDropRatePenalty);

        auto pPlayer = pKiller->IsPlayer() ? pKiller->As<Player>() : nullptr;
        if (pKiller->IsPlayer() && pPlayer->GetChaos() < 1 && pPlayer->GetQuestProgress(1032) == 1)
            sWorld.addChaos(this, pPlayer, 1.0f);

        if (m_Base->monster_type < 31 /* || !IsDungeonRaidMonster*/)
        {
            procDropGold(pos, pKiller, Priority, vPartyContribute, fDropRatePenalty);
            procDropItem(pos, pKiller, Priority, vPartyContribute, fDropRatePenalty);
        }
    }

    for (const auto &pHate : m_vHateList)
    {
        auto pEnemy = sMemoryPool.GetObjectInWorld<Unit>(pHate.uid);
        if (pEnemy != nullptr)
            pEnemy->RemoveFromEnemyList(GetHandle());
    }
}

void Monster::calcPartyContribute(Unit *pKiller, std::vector<VirtualParty> &vPartyContribute)
{
    if (m_vDamageList.empty())
        return;

    uint32_t t = sWorld.GetArTime();
    uint32_t hKiller{0};
    std::map<int32_t, PARTY_DAMAGE> mapPartyDamageContribute{};
    int32_t nTamerPartyID{0};
    int32_t nFirstAttackPartyID{0};
    int32_t nLastAttackPartyID{0};
    float fMaxDamageAdd{0.1f};

    if (m_nFirstAttackTime + GameRule::MAX_FIRST_ATTACK_BONUS_TIME < t)
    {
        fMaxDamageAdd = 0.4f;
        m_hFirstAttacker = 0;
    }

    if (pKiller->IsSummon())
    {
        auto pPlayer = pKiller->As<Summon>()->GetMaster();
        if (pPlayer != nullptr)
        {
            hKiller = pPlayer->GetHandle();
            nLastAttackPartyID = pPlayer->GetPartyID();
        }
    }
    else if (pKiller->IsPlayer())
    {
        hKiller = pKiller->GetHandle();
        nLastAttackPartyID = pKiller->As<Player>()->GetPartyID();
    }

    for (const auto &it : m_vDamageList)
    {
        auto pUnit = sMemoryPool.GetObjectInWorld<Unit>(it.uid);
        if (pUnit == nullptr)
            continue;

        auto pPlayer = (pUnit->IsPlayer()) ? pUnit->As<Player>() : nullptr;
        auto pSummon = (pUnit->IsSummon()) ? pUnit->As<Summon>() : nullptr;

        if (pPlayer == nullptr && pSummon == nullptr)
            continue;

        if (pPlayer == nullptr)
            pPlayer = pSummon->GetMaster();

        if (pPlayer->IsInParty())
        {
            if (GetTamer() == pPlayer->GetHandle())
                nTamerPartyID = pPlayer->GetPartyID();

            if (auto itMap = mapPartyDamageContribute.find(pPlayer->GetPartyID()); itMap == mapPartyDamageContribute.end())
            {
                mapPartyDamageContribute[pPlayer->GetPartyID()] = PARTY_DAMAGE(it.nDamage, pPlayer->GetLevel());
            }
            else
            {
                itMap->second.nDamage += it.nDamage;
                if (itMap->second.nLevel < pPlayer->GetLevel())
                    itMap->second.nLevel = pPlayer->GetLevel();
            }

            if (pPlayer->GetHandle() == m_hFirstAttacker)
                nFirstAttackPartyID = pPlayer->GetPartyID();
        }
        else
        {
            std::vector<VirtualParty>::iterator itVector;
            for (itVector = vPartyContribute.begin(); itVector != vPartyContribute.end(); ++itVector)
            {
                if ((*itVector).hPlayer == pPlayer->GetHandle())
                {
                    (*itVector).nDamage += it.nDamage;
                    break;
                }
            }

            if (itVector == vPartyContribute.end())
                vPartyContribute.emplace_back(VirtualParty(pPlayer->GetHandle(), it.nDamage, pPlayer->GetLevel()));
        }
    }

    for (const auto &itMap : mapPartyDamageContribute)
        vPartyContribute.emplace_back(VirtualParty(itMap.first, itMap.second.nDamage, itMap.second.nLevel));

    std::sort(vPartyContribute.begin(), vPartyContribute.end(), [](const VirtualParty &a, const VirtualParty &b) -> bool { return a.nDamage > b.nDamage; });

    for (auto itVector = vPartyContribute.begin(); itVector != vPartyContribute.end(); ++itVector)
    {
        float fContribute{0.0f};
        if (nTamerPartyID != 0 && nTamerPartyID == (*itVector).nPartyID)
            (*itVector).bTamer = true;

        if (m_nTotalDamage != 0)
            fContribute = static_cast<float>((*itVector).nDamage / m_nTotalDamage) * 0.5f;

        if (itVector == vPartyContribute.begin())
            fContribute += fMaxDamageAdd;

        if (nFirstAttackPartyID != 0)
        {
            if ((*itVector).nPartyID == nFirstAttackPartyID)
                fContribute += 0.3f;
        }
        else if (m_hFirstAttacker != 0 && (*itVector).hPlayer == m_hFirstAttacker)
        {
            fContribute += 0.3f;
        }

        if (nLastAttackPartyID != 0)
        {
            if ((*itVector).nPartyID == nLastAttackPartyID)
                fContribute += 0.1f;
        }
        else
        {
            if ((*itVector).hPlayer == hKiller)
                fContribute += 0.1f;
        }

        (*itVector).fContribute = fContribute;
    }
}

DamageTag *Monster::addDamage(uint32_t handle, int32_t nDamage)
{
    uint32_t ct = sWorld.GetArTime();
    auto tag = getDamageTag(handle, ct);
    if (tag == nullptr)
    {
        m_vDamageList.emplace_back(DamageTag{handle, ct, nDamage});
        tag = &m_vDamageList.back();
    }
    m_nTotalDamage += nDamage;
    tag->nDamage += nDamage;
    return tag;
}

DamageTag *Monster::getDamageTag(uint32_t handle, uint32_t t)
{
    if (m_vDamageList.empty())
        return nullptr;
    for (uint32_t i = 0; i < m_vDamageList.size(); i++)
    {
        DamageTag *dt = &m_vDamageList[i];
        if (dt->uid == handle)
        {
            if (t != 0)
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
    if (m_vDamageList.empty())
        return;

    int32_t nEXP = m_Base->exp[0];
    int32_t nJP = m_Base->jp[0];

    for (auto &vp : vPartyContribute)
    {
        int32_t nSharedEXP = nEXP * vp.fContribute;
        float fSharedJP = nJP * vp.fContribute;
        if (nSharedEXP < 1)
            nSharedEXP = 1;

        if (vp.hPlayer != 0)
        {
            if (vp.hPlayer == GetTamer())
            {
                nSharedEXP *= GetTameExpAdjust();
                fSharedJP *= GetTameExpAdjust();
            }
        }
        else
        {
            if (vp.bTamer)
            {
                nSharedEXP *= GetTameExpAdjust();
                fSharedJP *= GetTameExpAdjust();
            }
        }

        if (vp.hPlayer != 0)
        {
            auto pPlayer = sMemoryPool.GetObjectInWorld<Player>(vp.hPlayer);
            if (pPlayer != nullptr)
                sWorld.addEXP(this, pPlayer, nSharedEXP, fSharedJP);
        }
        else
        {
            sWorld.addEXP(this, vp.nPartyID, nSharedEXP, fSharedJP);
        }
    }
}

void Monster::Update(uint32_t diff)
{
    if (bForceKill)
        ForceKill(pFCClient);

    if (!m_bNearClient)
        return;

    uint32_t ct = sWorld.GetArTime();
    MONSTER_STATUS ms = GetStatus();

    if (ms != STATUS_NORMAL)
    {
        if ((int32_t)ms > 0)
        {
            if ((int32_t)ms <= 3)
            {
                if (GetUInt32Value(UNIT_LAST_UPDATE_TIME) + 50 < ct)
                    OnUpdate();

                if (IsActable())
                {
                    if (GetHealth() == 0)
                        return;

                    AI_processAttack(ct);
                }
            }
            else
            {
                if (ms == STATUS_DEAD)
                {
                    processDead(ct);
                    return;
                }
            }
        }
    }
    else
    {
        if (GetUInt32Value(UNIT_LAST_UPDATE_TIME) + 3000 < ct || HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_PENDED))
        {
            OnUpdate();

            if (GetHealth() == 0)
                return;
            //if(GetHealth() == GetMaxHealth())
            //clearHateList();
        }
        if (IsInWorld() && IsActable() && (/* m_bIsDungeonRaidMonster &&*/ !m_bComeBackHome))
            processFirstAttack(ct);
        if (IsInWorld() && IsActable() && IsMovable() && !m_bComeBackHome)
            processMove(ct);
    }

    if (GetHealth() != 0)
    {
        if (HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_PENDED))
        {
            if (IsInWorld())
                processPendingMove();
        }
        if (m_nLastHateUpdateTime < ct + 6000)
        {
            m_nLastHateUpdateTime = ct;
            //updateHate();
        }
        if (m_nTamedTime + 30000 < static_cast<int32_t>(ct))
        {
            m_hTamer = 0;
            m_nTamedTime = -1;
        }

        if (bIsMoving && IsInWorld())
        {
            if (IsMovable())
                processWalk(ct);
        }
    }
    Unit::Update(diff);
}

void Monster::processDead(uint32_t t)
{
    if (m_pDeleteHandler != nullptr)
    {
        m_pDeleteHandler->onMonsterDelete(this);
    }

    if (GetUInt32Value(UNIT_FIELD_DEAD_TIME) + 1200 < t)
    {
        if (IsInWorld())
            sWorld.RemoveObjectFromWorld(this);
        //sMemoryPool.RemoveObject(this, true);
        DeleteThis();
    }
}

void Monster::SetStatus(MONSTER_STATUS status)
{
    if (m_nStatus != STATUS_DEAD)
    {
        //if((int32_t)status != m_nStatus && (int32_t)status != 4 && (int32_t)status != 0 && m_nStatus == 0)
        //ResetTriggerCondition();
        if (m_nStatus != status)
        {
            m_nStatus = status;
            Messages::BroadcastStatusMessage(this);
        }
    }
}

void Monster::procDropItem(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, float fDropRatePenalty)
{
    int64_t item_count;
    for (int32_t i = 0; i < 10; ++i)
    {
        if (m_Base->drop_item_id[i] != 0 && sWorld.checkDrop(pKiller, m_Base->drop_item_id[i], m_Base->drop_percentage[i], fDropRatePenalty, 1))
        {
            item_count = irand(m_Base->drop_min_count[i], m_Base->drop_max_count[i]);
            if (item_count < m_Base->drop_min_count[i])
            {
                NG_LOG_WARN("entities.monster", "Monster::procDropItem: Min/Max Count error!");
            }
            else
            {
                int32_t level = irand(m_Base->drop_min_level[i], m_Base->drop_max_level[i]);
                int32_t code = m_Base->drop_item_id[i];
                if (code >= 0)
                    dropItem(pos, pKiller, pPriority, vPartyContribute, code, item_count, level, false, -1);
                else
                    dropItemGroup(pos, pKiller, pPriority, vPartyContribute, code, item_count, level, -1);
            }
        }
    }
    procQuest(pos, pKiller, pPriority, vPartyContribute);
}

void Monster::dropItem(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, int32_t code, long count, int32_t level, bool bIsEventItem, int32_t nFlagIndex)
{
    if (count == 0)
    {
        NG_LOG_ERROR("entities.monster", "Monster::dropItem: count was 0. (x: %f, y: %f, code: %u, Killer: %s", pos.GetPositionX(), pos.GetPositionY(), code, pKiller->GetName());
        return;
    }

    WorldObject *cr{nullptr};
    Player *player{nullptr};

    for (auto &ht : m_vHateList)
    {
        if (ht.uid != 0)
        {
            cr = sMemoryPool.GetObjectInWorld<WorldObject>(ht.uid);

            if (cr != nullptr)
            {
                if (cr->IsPlayer())
                    player = dynamic_cast<Player *>(cr);

                if (cr->IsSummon())
                    player = dynamic_cast<Summon *>(cr)->GetMaster();

                if (player != nullptr && player->IsInWorld())
                    break;
            }
        }
    }

    if (player == nullptr)
    {
        if (pKiller == nullptr)
            return;
        if (pKiller->IsPlayer())
            player = dynamic_cast<Player *>(pKiller);
        if (pKiller->IsSummon())
            player = dynamic_cast<Summon *>(pKiller)->GetMaster();
    }

    if (player != nullptr && player->IsInWorld())
    {
        auto ni = Item::AllocItem(0, code, count, BY_MONSTER, level, -1, -1, 0, 0, 0, 0, 0);
        if (ni == nullptr)
            return;
        ni->SetPickupOrder(pPriority.PickupOrder);
        ni->SetCurrentXY(pos.GetPositionX(), pos.GetPositionY());
        ni->SetLayer(GetLayer());
        ni->AddNoise(rand32(), rand32(), 9);

        if ((uint32_t)nFlagIndex <= 0x1F)
            ni->m_Instance.Flag |= (uint32_t)(1 << (nFlagIndex & 0x1F));
        if (bIsEventItem)
        {
            ni->m_Instance.Flag |= 0x10;
        }
        sWorld.MonsterDropItemToWorld(this, ni);
    }
}

void Monster::dropItemGroup(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, int32_t nDropGroupID, long count, int32_t level, int32_t nFlagIndex)
{
    std::map<int32_t, int64_t> mapDropItem{};

    for (int32_t i = 0; i < static_cast<int32_t>(count); ++i)
    {
        int32_t nItemID = nDropGroupID;
        int64_t nItemCount = 1;

        do
            GameContent::SelectItemIDFromDropGroup(nItemID, nItemID, nItemCount);
        while (nItemID < 0);

        if (nItemID > 0)
        {
            auto it = mapDropItem.find(nItemID);
            if (it == mapDropItem.end())
                mapDropItem.emplace(nItemID, nItemCount);
            else
                (*it).second += nItemCount;
        }
    }

    for (auto &drop : mapDropItem)
        dropItem(pos, pKiller, pPriority, vPartyContribute, drop.first, drop.second, level, false, nFlagIndex);
}

void Monster::ForceKill(Player *byPlayer)
{
    auto dmg = onDamage(byPlayer, TYPE_NONE, DT_NORMAL_PHYSICAL_DAMAGE, GetHealth(), false);
    damage(byPlayer, dmg, false);
    Messages::SendHPMPMessage(byPlayer, this, 0, 0, true);
}

void Monster::procDropGold(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, float fDropRatePenalty)
{
    int64_t gold;
    int32_t tax;
    int32_t nGuildID;
    Player *player{nullptr};
    fDropRatePenalty = GameRule::GetGoldDropRate() * fDropRatePenalty;
    if ((uint32_t)rand32() % 100 >= m_Base->gold_drop_percentage * fDropRatePenalty)
        return;

    gold = (int64_t)irand(m_Base->gold_min[0], m_Base->gold_max[0]);
    if (gold < 0)
        gold = 1;

    if (gold > 1000000)
        gold = 1000000;
    auto gi = sMemoryPool.AllocGold(gold, BY_MONSTER);
    gi->SetCurrentXY(pos.GetPositionX(), pos.GetPositionY());
    gi->SetLayer(GetLayer());

    gi->AddNoise(rand32(), rand32(), 9);
    gi->SetPickupOrder(pPriority.PickupOrder);
    sWorld.MonsterDropItemToWorld(this, gi);
}

void Monster::onApplyAttributeAdjustment()
{
    if (m_Base == nullptr)
        return;

    SetMaxHealth(GetMaxHealth() + m_Base->hp);
    SetMaxMana(GetMaxMana() + m_Base->mp);
    m_Attribute.nAttackPointRight += m_Base->attacK_point;
    m_Attribute.nMagicPoint += m_Base->magic_point;
    m_Attribute.nDefence += m_Base->defence;
    m_Attribute.nMagicDefence += m_Base->magic_defence;
    m_Attribute.nAttackSpeedRight += m_Base->attack_speed;
    m_Attribute.nAttackSpeedLeft += m_Base->attack_speed;
    m_Attribute.nCastingSpeed += m_Base->magic_speed;
    m_Attribute.nAccuracyRight += m_Base->accuracy;
    m_Attribute.nAccuracyLeft += m_Base->accuracy;
    m_Attribute.nMagicAccuracy += m_Base->magic_accuracy;
    m_Attribute.nAvoid += m_Base->avoid;
    m_Attribute.nMagicAvoid += m_Base->magic_avoid;
}

void Monster::OnUpdate()
{
    if (m_bNeedToFindEnemy)
    {
        m_bNeedToFindEnemy = false;
        findNextEnemy();
    }
    Unit::OnUpdate();
}

void Monster::findNextEnemy()
{
    int32_t nMaxHate{-1};
    uint32_t target{0};

    for (auto &ht : m_vHateList)
    {
        if (!ht.bIsActive)
            continue;

        int32_t nHate = ht.nHate;
        for (const auto &mit : m_vHateModifierByState)
        {
            if (mit.uid == ht.uid)
            {
                nHate += mit.nHate;
                break;
            }
        }

        if (nHate > nMaxHate)
        {
            auto pTarget = sMemoryPool.GetObjectInWorld<Unit>(ht.uid);
            if (pTarget != nullptr && sRegion.IsVisibleRegion(pTarget, this) && IsVisible(pTarget))
            {
                nMaxHate = nHate;
                target = ht.uid;
            }
        }
    }

    if (nMaxHate == -1)
    {
        comeBackHome(false);
    }
    else
    {
        m_nMaxHate = nMaxHate;
        SetUInt32Value(BATTLE_FIELD_TARGET_HANDLE, target);
        SetFlag(UNIT_FIELD_STATUS, STATUS_FIRST_ATTACK);
    }
}

void Monster::comeBackHome(bool bInvincible)
{
    std::vector<Position> pl{};
    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_FEARED))
    {
        SetStatus(STATUS_NORMAL);
        m_nLastHateUpdateTime = sWorld.GetArTime();
        SetUInt32Value(BATTLE_FIELD_TARGET_HANDLE, 0);
        m_nMaxHate = 0;
        m_bComeBackHome = true;

        pl.emplace_back(m_pRespawn);
        if (bInvincible)
            SetFlag(UNIT_FIELD_STATUS, STATUS_INVINCIBLE);

        SetPendingMove(pl, (uint8_t)(2 * m_Attribute.nMoveSpeed / 7));
    }
}

bool Monster::StartAttack(uint32_t target, bool bNeedFastReaction)
{
    bool result{false};
    if (Unit::StartAttack(target, bNeedFastReaction))
    {
        SetStatus(STATUS_ATTACK);
        result = true;
    }
    return result;
}

void Monster::AI_processAttack(uint32_t t)
{
    if (m_castingSkill != nullptr)
    {
        m_castingSkill->ProcSkill();
        return;
    }

    auto target = sMemoryPool.GetObjectInWorld<Unit>(GetTargetHandle());
    if (target == nullptr || target->GetHandle() != GetTargetHandle() || target->GetHealth() == 0 || !target->IsInWorld())
    {
        removeFromHateList(GetTargetHandle());
        findNextEnemy();
    }
    else
    {
        if (HasFlag(UNIT_FIELD_STATUS, STATUS_FIRST_ATTACK))
        {
            target->OnUpdate();
            RemoveFlag(UNIT_FIELD_STATUS, STATUS_FIRST_ATTACK);
        }
        if (target->GetHealth() != 0)
        {
            AI_processAttack(target, t);
            return;
        }
        removeFromHateList(GetTargetHandle());
        findNextEnemy();
    }
}

void Monster::AI_processAttack(Unit *pEnemy, uint32_t t)
{
    if (!IsInWorld() || pEnemy == nullptr)
        return;

    if ((IsAutoTrap() || (IsAgent())) && pEnemy->GetEnemyHandle() != GetHandle())
    {
        comeBackHome(false);
        return;
    }

    if (!sRegion.IsVisibleRegion(this, pEnemy) || pEnemy->GetLayer() != GetLayer())
    {
        comeBackHome(false);
        return;
    }

    Position enemyPosition = pEnemy->GetCurrentPosition(t);
    Position myPosition = GetCurrentPosition(t);
    auto enemy_distance = myPosition.GetExactDist2d(&enemyPosition);

    // Trigger condition
    if (GetNextAttackableTime() <= t)
    {
        // @todo
    }

    auto ht = getHateTag(pEnemy->GetHandle(), 0);
    if (ht != nullptr && ht->nTime + 2500 < t)
        m_bNeedToFindEnemy = true;

    if (/*!IsDungeonRaidMonster() &&*/ ((m_bNeedToFindEnemy && pEnemy->IsMoving() && enemy_distance > GameRule::MONSTER_TRACKING_RANGE_BY_TIME) || (m_nLastEnemyDistance != 0 && enemy_distance > GameRule::MONSTER_TRACKING_RANGE_BY_TIME && enemy_distance > m_nLastEnemyDistance)))
    {
        auto pHateTag = getHateTag(pEnemy->GetHandle(), t);
        if (pHateTag != nullptr)
        {
            pHateTag->bIsActive = false;
            ++pHateTag->nBadAttackCount;

            pHateTag->nHate -= std::max(static_cast<int32_t>(pHateTag->nHate * 0.5f), 100);
        }
        findNextEnemy();
        return;
    }

    m_nLastEnemyDistance = enemy_distance;
    auto gap = GetUnitSize() / 2 + pEnemy->GetUnitSize() / 2;
    if ((pEnemy->IsMoving(t) && (enemy_distance - gap > GetRealAttackRange() * 1.5f)) || (!pEnemy->IsMoving(t) && (enemy_distance - gap > GetRealAttackRange() * 1.2f)))
    {
        if (IsMoving() && !pEnemy->IsMoving(t))
        {
            auto targetPosition = GetTargetPos();
            if (targetPosition.GetExactDist2d(&enemyPosition) - gap <= GetRealAttackRange() * 1.2f)
                return;
        }

        if (!IsActable() || !IsMovable() || GetNextMovableTime() > t)
            return;

        auto targetPosition = enemyPosition;
        if (pEnemy->IsMoving())
            targetPosition = pEnemy->GetCurrentPosition(t + 15);
        else
            FindAttackablePosition(myPosition, targetPosition, enemy_distance, gap + GetRealAttackRange());

        if (sWorld.getBoolConfig(CONFIG_MONSTER_PATHFINDING) && !GameContent::IsBlocked(targetPosition.GetPositionX(), targetPosition.GetPositionY()))
        {
            // Pathfinding here
        }

        SetStatus(STATUS_TRACKING);

        auto homePosition = m_pRespawn;
        auto track_distance = myPosition.GetExactDist2d(&homePosition);

        if (/*!IsDungeonRaidMonster() &&*/ track_distance > GetChaseRange())
        {
            comeBackHome(true);
            return;
        }

        if (m_nLastTrackTime + 50 < t)
        {
            m_nLastTrackTime = t;

            float fMod = irand(0, 9);
            fMod /= 100.0f;
            fMod += 1.0f;

            if (GameContent::IsBlocked(targetPosition.GetPositionX(), targetPosition.GetPositionY()))
                return;

            if (IsMovable())
                sWorld.SetMove(this, GetCurrentPosition(t), targetPosition, static_cast<uint8_t>(GetRealMoveSpeed() * fMod), true, sWorld.GetArTime());
        }
    }
    else
    {
        if (GetStatus() == STATUS_FIND_ATTACK_POS && IsMoving(t))
            return;

        if (!pEnemy->IsMoving(t) && (GetStatus() == STATUS_TRACKING || (pEnemy->GetEnemyCount() < 6 && irand(0, 99) < GameRule::MONSTER_FIND_ATTACK_POS_RATIO)))
        {
            std::vector<uint32_t> vList{};
            sWorld.EnumMovableObject(myPosition, GetLayer(), GetUnitSize() / 2, vList);
            for (const auto &hUnit : vList)
            {
                auto pUnit = sMemoryPool.GetObjectInWorld<Unit>(hUnit);
                if (((pUnit != nullptr && pUnit->IsMonster()) && hUnit > GetHandle()) || ((GetStatus() == STATUS_TRACKING) && pEnemy->IsPlayer() && pEnemy->GetHandle() == hUnit))
                {
                    auto attack_pos = getNonDuplicateAttackPos(pEnemy);
                    SetStatus(STATUS_FIND_ATTACK_POS);
                    if (GameContent::IsBlocked(attack_pos.GetPositionX(), attack_pos.GetPositionY()))
                        return;

                    if (IsMovable())
                        sWorld.SetMove(this, myPosition, attack_pos, static_cast<uint8_t>(GetRealMoveSpeed()), true, sWorld.GetArTime());
                    return;
                }
            }
        }

        int32_t nPrevStatus = GetStatus();
        SetStatus(STATUS_ATTACK);

        if (GetNextAttackableTime() <= t)
        {
            if (!IsAttackable() || pEnemy == nullptr || !IsEnemy(pEnemy) || pEnemy->IsDead())
                return;

            if (IsMoving())
            {
                auto stopPosition = GetCurrentPosition(t + 10);
                sWorld.SetMove(this, myPosition, stopPosition, 0, true, sWorld.GetArTime());
            }

            AttackInfo Damages[4]{};
            bool bDoubleAttack{false};
            Attack(pEnemy, t, GetAttackInterval(), Damages, bDoubleAttack);

            float fAttackMotionRate{0.5f}; // We don't have this in Epic 4 yet, so lets default
            SetNextMovableTime(t + GetAttackInterval() * fAttackMotionRate);
            broadcastAttackMessage(pEnemy, Damages, GetAttackInterval() * 10, (GetNextAttackableTime() - t) * 10, bDoubleAttack, false);
        }

        if (pEnemy->IsMoving(t) && nPrevStatus == STATUS_TRACKING)
            SetStatus(STATUS_TRACKING);
    }
}

uint32_t Monster::GetCreatureGroup() const
{
    return m_Base->grp;
}

bool Monster::IsEnvironmentMonster() const
{
    return m_Base->fight_type == MonsterBase::FIGHT_TYPE_ENVIRONMENT;
}

bool Monster::IsBattleMode() const
{
    return GetStatus() == 1 || GetStatus() == 3 || GetStatus() == 2;
}

bool Monster::IsBossMonster() const
{
    return m_Base->monster_type >= MonsterBase::MONSTER_TYPE_DUNGEON_SUB_BOSS;
}

bool Monster::IsDungeonConnector() const
{
    return GetRace() == MonsterBase::MONSTER_RACE_DUNGEON_CONNECTOR;
}

bool Monster::IsDungeonCore() const
{
    return m_Base->race == MonsterBase::MONSTER_RACE_DUNGEON_CORE;
}

bool Monster::IsAgent() const
{
    return m_Base->fight_type == MonsterBase::FIGHT_TYPE_AGENT;
}

bool Monster::IsAutoTrap() const
{
    return m_Base->fight_type == MonsterBase::FIGHT_TYPE_AUTO_TRAP;
}

float Monster::GetChaseRange() const
{
    return (12 * m_Base->chase_range);
}

float Monster::GetFirstAttackRange()
{
    return m_Base->visible_range;
}

bool Monster::IsFirstAttacker() const
{
    return m_Base->flag[0] != 0;
}

bool Monster::IsGroupFirstAttacker() const
{
    return m_Base->flag[1] != 0;
}

bool Monster::IsCastRevenger() const
{
    return m_Base->flag[2] != 0;
}

bool Monster::IsBattleRevenger() const
{
    return m_Base->flag[4] != 0;
}

int32_t Monster::GetMonsterGroup() const
{
    return m_Base->monster_group;
}

int32_t Monster::GetTameItemCode() const
{
    int32_t result = m_Base->taming_id;
    if (result != 0)
    {
        auto si = sObjectMgr.GetSummonBase(result);
        if (si != nullptr)
            result = si->card_id;
    }
    return result;
}

int32_t Monster::GetTameCode() const
{
    return m_Base->taming_id;
}

float Monster::GetTamePercentage() const
{
    return m_Base->taming_percentage;
}

int32_t Monster::GetMonsterID() const
{
    return m_Base->id;
}

CreatureStat *Monster::GetBaseStat() const
{
    return sObjectMgr.GetStatInfo(m_Base->stat_id);
}

int32_t Monster::GetRace() const
{
    return m_Base->race;
}

bool Monster::IsMovable()
{
    if (m_Base->fight_type == MonsterBase::FIGHT_TYPE_DUNGEON_CONNECTOR ||
        m_Base->fight_type == MonsterBase::FIGHT_TYPE_AUTO_TRAP ||
        m_Base->fight_type == MonsterBase::FIGHT_TYPE_TRAP ||
        m_Base->fight_type == MonsterBase::FIGHT_TYPE_NOT_MOVABLE)
        return false;

    return Unit::IsMovable();
}

bool Monster::IsAttackable()
{
    if (m_Base->fight_type == MonsterBase::FIGHT_TYPE_ENVIRONMENT ||
        m_Base->fight_type == MonsterBase::FIGHT_TYPE_DUNGEON_CONNECTOR ||
        m_Base->fight_type == MonsterBase::FIGHT_TYPE_AUTO_TRAP ||
        m_Base->fight_type == MonsterBase::FIGHT_TYPE_TRAP)
        return false;

    return Unit::IsAttackable();
}

int32_t Monster::AddHate(uint32_t handle, int32_t pt, bool bBroadcast, bool bProcRoamingMonster)
{
    std::vector<uint32_t> vResult{};

    if (IsDead() || IsEnvironmentMonster())
        return 0;
    uint32_t ct = sWorld.GetArTime();
    auto unit = sMemoryPool.GetObjectInWorld<Unit>(handle);

    if (unit == nullptr /* || IsEnemy(unit, false)*/)
        return 0;

    auto ht = addHate(handle, pt);

    int32_t nHate = ht->nHate;
    if (unit != nullptr)
    {
        // IsRoamer
    }
    if (pt >= 0)
    {
        if (bBroadcast)
        {
            // moar roamer
        }
    }
    else
    {
        findNextEnemy();
    }
    return nHate;
}

HateTag *Monster::getHateTag(uint32_t handle, uint32_t t)
{
    auto pos = std::find_if(m_vHateList.begin(),
                            m_vHateList.end(),
                            [handle](const HateTag &ht) { return ht.uid == handle; });

    if (pos != m_vHateList.end())
    {
        pos->nTime = t;
        return &*pos;
    }
    return nullptr;
}

HateTag *Monster::addHate(uint32_t handle, int32_t nHate)
{
    auto t = sWorld.GetArTime();
    auto pHateTag = getHateTag(handle, t);

    if (pHateTag == nullptr)
    {
        m_vHateList.emplace_back(HateTag(handle, t, 0));
        pHateTag = &m_vHateList.back();

        sMemoryPool.GetObjectInWorld<Unit>(handle)->AddToEnemyList(GetHandle());
    }

    pHateTag->nHate += nHate;
    if (pHateTag->nHate < 0)
        pHateTag->nHate = 0;

    pHateTag->nLastMaxHate = pHateTag->nHate;
    if (!pHateTag->bIsActive)
    {
        pHateTag->bIsActive = true;

        int32_t nFrenzyLevel{0};
        uint32_t nFrenzyTime{0};

        if (pHateTag->nBadAttackCount == 1)
        {
            nFrenzyLevel = 1;
            nFrenzyTime = 300;
        }
        else
        {
            nFrenzyLevel = 2;
            nFrenzyTime = 500;
        }

        AddState(SG_NORMAL, SC_FRENZY, GetHandle(), nFrenzyLevel, t, t + nFrenzyTime);
    }

    if (!IsAttacking())
    {
        m_nMaxHate = pHateTag->nHate;
        auto pTarget = sMemoryPool.GetObjectInWorld<Unit>(handle);
        if (pTarget != nullptr)
        {
            StartAttack(handle, false);
        }
    }
    else if (GetTargetHandle() != handle && pHateTag->nHate > m_nMaxHate)
    {
        m_nMaxHate = pHateTag->nHate;
        SetUInt32Value(BATTLE_FIELD_TARGET_HANDLE, handle);
    }
    else if (GetTargetHandle() == handle)
    {
        m_nMaxHate = pHateTag->nHate;
    }

    return pHateTag;
}

bool Monster::removeFromHateList(uint32_t handle)
{
    bool found{false};
    for (int32_t i = m_vHateList.size() - 1; i >= 0; --i)
    {
        auto ht = m_vHateList[i];
        if (ht.uid == handle)
        {
            m_vHateList.erase(m_vHateList.begin() + i);
            auto unit = sMemoryPool.GetObjectInWorld<Unit>(handle);
            if (unit != nullptr)
            {
                found = true;
                // RemoveFromEnemyList
            }
            break;
        }
    }
    return found;
}

void Monster::processWalk(uint32_t t)
{
    ArMoveVector tmp_mv{};
    tmp_mv.Copy(*this);
    tmp_mv.Step(t);
    int32_t wpi{0};
    if (GetStatus() == STATUS_NORMAL && !m_bComeBackHome && m_pWayPointInfo != nullptr)
    {
        m_pRespawn.m_positionX = tmp_mv.GetPositionX();
        m_pRespawn.m_positionY = tmp_mv.GetPositionY();
        if (m_nWayPointIdx < 0 && m_pWayPointInfo->way_point_type == 1)
            wpi -= tmp_mv.ends.size();
        else
            wpi = (int32_t)(m_pWayPointInfo->vWayPoint.size() - tmp_mv.ends.size() - 1);
        m_nWayPointIdx = wpi;
    }

    if ((uint32_t)(tmp_mv.GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)) != (uint32_t)(GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)) || (uint32_t)(tmp_mv.GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)) != (uint32_t)(GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)) || !tmp_mv.bIsMoving)
    {
        if (!IsInWorld())
            return;

        if (m_bComeBackHome && !IsBattleMode())
        {
            if (tmp_mv.bIsMoving)
            {
                sWorld.onRegionChange(this, t - lastStepTime, tmp_mv.bIsMoving == 0);
                return;
            }
            if (HasFlag(UNIT_FIELD_STATUS, STATUS_INVINCIBLE))
                RemoveFlag(UNIT_FIELD_STATUS, STATUS_INVINCIBLE);
            m_bComeBackHome = false;
        }
        sWorld.onRegionChange(this, t - lastStepTime, !tmp_mv.bIsMoving);
    }
}

void Monster::processMove(uint32_t t)
{
    if (IsDungeonConnector() || GetHealth() == 0)
        return;

    std::vector<Position> vMoveInfo{};
    Position targetPos{};

    if (m_pWayPointInfo == nullptr)
    {
        if (GetStatus() != STATUS_NORMAL || m_bNearClient || !m_vHateList.empty())
        {
            if (m_bIsWandering)
            {
                int32_t rnd = rand32();
                if (GetStatus() == STATUS_NORMAL && GetHealth() != 0 && (!bIsMoving || !IsInWorld()) && rnd % 500 + lastStepTime + 1000 < t && (rnd % 3) != 0)
                {
                    getMovePosition(targetPos);
                    if (!GameContent::IsBlocked(targetPos.GetPositionX(), targetPos.GetPositionY()))
                    {
                        auto speed = (uint8_t)(m_Attribute.nMoveSpeed / 7);
                        sWorld.SetMove(this, GetCurrentPosition(t), targetPos, speed, true, sWorld.GetArTime(), true);
                        m_pRespawn.m_positionX = targetPos.GetPositionX();
                        m_pRespawn.m_positionY = targetPos.GetPositionY();
                    }
                }
            }
        }
        else
        {
            sWorld.ClearTamer(this, true);
            SetStatus(STATUS_NORMAL);
        }
    }
    else
    {
        if (!bIsMoving || !IsInWorld())
        {
            int32_t lwpi = (int32_t)m_pWayPointInfo->vWayPoint.size() - 1;
            if (m_nWayPointIdx >= lwpi)
            {
                if (m_pWayPointInfo->way_point_type == 1)
                    m_nWayPointIdx = -lwpi;
                else
                    m_nWayPointIdx = -1;
            }

            if (m_pWayPointInfo->way_point_type == 1 && m_nWayPointIdx < 0)
                lwpi = 0;
            int32_t wpi = m_nWayPointIdx;
            while (true)
            {
                ++wpi;
                if (wpi > lwpi)
                    break;
                int32_t awpi = std::abs(wpi);
                targetPos = m_pWayPointInfo->vWayPoint[awpi];
                vMoveInfo.emplace_back(targetPos);
            }

            t = sWorld.GetArTime();
            auto speed = (uint8_t)(m_pWayPointInfo->way_point_speed / 7);

            if (speed == 0)
                speed = (uint8_t)(m_Attribute.nMoveSpeed / 7);

            sWorld.SetMultipleMove(this, GetCurrentPosition(t), vMoveInfo, speed, true, t, true);
        }
    }
}

void Monster::processFirstAttack(uint32_t t)
{
    if (GetStatus() != STATUS_NORMAL || !(IsAgent() || IsFirstAttacker() || IsBattleRevenger()))
        return;

    std::vector<uint32_t> vResult{};
    std::vector<Monster *> vSameGroup{};
    uint32_t target{0};

    if (IsAgent())
    {
    }
    else
    {
        sWorld.EnumMovableObject(this->GetPosition(), GetLayer(), GetFirstAttackRange(), vResult, true, true);

        auto distance = GetFirstAttackRange() + 1.0f;
        for (const auto &handle : vResult)
        {
            auto unit = sMemoryPool.GetObjectInWorld<Unit>(handle);
            if (unit != nullptr && unit->GetHealth() != 0)
            {
                if (!IsFirstAttacker() && unit->IsMonster() && unit->GetTargetHandle() != 0)
                    target = unit->GetTargetHandle();

                if (IsGroupFirstAttacker() && unit->IsMonster() && unit != this)
                {
                    auto mob = unit->As<Monster>();
                    if (mob->GetMonsterGroup() == GetMonsterGroup())
                        vSameGroup.emplace_back(mob);
                }

                if (IsFirstAttacker() && IsEnemy(unit, false))
                {
                    auto _distance = unit->GetCurrentPosition(t).GetExactDist2d(this);
                    if (!unit->IsSummon() || (GetFirstAttackRange() * 0.5f >= _distance && distance > _distance))
                    {
                        distance = _distance;
                        target = handle;
                    }
                }
            }
        }
    }

    if (target == 0)
        return;

    if (IsFirstAttacker())
    {
        AddHate(target, 1, false, true);
        if (IsGroupFirstAttacker())
        {
            for (const auto &m : vSameGroup)
                m->AddHate(target, 1, false, true);
        }
        else
        {
            AddHate(target, 1, true, true);
        }
    }
}

void Monster::FindAttackablePosition(Position &myPosition, Position &enemyPosition, float distance, float gap)
{
    auto duplicateEnemyPosition = enemyPosition.GetPosition();
    float walk_length = distance - gap;
    float v{0};
    if (walk_length >= 0.0f)
    {
        v = (walk_length / distance) + 0.1f;
        enemyPosition.m_positionX = ((enemyPosition.GetPositionX() - myPosition.GetPositionX()) * v) + myPosition.GetPositionX();
        enemyPosition.m_positionY = ((enemyPosition.GetPositionY() - myPosition.GetPositionY()) * v) + myPosition.GetPositionY();
        enemyPosition.m_positionZ = ((enemyPosition.GetPositionZ() - myPosition.GetPositionZ()) * v) + myPosition.GetPositionZ();

        if (!GameContent::IsBlocked(enemyPosition.GetPositionX(), enemyPosition.GetPositionY()))
            return;

        enemyPosition = duplicateEnemyPosition;
    }
    else
    {
        enemyPosition = myPosition;
    }
}

void Monster::getMovePosition(Position &newPos)
{
    newPos.m_positionX = (((uint32_t)rand32()) % 120) + GetPositionX() - 60.0f;
    newPos.m_positionY = (((uint32_t)rand32()) % 120) + GetPositionY() - 60.0f;

    if (newPos.m_positionX < 0.0f)
        newPos.m_positionX = 0.0f;
    if (newPos.m_positionY < 0.0f)
        newPos.m_positionY = 0.0f;
}

Position Monster::getNonDuplicateAttackPos(Unit *pEnemy)
{
    uint32_t ct = sWorld.GetArTime();
    auto result = pEnemy->GetCurrentPosition(ct);

    auto range = GetRealAttackRange() + (GetUnitSize() * 0.5f) + (pEnemy->GetUnitSize() * 0.5f);
    float fMod = irand(-1, 3);
    if (fMod == 0.0f)
        fMod = 1.0f;

    float angle = fMod * 0.6283f;
    auto myPosition = GetCurrentPosition(ct);
    float distance = myPosition.GetExactDist2d(&result);
    fMod = 0.0f;
    if (distance != 0.0f)
        fMod = acos((myPosition.GetPositionX() - result.GetPositionX()) / distance);
    if (myPosition.GetPositionY() - result.GetPositionY() < 0.0f)
        fMod = 0.6283f - fMod;
    angle = fMod + angle;
    fMod = cos(angle);
    result.m_positionX = fMod * range + result.GetPositionX();
    fMod = sin(angle);
    result.m_positionY = fMod * range + result.GetPositionY();
    return result;
}

void Monster::onBeforeCalculateStat()
{
    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_SPEED_FIXED))
        m_Attribute.nMoveSpeed += (m_Base->run_speed - 120);

    m_Attribute.nAttackRange = m_Base->attack_range;
    m_vHateModifierByState.clear();
}

void Monster::SetTamer(uint32_t handle, int32_t nTamingSkillLevel)
{
    m_hTamer = handle;
    m_nTamedTime = sWorld.GetArTime();
    m_nTamingSkillLevel = nTamingSkillLevel;
}

uint32_t Monster::GetTamer() const
{
    return m_hTamer;
}

void Monster::procQuest(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute)
{
    std::vector<Quest *> vQuestList{};
    std::vector<Player *> vPlayer{};

    if (!vPartyContribute.empty())
    {
        if (vPartyContribute.front().hPlayer != 0)
        {
            auto player = sMemoryPool.GetObjectInWorld<Player>(vPartyContribute.front().hPlayer);
            if (player != nullptr)
                vPlayer.emplace_back(player);
        }
        else
        {
            sGroupManager.DoEachMemberTag(vPartyContribute.front().nPartyID, [&vPlayer, pos](PartyMemberTag &tag) {
                if (tag.bIsOnline && tag.pPlayer != nullptr && tag.pPlayer->GetExactDist2d(&pos) <= 500.0f)
                {
                    vPlayer.emplace_back(tag.pPlayer);
                }
            });
        }
        for (auto &p : vPlayer)
        {
            p->UpdateQuestStatusByMonsterKill(m_Base->id);
            vQuestList.clear();
            p->GetQuestByMonster(m_Base->id, vQuestList, 4112);

            for (auto &q : vQuestList)
            {
                if (q != nullptr && q->m_QuestBase->nDropGroupID != 0)
                {
                    if (!q->IsFinishable())
                    {
                        dropItemGroup(pos, pKiller, pPriority, vPartyContribute, q->m_QuestBase->nDropGroupID, 1, 1, -1);
                    }
                }
            }
        }
    }
}

void Monster::procDropChaos(Unit *pKiller, std::vector<VirtualParty> &vPartyContribute, float fDropRatePenalty)
{
    float fChance = m_Base->chaos_drop_percentage * GameRule::GetChaosDropRate() * fDropRatePenalty;
    auto rnd = (uint32_t)rand32();
    float chaos = rnd % 100;

    if (chaos >= fChance)
        return;

    chaos = irand(m_Base->chaos_min[0], m_Base->chaos_max[0]);
    for (auto &vp : vPartyContribute)
    {
        float fSharedChaos = vp.fContribute * chaos;
        if (vp.hPlayer == 0)
        {
            //sWorld.addChaos(this, vp.nPartyID, fSharedChaos);
        }
        else
        {
            auto p = sMemoryPool.GetObjectInWorld<Player>(vp.hPlayer);
            if (p != nullptr)
                sWorld.addChaos(this, p, fSharedChaos);
        }
    }
}

bool Monster::IsAlly(const Unit *pTarget)
{
    return pTarget->IsMonster() || pTarget->IsNPC();
}

void Monster::TriggerForceKill(Player *pPlayer)
{
    pFCClient = pPlayer;
    bForceKill = true;
}

const std::string &Monster::GetNameAsString()
{
    return sObjectMgr.GetValueFromNameID(m_Base->name_id);
}

int32_t Monster::RemoveHate(uint32_t handle, int32_t pt)
{
    auto pHateTag = getHateTag(handle, sWorld.GetArTime());
    if (pHateTag != nullptr)
    {
        if (pt < pHateTag->nHate)
        {
            pHateTag->nHate -= pt;
            if (handle == GetTargetHandle() && !IsDead())
                findNextEnemy();
            return pHateTag->nHate;
        }
        else
        {
            removeFromHateList(handle);
            if (handle == GetTargetHandle())
            {
                SetUInt32Value(BATTLE_FIELD_TARGET_HANDLE, 0);
                findNextEnemy();
            }
            return 0;
        }
    }

    return -1;
}

int32_t Monster::GetHate(uint32_t handle)
{
    if (auto ht = getHateTag(handle, 0); ht != nullptr)
        return ht->nHate;
    return 0;
}
