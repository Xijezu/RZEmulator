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

#include "Monster.h"
#include "Messages.h"
#include "XPacket.h"
#include "World.h"
#include "Summon.h"
#include "Player.h"
#include "MemPool.h"
#include "ObjectMgr.h"
#include "GameRule.h"
#include "RegionContainer.h"
#include "Skill.h"
#include "GroupManager.h"

Monster::Monster(uint handle, MonsterBase *mb) : Unit(true)
{
    _mainType    = MT_NPC;
    _subType     = ST_Mob;
    _objType     = OBJ_MOVABLE;
    _valuesCount = BATTLE_FIELD_END;

    _InitValues();
    _InitTimerFieldsAndStatus();
    SetUInt32Value(UNIT_FIELD_HANDLE, handle);
    SetInt32Value(UNIT_FIELD_UID, mb->id);
    m_Base = mb;
    SetInt32Value(UNIT_FIELD_RACE, m_Base->race);
    m_nStatus = STATUS_NORMAL;
    SetLevel((uint8)mb->level);
    CalculateStat();
    SetHealth(GetMaxHealth());
    SetMana(GetMaxMana());
    SetOrientation((float)(rand32() / 100));

    m_bNearClient         = false;
    m_nLastEnemyDistance  = 0.0f;
    m_nLastTrackTime      = 0;
    m_bComeBackHome       = false;
    m_nLastHateUpdateTime = 0;
    m_bNeedToFindEnemy    = false;
    m_hFirstAttacker      = 0;
    m_nFirstAttackTime    = 0;
    m_nTotalDamage        = 0;
    m_nMaxHate            = 0;
    m_hEnemy              = 0;

    m_nTamingSkillLevel = 0;
    m_hTamer            = 0;
    m_nTamedTime        = 0;
    m_bTamedSuccess     = false;
    m_bIsWandering      = false;
    m_pWayPointInfo     = nullptr;
    m_nWayPointIdx      = 0;
}

void Monster::EnterPacket(XPacket &pEnterPct, Monster *monster, Player *pPlayer)
{
    Unit::EnterPacket(pEnterPct, monster, pPlayer);
    //pEnterPct << (uint32_t)0;
    Messages::GetEncodedInt(pEnterPct, (uint)bits_scramble(monster->m_Base->id));
    // pEnterPct << (uint32_t)0;
    pEnterPct << (uint8_t)0;
}

int Monster::onDamage(Unit *pFrom, ElementalType elementalType, DamageType damageType, int nDamage, bool bCritical)
{
    Player *player{nullptr};
    Summon *summon{nullptr};

    if (GetHealth() - nDamage <= 0)
        nDamage = GetHealth();

    if (nDamage == 0)
        return Unit::onDamage(pFrom, elementalType, damageType, nDamage, bCritical);

    uint ct = sWorld->GetArTime();

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
        m_nFirstAttackTime   = ct;
    }
    this->addDamage(pFrom->GetHandle(), nDamage);
    if (player != nullptr && m_hTamer == player->GetHandle())
        m_nTamedTime += (int)(ct + 30000);
    return Unit::onDamage(pFrom, elementalType, damageType, nDamage, bCritical);
}

void Monster::onDead(Unit *pFrom, bool decreaseEXPOnDead)
{
    Unit::onDead(pFrom, decreaseEXPOnDead);
    SetStatus(STATUS_DEAD);

    std::vector<VirtualParty> vPartyContribute{ };
    takePriority              Priority{ };

    m_bTamedSuccess = false;
    if (m_hTamer != 0)
    {
        auto player         = sMemoryPool->GetObjectInWorld<Player>(m_hTamer);
        if (player != nullptr)
            m_bTamedSuccess = sWorld->ProcTame(this);
    }

    calcPartyContribute(pFrom, vPartyContribute);
    procEXP(pFrom, vPartyContribute);
    if (!m_bTamedSuccess)
    {
        uint ct  = sWorld->GetArTime();
        auto pos = GetCurrentPosition(ct);

        int       i = 0;
        for (auto &vp : vPartyContribute)
        {
            if (m_Base->monster_type >= 31 /* && isDungeonRaidMonster */)
            {
                Priority.PickupOrder.hPlayer[i]  = 0;
                Priority.PickupOrder.nPartyID[i] = 0;
            }
            else if (vp.hPlayer != 0)
            {
                Priority.PickupOrder.hPlayer[i]  = vp.hPlayer;
                Priority.PickupOrder.nPartyID[i] = 0;
            }
            else
            {
                Priority.PickupOrder.hPlayer[i]  = 0;
                Priority.PickupOrder.nPartyID[i] = vp.nPartyID;
            }

            i++;
            if (i >= 3)
                break;
        }

        int cl;
        if (!vPartyContribute.empty())
            cl = vPartyContribute[0].nLevel;
        else
            cl = 0;

        auto fDropRatePenalty = 1.0f;
        cl -= GetLevel();
        if (cl >= 5)
        {
            fDropRatePenalty     = 1.0f - (float)pow(cl - 3, 2.0f) * 0.02f;
            if (fDropRatePenalty < 0.0f)
                fDropRatePenalty = 0.0f;
        }

        float fChaosDropRateBonus = 1.0f;
        float fItemDropRateBonus  = 1.0f;
        float fGoldDropRateBonus  = 1.0f;

        auto player = dynamic_cast<Player *>(pFrom);
        procDropChaos(pFrom, Priority, vPartyContribute, fDropRatePenalty);
        if (pFrom->IsPlayer() && player->GetChaos() < 1 && player->GetQuestProgress(1032) == 1)
            sWorld->addChaos(this, player, 1.0f);

        if (m_Base->monster_type < 31 /* || !IsDungeonRaidMonster*/)
        {
            procDropGold(pos, pFrom, Priority, vPartyContribute, fDropRatePenalty);
            procDropItem(pos, pFrom, Priority, vPartyContribute, fDropRatePenalty);
        }



        // TODO: OnDeath script
    }
}

void Monster::calcPartyContribute(Unit *pKiller, std::vector<VirtualParty> &vPartyContribute)
{
    if (m_vDamageList.empty())
        return;

    uint   t                   = sWorld->GetArTime();
    uint   hKiller             = 0;
    float  fMaxDamageAdd       = 0.1f;
    uint   totalDamage         = 0;
    int    nLastAttackPartyID  = 0;
    int    nFirstAttackPartyID = 0;
    Player *player{nullptr};

    if (m_nFirstAttackTime + 6000 < t)
    {
        m_hFirstAttacker = 0;
        fMaxDamageAdd    = 0.4f;
    }

    if (pKiller->IsSummon())
    {
        player = dynamic_cast<Summon *>(pKiller)->GetMaster();
    }
    else if (pKiller->IsPlayer())
    {
        player = dynamic_cast<Player *>(pKiller);
        if (player != nullptr)
        {
            hKiller            = player->GetHandle();
            nLastAttackPartyID = player->GetPartyID();
        }
    }

    for (auto &dt : m_vDamageList)
    {
        totalDamage += dt.nDamage;
        auto p = sMemoryPool->GetObjectInWorld<Player>(dt.uid);
        if (p != nullptr)
        {
            if (p->GetHandle() == m_hFirstAttacker)
            {
                nFirstAttackPartyID = p->GetPartyID();
            }
            if (p->GetPartyID() != 0)
            {
                bool      bHandled = false;
                for (auto &contribute : vPartyContribute)
                {
                    if (contribute.nPartyID == p->GetPartyID())
                    {
                        bHandled = true;
                        contribute.nDamage += dt.nDamage;
                        break;
                    }
                }
                if (!bHandled)
                {
                    VirtualParty virtualParty{ };
                    virtualParty.nDamage += dt.nDamage;
                    virtualParty.nPartyID = p->GetPartyID();
                    virtualParty.nLevel   = p->GetLevel();
                    vPartyContribute.emplace_back(virtualParty);
                }
                continue;
            }
            VirtualParty vp{dt.uid, dt.nDamage, (int)p->GetLevel()};
            vPartyContribute.emplace_back(vp);
        }
    }

    for (auto &vp : vPartyContribute)
    {
        vp.fContribute = GetPct((float)vp.nDamage, totalDamage) / 100;
        if (nFirstAttackPartyID != 0)
        {
            if (nFirstAttackPartyID == vp.nPartyID)
                vp.fContribute += 0.3f;
        }
        else if (m_hFirstAttacker != 0 && m_hFirstAttacker == vp.hPlayer)
        {
            vp.fContribute += 0.3f;
        }
        if (nLastAttackPartyID != 0)
        {
            if (vp.nPartyID == nLastAttackPartyID)
                vp.fContribute += 0.1f;
        }
        else
        {
            if (vp.hPlayer == hKiller)
                vp.fContribute += 0.1f;
        }
    }

    std::sort(vPartyContribute.begin(), vPartyContribute.end(),
              [](const VirtualParty &a, const VirtualParty &b) -> bool {
                  return a.fContribute > b.fContribute;
              });
}

DamageTag *Monster::addDamage(uint handle, int nDamage)
{
    uint ct  = sWorld->GetArTime();
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

DamageTag *Monster::getDamageTag(uint handle, uint t)
{
    if (m_vDamageList.empty())
        return nullptr;
    for (uint i = 0; i < m_vDamageList.size(); i++)
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

    float fSharedJP  = 0.0f;
    float fSharedEXP = 0.0f;

    for (auto &vp : vPartyContribute)
    {
        fSharedEXP     = vp.fContribute * m_Base->exp[0];
        fSharedJP      = vp.fContribute * m_Base->jp[0];
        if (fSharedEXP < 1)
            fSharedEXP = 1;

        if (vp.bTamer || (vp.hPlayer != 0 && vp.hPlayer == m_hTamer))
        {
            fSharedEXP = fSharedEXP * m_Base->taming_exp_mod;
            fSharedJP  = fSharedJP * m_Base->taming_exp_mod;
        }

        if (vp.hPlayer != 0)
        {
            auto player2 = sMemoryPool->GetObjectInWorld<Player>(vp.hPlayer);
            if (player2 != nullptr)
                sWorld->addEXP(this, player2, fSharedEXP, fSharedJP);
        }
        else
        {
            sWorld->addEXP(this, vp.nPartyID, (int)fSharedEXP, (int)fSharedJP);
        }
    }
}

void Monster::Update(uint diff)
{
    if (bForceKill)
        ForceKill(pFCClient);

    if (!m_bNearClient)
        return;

    uint           ct = sWorld->GetArTime();
    MONSTER_STATUS ms = GetStatus();

    if (ms != STATUS_NORMAL)
    {
        if ((int)ms > 0)
        {
            if ((int)ms <= 3)
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
        if (IsInWorld() && IsActable() && (/* m_bIsDungeonRaidMonster && */!m_bComeBackHome))
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
        if (m_nTamedTime + 30000 < static_cast<int>(ct))
        {
            m_hTamer     = 0;
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

void Monster::processDead(uint t)
{
    if (m_pDeleteHandler != nullptr)
    {
        m_pDeleteHandler->onMonsterDelete(this);
    }

    if (GetUInt32Value(UNIT_FIELD_DEAD_TIME) + 1200 < t)
    {
        if (IsInWorld())
            sWorld->RemoveObjectFromWorld(this);
        //sMemoryPool->RemoveObject(this, true);
        DeleteThis();
    }
}

void Monster::SetStatus(MONSTER_STATUS status)
{
    if (m_nStatus != STATUS_DEAD)
    {
        //if((int)status != m_nStatus && (int)status != 4 && (int)status != 0 && m_nStatus == 0)
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
    int64    item_count;
    for (int i = 0; i < 10; ++i)
    {
        if (m_Base->drop_item_id[i] != 0 && sWorld->checkDrop(pKiller, m_Base->drop_item_id[i], m_Base->drop_percentage[i], fDropRatePenalty, 1))
        {
            item_count = irand(m_Base->drop_min_count[i], m_Base->drop_max_count[i]);
            if (item_count < m_Base->drop_min_count[i])
            {
                NG_LOG_WARN("entities.monster", "Monster::procDropItem: Min/Max Count error!");
            }
            else
            {
                int level = irand(m_Base->drop_min_level[i], m_Base->drop_max_level[i]);
                int code  = m_Base->drop_item_id[i];
                if (code >= 0)
                    dropItem(pos, pKiller, pPriority, vPartyContribute, code, item_count, level, false, -1);
                else
                    dropItemGroup(pos, pKiller, pPriority, vPartyContribute, code, item_count, level, -1);
            }
        }
    }
    procQuest(pos, pKiller, pPriority, vPartyContribute);
}

void Monster::dropItem(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, int code, long count, int level, bool bIsEventItem, int nFlagIndex)
{
    if (count == 0)
    {
        NG_LOG_ERROR("entities.monster", "Monster::dropItem: count was 0. (x: %f, y: %f, code: %u, Killer: %s", pos.GetPositionX(), pos.GetPositionY(), code, pKiller->GetName());
        return;
    }

    WorldObject *cr{nullptr};
    Player      *player{nullptr};

    for (auto &ht : m_vHateList)
    {
        if (ht.uid != 0)
        {
            cr = sMemoryPool->GetObjectInWorld<WorldObject>(ht.uid);

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

        if ((uint)nFlagIndex <= 0x1F)
            ni->m_Instance.Flag |= (uint)(1 << (nFlagIndex & 0x1F));
        if (bIsEventItem)
        {
            ni->m_Instance.Flag |= 0x10;
        }
        sWorld->MonsterDropItemToWorld(this, ni);
    }
}

void Monster::dropItemGroup(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, int nDropGroupID, long count, int level, int nFlagIndex)
{
    std::map<int, long> mapDropItem{ };
    int64               nItemCount;
    int                 nItemID;

    for (int i = 0; i < count; ++i)
    {
        nItemID    = nDropGroupID;
        nItemCount = 1;

        do
            sObjectMgr->SelectItemIDFromDropGroup(nItemID, nItemID, nItemCount);
        while (nItemID < 0);
        if (nItemID > 0)
        {
            dropItem(pos, pKiller, pPriority, vPartyContribute, nItemID, nItemCount, level, false, nFlagIndex);
        }
    }
}

void Monster::ForceKill(Player *byPlayer)
{
    auto dmg = onDamage(byPlayer, TYPE_NONE, DT_NORMAL_PHYSICAL_DAMAGE, GetHealth(), false);
    damage(byPlayer, dmg, false);
    Messages::SendHPMPMessage(byPlayer, this, 0, 0, true);
}

void Monster::procDropGold(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, float fDropRatePenalty)
{
    int64  gold;
    int    tax;
    int    nGuildID;
    Player *player{nullptr};
    fDropRatePenalty = GameRule::GetGoldDropRate() * fDropRatePenalty;
    if ((uint)rand32() % 100 >= m_Base->gold_drop_percentage * fDropRatePenalty)
        return;

    gold     = (int64)irand(m_Base->gold_min[0], m_Base->gold_max[0]);
    if (gold < 0)
        gold = 1;

    if (gold > 1000000)
        gold = 1000000;
    auto gi = sMemoryPool->AllocGold(gold, BY_MONSTER);
    gi->SetCurrentXY(pos.GetPositionX(), pos.GetPositionY());
    gi->SetLayer(GetLayer());

    gi->AddNoise(rand32(), rand32(), 9);
    gi->SetPickupOrder(pPriority.PickupOrder);
    sWorld->MonsterDropItemToWorld(this, gi);
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
    int  nMaxHate{-1};
    uint target{0};

    for (auto &ht : m_vHateList)
    {
        if (ht.bIsActive)
        {
            int nHate = ht.nHate;

            for (auto &hmt : m_vHateModifierByState)
            {
                if (hmt.uid == ht.uid)
                {
                    nMaxHate += hmt.nHate;
                    break;
                }
            }

            if (nHate > nMaxHate)
            {
                auto *unit = sMemoryPool->GetObjectInWorld<WorldObject>(ht.uid);
                if (unit != nullptr
                    && sRegion->IsVisibleRegion((uint)(unit->GetPositionX() / g_nRegionSize), (uint)(unit->GetPositionY() / g_nRegionSize),
                                                (uint)(GetPositionX() / g_nRegionSize), (uint)(GetPositionY() / g_nRegionSize)) != 0
                    /*&&  IsVisible(unit) */)
                {
                    nMaxHate = nHate;
                    target   = ht.uid;
                }
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
        m_hEnemy   = target;
        SetFlag(UNIT_FIELD_STATUS, STATUS_FIRST_ATTACK);
    }
}

void Monster::comeBackHome(bool bInvincible)
{
    std::vector<Position> pl{ };
    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_FEARED))
    {
        SetStatus(STATUS_NORMAL);
        m_nLastHateUpdateTime = sWorld->GetArTime();
        m_hEnemy              = 0;
        m_nMaxHate            = 0;
        m_bComeBackHome       = true;

        pl.emplace_back(m_pRespawn);
        if (bInvincible)
            SetFlag(UNIT_FIELD_STATUS, STATUS_INVINCIBLE);

        SetPendingMove(pl, (uint8)(2 * m_Attribute.nMoveSpeed / 7));
    }
}

bool Monster::StartAttack(uint target, bool bNeedFastReaction)
{
    bool result{false};
    if (Unit::StartAttack(target, bNeedFastReaction))
    {
        SetStatus(STATUS_ATTACK);
        result = true;
    }
    return result;
}

void Monster::AI_processAttack(uint t)
{
    if (m_castingSkill != nullptr)
    {
        m_castingSkill->ProcSkill();
        return;
    }

    auto target = sMemoryPool->GetObjectInWorld<Unit>(m_hEnemy);
    if (target == nullptr
        || target->GetHandle() != m_hEnemy
        || target->GetHealth() == 0
        || !target->IsInWorld())
    {
        removeFromHateList(m_hEnemy);
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
        removeFromHateList(m_hEnemy);
        findNextEnemy();
    }
}

void Monster::AI_processAttack(Unit *pEnemy, uint t)
{
    if (!IsInWorld() || pEnemy == nullptr)
        return;

    if ((IsNonAttacker()
         || IsAutoTrap()
         || (IsAgent() && !pEnemy->IsPlayer())
         || sRegion->IsVisibleRegion(pEnemy, this) == 0
         || pEnemy->GetLayer() != GetLayer())
        && pEnemy->GetTargetHandle() != GetHandle())
    {
        comeBackHome(false);
        return;
    }

    //Step(t);
    Position enemyPosition = pEnemy->GetCurrentPosition(t);
    Position myPosition    = GetCurrentPosition(t);
    auto     ry1           = myPosition.GetExactDist2d(&enemyPosition);

    // Trigger condition
    if (GetNextAttackableTime() <= t)
    {

    }

    auto ht = getHateTag(pEnemy->GetHandle(), 0);
    if (/*!m_bIsDungeonRaidMonster*/ ((ht != nullptr && ht->nTime + 2500 < t)
                                      && pEnemy->bIsMoving
                                      && pEnemy->IsInWorld()
                                      && (ry1 > 360.0f
                                          || (m_nLastEnemyDistance != 0.0f
                                              && ry1 > 600.0f
                                              && m_nLastEnemyDistance < ry1))))
    {
        auto ht2 = getHateTag(pEnemy->GetHandle(), t);

        if (ht2 != nullptr)
        {
            ht2->nBadAttackCount++;
            ht2->bIsActive = false;
            float fMod = ((float)ht2->nHate * 0.5f);
            if (fMod < 100)
                fMod = 100;
            ht2->nHate -= (int)fMod;
        }
        findNextEnemy();
        return;
    }
    m_nLastEnemyDistance = ry1;

    auto rx1 = (pEnemy->GetUnitSize() * 0.5f) + (GetUnitSize() * 0.5f);
    if ((pEnemy->IsMoving(t)
         && GetRealAttackRange() * 1.5f < ry1 - rx1)
        || (!pEnemy->IsMoving(t)
            && GetRealAttackRange() * 1.2f < ry1 - rx1))
    {

        if (bIsMoving
            && IsInWorld()
            && !pEnemy->IsMoving(t)
            && ((GetRealAttackRange() * 1.2f >= GetTargetPos().GetExactDist2d(&enemyPosition) - rx1)
            || !IsActable()
            || !IsMovable()
            || m_nMovableTime > t))
        {
            return;
        }

        auto targetPosition = enemyPosition.GetPosition();
        if (!pEnemy->bIsMoving || !pEnemy->IsInWorld())
            FindAttackablePosition(myPosition, targetPosition, ry1, GetRealAttackRange() + rx1);
        else
            targetPosition = pEnemy->GetCurrentPosition(t + 15);
        // Pathfinding here
        SetStatus(STATUS_TRACKING);
        auto homePosition   = m_pRespawn.GetPosition();
        auto track_distance = myPosition.GetExactDist2d(&homePosition);
        if (/*isDungeonRaidMonster || */GetChaseRange() >= track_distance)
        {
            if (m_nLastTrackTime + 50 < t)
            {
                m_nLastTrackTime = t;
                track_distance   = (((float)irand(0, 9) / 100.0f) + 1.0f);
                if (!sObjectMgr->IsBlocked(targetPosition.GetPositionX(), targetPosition.GetPositionY()) && IsMovable())
                {
                    sWorld->SetMove(this, GetCurrentPosition(t), targetPosition, (uint8)((m_Attribute.nMoveSpeed / 7) * track_distance), true, sWorld->GetArTime(), true);
                }
            }
            return;
        }
        comeBackHome(true);
        return;
    }
    if (GetStatus() == STATUS_FIND_ATTACK_POS && IsMoving(t))
    {
        return;
    }

    if (!pEnemy->IsMoving(t) && (GetStatus() == STATUS_TRACKING && irand(0, 99) < 5))
    {
        auto attack_pos = getNonDuplicateAttackPos(pEnemy);
        SetStatus(STATUS_FIND_ATTACK_POS);
        if (!sObjectMgr->IsBlocked(attack_pos.GetPositionX(), attack_pos.GetPositionY()) && IsMovable())
        {
            sWorld->SetMove(this, myPosition, attack_pos, (uint8)(m_Attribute.nMoveSpeed / 7), true, sWorld->GetArTime(), true);
        }
        return;
    }

    int nPrevStatus = GetStatus();
    SetStatus(STATUS_ATTACK);
    if (GetNextAttackableTime() > t)
    {
        if (pEnemy->IsMoving(t) && nPrevStatus == STATUS_TRACKING)
            SetStatus(STATUS_TRACKING);
        return;
    }
    // Checks for attackable
    if (bIsMoving && IsInWorld())
    {
        sWorld->SetMove(this, myPosition, GetCurrentPosition(t + 10), 0, true, sWorld->GetArTime(), true);
    }

    AttackInfo Damages[4]{ };
    bool       bDoubleAttack{false};

    Attack(pEnemy, t, GetAttackInterval(), Damages, bDoubleAttack);
    float fMod     = 0.5f;

    switch (m_Base->weapon_type)
    {
        case 1:
            fMod = 0.6f;
            break;
        case 2:
            fMod = 0.7f;
            break;
        case 3:
            fMod = 1.0f;
            break;
        default:
            fMod = 0.5f;
            break;
    }
    m_nMovableTime = (uint)((float)GetAttackInterval() * fMod + t);
    broadcastAttackMessage(pEnemy, Damages, (int)(10 * GetAttackInterval()), (int)(10 * (GetNextAttackableTime() - t)), bDoubleAttack, false, false, false);
    if (pEnemy->IsMoving(t) && nPrevStatus == STATUS_TRACKING)
        SetStatus(STATUS_TRACKING);
}

uint Monster::GetCreatureGroup() const
{
    return m_Base->grp;
}

bool Monster::IsEnvironmentMonster() const
{
    return m_Base->fight_type == 1;
}

bool Monster::IsBattleMode() const
{
    return GetStatus() == 1 || GetStatus() == 3 || GetStatus() == 2;
}

bool Monster::IsBossMonster() const
{
    return m_Base->monster_type >= 22;
}

bool Monster::IsDungeonConnector() const
{
    return GetRace() == 20001;
}

bool Monster::IsAgent() const
{
    return m_Base->fight_type == 4;
}

bool Monster::IsAutoTrap() const
{
    return m_Base->fight_type == 5;
}

bool Monster::IsNonAttacker() const
{
    return m_Base->fight_type == 6 || m_Base->attacK_point == 0;
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

int Monster::GetMonsterGroup() const
{
    return m_Base->monster_group;
}

int Monster::GetTameItemCode() const
{
    int result = m_Base->taming_id;
    if (result != 0)
    {
        auto si = sObjectMgr->GetSummonBase(result);
        if (si != nullptr)
            result = si->card_id;
    }
    return result;
}

int Monster::GetTameCode() const
{
    return m_Base->taming_id;
}

float Monster::GetTamePercentage() const
{
    return m_Base->taming_percentage;
}

int Monster::GetMonsterID() const
{
    return m_Base->id;
}

CreatureStat *Monster::GetBaseStat() const
{
    return sObjectMgr->GetStatInfo(m_Base->stat_id);
}

int Monster::GetRace() const
{
    return m_Base->race;
}

int Monster::AddHate(uint handle, int pt, bool bBroadcast, bool bProcRoamingMonster)
{
    std::vector<uint> vResult{ };

    if (GetHealth() == 0 || IsEnvironmentMonster())
        return 0;
    uint ct   = sWorld->GetArTime();
    auto unit = sMemoryPool->GetObjectInWorld<Unit>(handle);

    if (unit == nullptr/* || IsEnemy(unit, false)*/)
        return 0;

    auto ht = addHate(handle, pt);

    int nHate = ht->nHate;
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

HateTag *Monster::getHateTag(uint handle, uint t)
{
    auto pos = std::find_if(m_vHateList.begin(),
                            m_vHateList.end(),
                            [handle] (const HateTag& ht) { return ht.uid == handle; });

    if(pos != m_vHateList.end())
    {
        pos->nTime = t;
        return &*pos;
    }
    return nullptr;
}

HateTag *Monster::addHate(uint handle, int nHate)
{
    uint ct       = sWorld->GetArTime();
    auto ht       = getHateTag(handle, ct);
    if (ht == nullptr)
    {
        HateTag nht{handle, ct, nHate};
        nht.bIsActive = true;
        m_vHateList.emplace_back(nht);
        ht = &m_vHateList.back();
    }
    ht->nHate += nHate;
    if (ht->nHate < 0)
        ht->nHate = 0;

    ht->nLastMaxHate = ht->nHate;
    if (!ht->bIsActive)
    {
        ht->bIsActive = true;
        int  lvl{0};
        uint tm{0};
        if (ht->nBadAttackCount == 1)
        {
            lvl = 1;
            tm  = ct + 300;
        }
        else
        {
            lvl = 2;
            tm  = ct + 500;
        }
        AddState(StateType::SG_NORMAL, StateCode::SC_FRENZY, GetHandle(), lvl, ct, tm, false, 0, "");
    }

    if (m_hEnemy != 0)
    {
        if (m_hEnemy != handle)
        {
            if (ht->nHate > m_nMaxHate)
            {
                m_nMaxHate = ht->nHate;
                m_hEnemy   = handle;
                return ht;
            }
            if (m_hEnemy != handle)
                return ht;
        }
        m_nMaxHate = ht->nHate;
        return ht;
    }

    m_nMaxHate = ht->nHate;
    if (sMemoryPool->GetObjectInWorld<WorldObject>(handle) != nullptr)
        StartAttack(handle, false);
    return ht;
}

bool Monster::removeFromHateList(uint handle)
{
    bool     found{false};
    for (int i = m_vHateList.size() - 1; i >= 0; --i)
    {
        auto ht = m_vHateList[i];
        if (ht.uid == handle)
        {
            m_vHateList.erase(m_vHateList.begin() + i);
            auto unit = sMemoryPool->GetObjectInWorld<Unit>(handle);
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

void Monster::processWalk(uint t)
{
    ArMoveVector tmp_mv{ };
    tmp_mv.Copy(*this);
    tmp_mv.Step(t);
    int wpi{0};
    if (GetStatus() == STATUS_NORMAL && !m_bComeBackHome && m_pWayPointInfo != nullptr)
    {
        m_pRespawn.m_positionX = tmp_mv.GetPositionX();
        m_pRespawn.m_positionY = tmp_mv.GetPositionY();
        if (m_nWayPointIdx < 0 && m_pWayPointInfo->way_point_type == 1)
            wpi -= tmp_mv.ends.size();
        else
            wpi        = (int)(m_pWayPointInfo->vWayPoint.size() - tmp_mv.ends.size() - 1);
        m_nWayPointIdx = wpi;

    }

    if ((uint)(tmp_mv.GetPositionX() / g_nRegionSize) != (uint)(GetPositionX() / g_nRegionSize)
        || (uint)(tmp_mv.GetPositionY() / g_nRegionSize) != (uint)(GetPositionY() / g_nRegionSize)
        || !tmp_mv.bIsMoving)
    {
        if (!IsInWorld())
            return;

        if (m_bComeBackHome && !IsBattleMode())
        {
            if (tmp_mv.bIsMoving)
            {
                sWorld->onRegionChange(this, t - lastStepTime, tmp_mv.bIsMoving == 0);
                return;
            }
            if (HasFlag(UNIT_FIELD_STATUS, STATUS_INVINCIBLE))
                RemoveFlag(UNIT_FIELD_STATUS, STATUS_INVINCIBLE);
            m_bComeBackHome = false;
        }
        sWorld->onRegionChange(this, t - lastStepTime, !tmp_mv.bIsMoving);
    }
}

void Monster::processMove(uint t)
{
    if (IsDungeonConnector() || GetHealth() == 0)
        return;

    std::vector<Position> vMoveInfo{ };
    Position              targetPos{ };

    if (m_pWayPointInfo == nullptr)
    {
        if (GetStatus() != STATUS_NORMAL || m_bNearClient || !m_vHateList.empty())
        {
            if (m_bIsWandering)
            {
                int rnd = rand32();
                if (GetStatus() == STATUS_NORMAL
                    && GetHealth() != 0
                    && (!bIsMoving || !IsInWorld())
                    && rnd % 500 + lastStepTime + 1000 < t
                    && (rnd % 3) != 0)
                {
                    getMovePosition(targetPos);
                    if (!sObjectMgr->IsBlocked(targetPos.GetPositionX(), targetPos.GetPositionY()))
                    {
                        auto speed = (uint8)(m_Attribute.nMoveSpeed / 7);
                        sWorld->SetMove(this, GetCurrentPosition(t), targetPos, speed, true, sWorld->GetArTime(), true);
                        m_pRespawn.m_positionX = targetPos.GetPositionX();
                        m_pRespawn.m_positionY = targetPos.GetPositionY();
                    }
                }
            }
        }
        else
        {
            sWorld->ClearTamer(this, true);
            SetStatus(STATUS_NORMAL);
        }
    }
    else
    {
        if (!bIsMoving || !IsInWorld())
        {
            int lwpi = (int)m_pWayPointInfo->vWayPoint.size() - 1;
            if (m_nWayPointIdx >= lwpi)
            {
                if (m_pWayPointInfo->way_point_type == 1)
                    m_nWayPointIdx = -lwpi;
                else
                    m_nWayPointIdx = -1;
            }

            if (m_pWayPointInfo->way_point_type == 1 && m_nWayPointIdx < 0)
                lwpi = 0;
            int wpi = m_nWayPointIdx;
            while (true)
            {
                ++wpi;
                if (wpi > lwpi)
                    break;
                int awpi = std::abs(wpi);
                targetPos = m_pWayPointInfo->vWayPoint[awpi];
                vMoveInfo.emplace_back(targetPos);
            }

            t = sWorld->GetArTime();
            auto speed = (uint8)(m_pWayPointInfo->way_point_speed / 7);

            if (speed == 0)
                speed = (uint8)(m_Attribute.nMoveSpeed / 7);

            sWorld->SetMultipleMove(this, GetCurrentPosition(t), vMoveInfo, speed, true, t, true);
        }
    }
}

void Monster::processFirstAttack(uint t)
{
    if (GetStatus() != STATUS_NORMAL || !(IsAgent() || IsFirstAttacker() || IsBattleRevenger()))
        return;

    std::vector<uint>      vResult{ };
    std::vector<Monster *> vSameGroup{ };
    uint                   target{0};

    if (IsAgent())
    {

    }
    else
    {
        sWorld->EnumMovableObject(this->GetPosition(), GetLayer(), GetFirstAttackRange(), vResult, true, true);

        auto            distance = GetFirstAttackRange() + 1.0f;
        for (const auto &handle : vResult)
        {
            auto unit = sMemoryPool->GetObjectInWorld<Unit>(handle);
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
                        target   = handle;
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
    auto  duplicateEnemyPosition = enemyPosition.GetPosition();
    float walk_length            = distance - gap;
    float v{0};
    if (walk_length >= 0.0f)
    {
        v = (walk_length / distance) + 0.1f;
        enemyPosition.m_positionX = ((enemyPosition.GetPositionX() - myPosition.GetPositionX()) * v) + myPosition.GetPositionX();
        enemyPosition.m_positionY = ((enemyPosition.GetPositionY() - myPosition.GetPositionY()) * v) + myPosition.GetPositionY();
        enemyPosition.m_positionZ = ((enemyPosition.GetPositionZ() - myPosition.GetPositionZ()) * v) + myPosition.GetPositionZ();

        if (!sObjectMgr->IsBlocked(enemyPosition.GetPositionX(), enemyPosition.GetPositionY()))
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
    newPos.m_positionX = (((uint)rand32()) % 120) + GetPositionX() - 60.0f;
    newPos.m_positionY = (((uint)rand32()) % 120) + GetPositionY() - 60.0f;

    if (newPos.m_positionX < 0.0f)
        newPos.m_positionX = 0.0f;
    if (newPos.m_positionY < 0.0f)
        newPos.m_positionY = 0.0f;

}

Position Monster::getNonDuplicateAttackPos(Unit *pEnemy)
{
    uint ct     = sWorld->GetArTime();
    auto result = pEnemy->GetCurrentPosition(ct);

    auto  range = GetRealAttackRange() + (GetUnitSize() * 0.5f) + (pEnemy->GetUnitSize() * 0.5f);
    float fMod  = irand(-1, 3);
    if (fMod == 0.0f)
        fMod = 1.0f;

    float angle      = fMod * 0.6283f;
    auto  myPosition = GetCurrentPosition(ct);
    float distance   = myPosition.GetExactDist2d(&result);
    fMod     = 0.0f;
    if (distance != 0.0f)
        fMod = acos((myPosition.GetPositionX() - result.GetPositionX()) / distance);
    if (myPosition.GetPositionY() - result.GetPositionY() < 0.0f)
        fMod = 0.6283f - fMod;
    angle    = fMod + angle;
    fMod     = cos(angle);
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

void Monster::SetTamer(uint handle, int nTamingSkillLevel)
{
    m_hTamer            = handle;
    m_nTamedTime        = sWorld->GetArTime();
    m_nTamingSkillLevel = nTamingSkillLevel;
}

uint Monster::GetTamer() const
{
    return m_hTamer;
}

void Monster::procQuest(Position pos, Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute)
{
    std::vector<Quest *>  vQuestList{ };
    std::vector<Player *> vPlayer{ };

    if (!vPartyContribute.empty())
    {
        if (vPartyContribute.front().hPlayer != 0)
        {
            auto player = sMemoryPool->GetObjectInWorld<Player>(vPartyContribute.front().hPlayer);
            if (player != nullptr)
                vPlayer.emplace_back(player);
        }
        else
        {
            sGroupManager->DoEachMemberTag(vPartyContribute.front().nPartyID, [&vPlayer, pos](PartyMemberTag &tag) {
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

void Monster::procDropChaos(Unit *pKiller, takePriority pPriority, std::vector<VirtualParty> &vPartyContribute, float fDropRatePenalty)
{
    float fChance = m_Base->chaos_drop_percentage * GameRule::GetChaosDropRate() * fDropRatePenalty;
    auto  rnd     = (uint)rand32();
    float chaos   = rnd % 100;

    if (chaos >= fChance)
        return;

    chaos = irand(m_Base->chaos_min[0], m_Base->chaos_max[0]);
    for (auto &vp : vPartyContribute)
    {
        float fSharedChaos = vp.fContribute * chaos;
        if (vp.hPlayer == 0)
        {
            //sWorld->addChaos(this, vp.nPartyID, fSharedChaos);
        }
        else
        {
            auto p = sMemoryPool->GetObjectInWorld<Player>(vp.hPlayer);
            if (p != nullptr)
                sWorld->addChaos(this, p, fSharedChaos);
        }
    }
}

bool Monster::IsAlly(const Unit *pTarget)
{
    return pTarget->IsMonster() || pTarget->IsNPC();
}

void Monster::TriggerForceKill(Player *pPlayer)
{
    pFCClient  = pPlayer;
    bForceKill = true;
}

const std::string &Monster::GetNameAsString()
{
    return sObjectMgr->GetValueFromNameID(m_Base->name_id);
}
