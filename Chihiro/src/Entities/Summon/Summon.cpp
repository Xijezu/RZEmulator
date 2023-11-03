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

#include "Summon.h"

#include "ClientPackets.h"
#include "DatabaseEnv.h"
#include "MemPool.h"
#include "Messages.h"
#include "ObjectMgr.h"
#include "RegionContainer.h"
#include "Skill.h"
#include "World.h"

// static
void Summon::EnterPacket(TS_SC_ENTER &pEnterPct, Summon *pSummon, Player *pPlayer)
{
    TS_SC_ENTER__SUMMON_INFO summonInfo{};
    Unit::EnterPacket(summonInfo.creatureInfo, pSummon, pPlayer);
    summonInfo.master_handle = pSummon->GetMaster()->GetHandle();
    summonInfo.summon_code = pSummon->GetSummonCode();
    summonInfo.szName = pSummon->GetNameAsString();
    pEnterPct.summonInfo = summonInfo;
};

Summon::Summon(uint32_t pHandle, uint32_t pIdx)
    : Unit(true)
{
#ifdef _MSC_VER
#pragma warning(default : 4355)
#endif
    _mainType = MT_NPC; // dont question it :^)
    _subType = ST_Summon;
    _objType = OBJ_MOVABLE;
    _valuesCount = BATTLE_FIELD_END;

    _InitValues();
    _InitTimerFieldsAndStatus();
    SetInt32Value(UNIT_FIELD_HANDLE, pHandle);
    SetSummonInfo(pIdx);
}

Summon *Summon::AllocSummon(Player *pMaster, uint32_t pCode)
{
    Summon *summon = sMemoryPool.AllocSummon(pCode);
    summon->m_pMaster = pMaster;
    summon->CalculateStat();
    summon->SetHealth(summon->GetMaxHealth());
    summon->SetMana(summon->GetMaxMana());
    summon->SetJP(1);

    if (summon->GetRidingInfo() == SUMMON_RIDE_TYPE::RIDING_LENT) {
        summon->SetSkill(static_cast<int32_t>(SKILL_UID_TYPE::SKILL_UID_SUMMON_SKILL), SKILL_UID::SKILL_CREATURE_RIDING, 1, 0);
    }

    return summon;
}

SUMMON_RIDE_TYPE Summon::GetRidingInfo()
{

    return m_tSummonBase != nullptr ? static_cast<SUMMON_RIDE_TYPE>(m_tSummonBase->is_riding_only) : SUMMON_RIDE_TYPE::CANT_RIDING;
}

void Summon::SetSummonInfo(int32_t idx)
{
    m_tSummonBase = sObjectMgr.GetSummonBase(idx);
    if (m_tSummonBase == nullptr)
        ASSERT(false);
    SetCurrentJob(idx);
    this->m_nTransform = m_tSummonBase->form;
}

int32_t Summon::GetSummonCode()
{
    return m_tSummonBase->id;
}

uint32_t Summon::GetCardHandle()
{
    if (m_pItem != nullptr)
        return m_pItem->m_nHandle;
    else
        return 0;
}

void Summon::DB_UpdateSummon(Player * /*pMaster*/, Summon *pSummon)
{
    // PrepareStatement(CHARACTER_UPD_SUMMON, "UPDATE Summon SET account_id = ?, owner_id = ?, code = ?,
    // exp = ?, jp = ?, last_decreased_exp = ?, name = ?, transform = ?, lv = ?, jlv = ?, max_level = ?,
    // prev_level_01 = ?, prev_level_02 = ?, prev_id_01 = ?, prev_id_02 = ?, hp = ?, mp = ? WHERE sid = ?;", CONNECTION_ASYNC);
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_UPD_SUMMON);

    uint8_t i = 0;
    stmt->setInt32(i++, (pSummon->GetMaster() != nullptr ? 0 : pSummon->m_nAccountID)); // account id
    stmt->setInt32(i++, (pSummon->GetMaster() != nullptr ? pSummon->GetMaster()->GetUInt32Value(UNIT_FIELD_UID) : 0));
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
    stmt->setInt32(i++, pSummon->GetSP());
    stmt->setInt32(i++, pSummon->GetHealth());
    stmt->setInt32(i++, pSummon->GetMana());
    stmt->setInt32(i, pSummon->GetUInt32Value(UNIT_FIELD_UID));
    CharacterDatabase.Execute(stmt);
}

void Summon::DB_InsertSummon(Player *pMaster, Summon *pSummon)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_SUMMON);
    stmt->setInt32(0, pSummon->GetUInt32Value(UNIT_FIELD_UID)); // handle
    stmt->setInt32(1, 0); // account_id
    stmt->setInt32(2, pMaster->GetUInt32Value(UNIT_FIELD_UID)); // owner_id
    stmt->setInt64(3, pSummon->GetSummonCode()); // summon_id
    stmt->setInt64(4, pSummon->m_pItem->GetItemInstance().GetUID()); // card_uid
    stmt->setUInt64(5, pSummon->GetEXP());
    stmt->setInt32(6, pSummon->GetJP());
    stmt->setUInt64(7, 0); // Last Decreased EXP
    stmt->setString(8, pSummon->GetName());
    stmt->setInt32(9, pSummon->m_nTransform); // transform
    stmt->setInt32(10, pSummon->GetLevel());
    stmt->setInt32(11, pSummon->GetCurrentJLv());
    stmt->setInt32(12, 1); // max lvl
    stmt->setInt32(13, 0); // fp
    stmt->setInt32(14, 0);
    stmt->setInt32(15, 0);
    stmt->setInt32(16, 0);
    stmt->setInt32(17, 0); // prev_...stuff
    stmt->setInt32(18, pSummon->GetMaxSP()); // sp
    stmt->setInt32(19, pSummon->GetMaxHealth());
    stmt->setInt32(20, pSummon->GetMaxMana());
    CharacterDatabase.Execute(stmt);
}

void Summon::processWalk(uint32_t t)
{
    // Do Ride check here
    ArMoveVector tmp_mv{*dynamic_cast<ArMoveVector *>(this)};
    tmp_mv.Step(t);
    if ((tmp_mv.GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)) != (GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)) ||
        (tmp_mv.GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)) != (GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)) || !tmp_mv.bIsMoving) {
        if (bIsMoving && IsInWorld()) {
            sWorld.onRegionChange(this, t - lastStepTime, !tmp_mv.bIsMoving);
        }
    }
}

void Summon::OnAfterReadSummon() {}

void Summon::onExpChange()
{
    int32_t level = 1;
    int32_t lvl = 0;
    int32_t oblv = 0;
    int32_t jp = 0;
    switch (m_tSummonBase->form) {
    case 1:
        lvl = 50;
        oblv = 60;
        break;
    case 2:
        lvl = 100;
        oblv = 115;
        break;
    case 3:
        lvl = 170;
        oblv = 170;
        break;
    default:
        return;
    }
    if (lvl > 1) {
        do {
            auto need = sObjectMgr.GetNeedSummonExp(level);
            if (need == 0 || need > GetEXP())
                break;
            ++level;
            if (GetLevel() < level) {
                ++jp;
                if (level > oblv) /// @todo add max level reached
                    ++jp;
            }
        } while (level < oblv);
    }
    if (m_pMaster != nullptr)
        Messages::SendEXPMessage(m_pMaster, this);

    if (level != 0 && level != GetLevel()) {
        // Implement Max Summon Level
        // uint64_t uid{0};
        // if (m_pItem != nullptr)
        //     uid = m_pItem->GetItemInstance().GetUID();
        // int32_t ljp{0};
        // if (level <= GetLevel())
        //     ljp = 0;
        // else
        //     ljp = jp;

        int32_t levelchange = level - GetLevel();
        SetCurrentJLv(level);
        SetInt32Value(UNIT_FIELD_LEVEL, level);
        if (levelchange <= 0) {
            CalculateStat();
        }
        else {
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
            }
            else {
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

bool Summon::DoEvolution()
{
    auto prev_hp = GetHealth();
    auto prev_mp = GetMana();

    if (this->m_tSummonBase->form < 3) {
        // @TODO Ride
        if (false) {
            return false;
        }
        else {
            auto nTargetCode = m_tSummonBase->evolve_target;
            SetSummonInfo(nTargetCode);
            CalculateStat();
            m_pMaster->Save(false);

            TS_SC_SUMMON_EVOLUTION evoPct{};
            evoPct.card_handle = m_pItem->GetHandle();
            evoPct.summon_handle = GetHandle();
            evoPct.name = GetName();
            evoPct.code = m_tSummonBase->id;
            if (IsInWorld()) {
                sWorld.Broadcast(
                    (uint32_t)(GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), (uint32_t)(GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), GetLayer(), evoPct);
            }
            else {
                if (m_pMaster != nullptr)
                    m_pMaster->SendPacket(evoPct);
            }

            if (sRegion.IsVisibleRegion(this, GetMaster()) == 0) {
                m_pMaster->SendPacket(evoPct);
            }
            Messages::SendStatInfo(m_pMaster, this);
            Messages::SendHPMPMessage(m_pMaster, this, GetHealth() - prev_hp, GetMana() - prev_mp, false);
            Messages::SendLevelMessage(m_pMaster, this);
            Messages::SendEXPMessage(m_pMaster, this);

            if (m_pItem != nullptr) {
                /*int32_t i = 0;
                for( i = 0; i < m_tSummonBase.form - 1; ++i) {
                    m_pItem->m_Instance.Socket[i+1] = GetPrevJobLv(i);
                }
                m_pItem->m_Instance.Socket[i + 1] = GetLevel();*/
                if (m_pMaster != nullptr)
                    Messages::SendItemMessage(m_pMaster, m_pItem);
            }
            return true;
        }
    }
    return false;
};

bool Summon::TranslateWearPosition(ItemWearType &pos, Item *pItem, std::vector<int32_t> *ItemList)
{
    if (!Unit::TranslateWearPosition(pos, pItem, ItemList))
        return false;

    if ((pItem->GetItemInstance().GetFlag() & FlagBits::ITEM_FLAG_CARD) == 0)
        return false;

    if ((pos) > MAX_ITEM_WEAR || (pos) < 0)
        return false;

    if (pos == (MAX_ITEM_WEAR)) {
        int32_t startPos{0};
        int32_t endPos{static_cast<int32_t>(SUMMON_DEFAULTS::SUMMON_MAX_NON_ARTIFACT_ITEM_WEAR)};

        ///- Later epics: Accessory + Artifact slots, ain't gonna implement that yet

        for (int32_t i = startPos; i < endPos; ++i) {
            auto pWornItem = m_anWear[i];
            if (!pItem->IsArtifact() && pWornItem != nullptr && pWornItem->GetItemGroup() == pItem->GetItemGroup())
                return false;

            if (pWornItem == pItem)
                return false;

            if (pWornItem != nullptr && (pos) == MAX_ITEM_WEAR)
                pos = static_cast<ItemWearType>(i);
        }

        if ((pos) == MAX_ITEM_WEAR)
            return false;
    }
    else if ((pos) >= static_cast<int32_t>(SUMMON_DEFAULTS::SUMMON_MAX_NON_ARTIFACT_ITEM_WEAR)) ///- Replace with m_nMaxItemWear w/ staged pets
    {
        return false;
    }
    else if (ItemList != nullptr) {
        if (m_anWear[(pos)] != nullptr)
            ItemList->emplace_back(pos);
    }

    return true;
}

CreatureStat *Summon::GetBaseStat() const
{
    return sObjectMgr.GetStatInfo((uint32_t)m_tSummonBase->stat_id);
}

Summon::~Summon()
{
    if (IsInWorld()) {
        sWorld.RemoveObjectFromWorld(this);
    }
}

void Summon::onRegisterSkill(int64_t skillUID, int32_t skill_id, int32_t prev_level, int32_t skill_level)
{
    Skill::DB_InsertSkill(this, skillUID, skill_id, skill_level, GetRemainCoolTime(skill_id));
    Messages::SendSkillList(GetMaster(), this, skill_id);
}

void Summon::Update(uint32_t /*diff*/)
{
    if (!IsInWorld())
        return;

    uint32_t ct = sWorld.GetArTime();
    if (GetHealth() == 0) {
        if (GetUInt32Value(UNIT_FIELD_DEAD_TIME) + 6000 < ct) {
            GetMaster()->DoUnSummon(this);
        }
        return;
    }

    if (bIsMoving && IsInWorld()) {
        processWalk(ct);
        return;
    }
    if (GetTargetHandle() != 0 || m_castingSkill != nullptr) {
        onAttackAndSkillProcess();
        return;
    }

    if (HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_PENDED)) {
        processPendingMove();
    }
}

uint16_t Summon::putonItem(ItemWearType pos, Item *pItem)
{
    auto pMaster = GetMaster();
    if (pMaster == nullptr || !pMaster->IsInWorld() || (pItem->GetItemInstance().GetFlag() & FlagBits::ITEM_FLAG_CARD) == 0)
        return TS_RESULT_ACCESS_DENIED;

    if (auto nRet = Unit::putonItem(pos, pItem); nRet != TS_RESULT_SUCCESS)
        return nRet;

    pItem->SetOwnSummonInfo(GetHandle(), GetUInt32Value(UNIT_FIELD_UID));

    ///- @todo: Item on_equip_item
    if (m_anWear[(pos)] == pItem)
        pMaster->GetInventory()->AddWeightModifier(-pItem->GetWeight());

    pMaster->UpdateWeightWithInventory();
    pMaster->UpdateQuestStatusByItemUpgrade();

    return TS_RESULT_SUCCESS;
}

uint16_t Summon::putoffItem(ItemWearType pos)
{
    auto pMaster = GetMaster();
    if (pMaster == nullptr || !pMaster->IsInWorld())
        return TS_RESULT_ACCESS_DENIED;

    if (m_anWear[(pos)] == nullptr)
        return TS_RESULT_NOT_EXIST;

    auto pItem = m_anWear[(pos)];
    pItem->SetOwnSummonInfo(0, 0);
    auto nRet = Unit::putoffItem(pos);

    if (m_anWear[(pos)] == nullptr)
        pMaster->GetInventory()->AddWeightModifier(pItem->GetWeight());

    pMaster->UpdateWeightWithInventory();
    pMaster->UpdateQuestStatusByItemUpgrade();

    return nRet;
}

void Summon::onCompleteCalculateStat()
{

    m_Attribute.nAttackRange = m_tSummonBase->attack_range;
}

void Summon::onModifyStatAndAttribute()
{
    if (GetMaster() != nullptr) {
        Messages::SendStatInfo(GetMaster(), this);
    }
}

void Summon::onItemWearEffect(Item *pItem, bool bIsBaseVar, int32_t type, float var1, float var2, float fRatio)
{
    if (bIsBaseVar) {
        switch (type) {
        case IEP_ATTACK_POINT:
            var1 = var1 * m_fBaseAttackPointRatio;
            var2 = var2 * m_fBaseAttackPointRatio;
            break;
        case IEP_MAGIC_POINT:
            var1 = var1 * m_fBaseMagicPointRatio;
            var2 = var2 * m_fBaseMagicPointRatio;
            break;
        case IEP_DEFENCE:
            var1 = var1 * m_fBaseDefenceRatio;
            var2 = var2 * m_fBaseDefenceRatio;
            break;
        case IEP_MAGIC_DEFENCE:
            var1 = var1 * m_fBaseMagicDefenceRatio;
            var2 = var2 * m_fBaseMagicDefenceRatio;
            break;
        default:
            break;
        }
    }

    Unit::onItemWearEffect(pItem, bIsBaseVar, type, var1, var2, fRatio);
}

void Summon::applyJobLevelBonus()
{
    auto stat = sObjectMgr.GetSummonLevelBonus(m_tSummonBase->id, m_tSummonBase->form, GetLevel());
    m_cStat.strength += stat.strength;
    m_cStat.vital += stat.vital;
    m_cStat.agility += stat.agility;
    m_cStat.dexterity += stat.dexterity;
    m_cStat.intelligence += stat.intelligence;
    m_cStat.luck += stat.luck;
    m_cStat.mentality += stat.mentality;
}

float Summon::GetFCM() const
{
    Skill *skill{nullptr};
    if (GetMaster() != nullptr && (skill = GetMaster()->GetSkill(SKILL_CREATURE_MASTERY)) != nullptr) {
        return skill->m_nSkillLevel * 0.03f + 0.69999999f;
    }
    return 0.69999999f;
}

void Summon::onBeforeCalculateStat()
{
    m_fBaseAttackPointRatio = 0;
    m_fBaseMagicPointRatio = 0;
    m_fBaseDefenceRatio = 0;
    m_fBaseMagicDefenceRatio = 0;

    auto pItemExpert = GetSkillByEffectType(EF_SUMMON_ITEM_EXPERT);
    if (pItemExpert) {
        m_fBaseAttackPointRatio = pItemExpert->GetVar(0);
        m_fBaseMagicPointRatio = pItemExpert->GetVar(1);
        m_fBaseDefenceRatio = pItemExpert->GetVar(2);
        m_fBaseMagicDefenceRatio = pItemExpert->GetVar(3);
    }

    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_SPEED_FIXED))
        m_Attribute.nMoveSpeed += m_tSummonBase->run_speed - GameRule::GetBaseMoveSpeed();
}

float Summon::GetSize() const
{
    return m_tSummonBase->size;
}

float Summon::GetScale() const
{
    return m_tSummonBase->scale;
}

void Summon::onCantAttack(uint32_t handle, uint32_t t)
{
    if (GetUInt32Value(UNIT_LAST_CANT_ATTACK_TIME) + 100 < t) {
        SetUInt32Value(UNIT_LAST_CANT_ATTACK_TIME, t);
        Messages::SendCantAttackMessage(GetMaster(), GetHandle(), handle, TS_RESULT_TOO_FAR);
    }
}

bool Summon::IsAlly(const Unit *pTarget)
{
    return GetMaster()->IsAlly(pTarget);
}

void Summon::onAfterRemoveState(State *state, bool)
{
    SetFlag(UNIT_FIELD_STATUS, STATUS_NEED_TO_CALCULATE_STAT);
    Unit::onAfterRemoveState(state);
}

void Summon::onUpdateState(State *state, bool bIsExpire)
{
    if (GetMaster() != nullptr && GetMaster()->IsInWorld())
        Messages::SendStateMessage(GetMaster(), GetHandle(), state, bIsExpire);

    Unit::onUpdateState(state, bIsExpire);
}

struct applyPassiveSkillEffectFunctor : SkillFunctor {
    applyPassiveSkillEffectFunctor(Summon *pSummon)
        : m_pSummon(pSummon)
    {
    }

    void onSkill(const Skill *pSkill) override
    {
        switch (pSkill->GetSkillBase()->GetSkillEffectType()) {
        case EF_INCREASE_SUMMON_HP_MP_SP: {
            int32_t nMaxHPInc = pSkill->GetVar(0) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(1);
            int32_t nMaxMPInc = pSkill->GetVar(2) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(3);
            // int32_t nMaxSPInc = pSkill->GetVar(4) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(5);
            int32_t nHPRegenInc = pSkill->GetVar(6) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(7);
            int32_t nMPRegenInc = pSkill->GetVar(8) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(9);
            // int32_t nSPRegenInc = pSkill->GetVar(10) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(11);

            m_pSummon->SetMaxHealth(m_pSummon->GetMaxHealth() + nMaxHPInc);
            m_pSummon->SetMaxMana(m_pSummon->GetMaxMana() + nMaxMPInc);
            m_pSummon->GetAttribute().nHPRegenPoint += nHPRegenInc;
            m_pSummon->GetAttribute().nMPRegenPoint += nMPRegenInc;
        } break;
        default:
            break;
        }
    }

    Summon *m_pSummon{nullptr};
};

void Summon::applyPassiveSkillEffect()
{
    Unit::applyPassiveSkillEffect();

    if (GetMaster() != nullptr) {
        applyPassiveSkillEffectFunctor fn(this);
        GetMaster()->EnumSummonPassiveSkill(fn);
    }
}

struct applyPassiveSkillAmplifyEffectFunctor : SkillFunctor {
    applyPassiveSkillAmplifyEffectFunctor(Summon *pSummon)
        : m_pSummon(pSummon)
    {
    }

    void onSkill(const Skill *pSkill) override
    {
        switch (pSkill->GetSkillBase()->GetSkillEffectType()) {
        case EF_AMPLIFY_SUMMON_HP_MP_SP: {
            float fMaxHPInc = pSkill->GetVar(0) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(1);
            float fMaxMPInc = pSkill->GetVar(2) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(3);
            // float fMaxSPInc = pSkill->GetVar(4) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(5);
            float fHPRegenInc = pSkill->GetVar(6) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(7);
            float fMPRegenInc = pSkill->GetVar(8) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(9);
            // float fSPRegenInc = pSkill->GetVar(10) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(11);

            m_pSummon->SetFloatValue(UNIT_FIELD_MAX_HEALTH_MODIFIER, m_pSummon->GetFloatValue(UNIT_FIELD_MAX_HEALTH_MODIFIER) + fMaxHPInc);
            m_pSummon->SetFloatValue(UNIT_FIELD_MAX_MANA_MODIFIER, m_pSummon->GetFloatValue(UNIT_FIELD_MAX_MANA_MODIFIER) + fMaxMPInc);
            m_pSummon->SetFloatValue(UNIT_FIELD_HP_REGEN_MOD, m_pSummon->GetFloatValue(UNIT_FIELD_HP_REGEN_MOD) + fHPRegenInc);
            m_pSummon->SetFloatValue(UNIT_FIELD_MP_REGEN_MOD, m_pSummon->GetFloatValue(UNIT_FIELD_MP_REGEN_MOD) + fMPRegenInc);
        } break;
        default:
            break;
        }
    }

    Summon *m_pSummon{nullptr};
};

void Summon::applyPassiveSkillAmplifyEffect()
{
    Unit::applyPassiveSkillAmplifyEffect();

    if (GetMaster() != nullptr) {
        applyPassiveSkillAmplifyEffectFunctor fn(this);
        GetMaster()->EnumSummonAmplifySkill(fn);
    }
}

void Summon::applyState(State &state)
{
    //@Todo: SP
}

void Summon::applyStatByState()
{
    Unit::applyStatByState();
    if (GetMaster() == nullptr)
        return;
}

struct onApplyStatFunctor : SkillFunctor {
    onApplyStatFunctor(Summon *pSummon)
        : m_pSummon(pSummon)
    {
    }

    void onSkill(const Skill *pSkill) override
    {
        if (pSkill->GetSkillBase()->GetSkillEffectType() == EF_INCREASE_STAT) {
            int32_t nSkillLevel = pSkill->GetCurrentSkillLevel();

            m_pSummon->GetCreatureStat().strength += pSkill->GetVar(0) + pSkill->GetVar(1) * nSkillLevel;
            m_pSummon->GetCreatureStat().vital += pSkill->GetVar(2) + pSkill->GetVar(3) * nSkillLevel;
            m_pSummon->GetCreatureStat().agility += pSkill->GetVar(4) + pSkill->GetVar(5) * nSkillLevel;
            m_pSummon->GetCreatureStat().dexterity += pSkill->GetVar(6) + pSkill->GetVar(7) * nSkillLevel;
            m_pSummon->GetCreatureStat().intelligence += pSkill->GetVar(8) + pSkill->GetVar(9) * nSkillLevel;
            m_pSummon->GetCreatureStat().mentality += pSkill->GetVar(10) + pSkill->GetVar(11) * nSkillLevel;
            m_pSummon->GetCreatureStat().luck += pSkill->GetVar(12) + pSkill->GetVar(13) * nSkillLevel;
        }
        else if (pSkill->GetSkillBase()->GetSkillEffectType() == EF_AMPLIFY_STAT) {
            int32_t nSkillLevel = pSkill->GetCurrentSkillLevel();

            m_pSummon->GetCreatureStatAmplifier().strength += pSkill->GetVar(0) * nSkillLevel;
            m_pSummon->GetCreatureStatAmplifier().vital += pSkill->GetVar(1) * nSkillLevel;
            m_pSummon->GetCreatureStatAmplifier().agility += pSkill->GetVar(2) * nSkillLevel;
            m_pSummon->GetCreatureStatAmplifier().dexterity += pSkill->GetVar(3) * nSkillLevel;
            m_pSummon->GetCreatureStatAmplifier().intelligence += pSkill->GetVar(4) * nSkillLevel;
            m_pSummon->GetCreatureStatAmplifier().mentality += pSkill->GetVar(5) * nSkillLevel;
            m_pSummon->GetCreatureStatAmplifier().luck += pSkill->GetVar(6) * nSkillLevel;
        }
    }

    Summon *m_pSummon{nullptr};
};

void Summon::onApplyStat()
{
    onApplyStatFunctor fn(this);
    EnumPassiveSkill(fn);
}

void Summon::onSPChange()
{
    if (GetMaster() != nullptr)
        Messages::SendSPMessage(GetMaster(), this);
}