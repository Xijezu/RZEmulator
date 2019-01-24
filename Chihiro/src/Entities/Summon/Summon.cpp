/*
 *  Copyright (C) 2017-2019 NGemity <https://ngemity.org/>
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
#include "RegionContainer.h"
#include "Skill.h"

// static
void Summon::EnterPacket(XPacket &pEnterPct, Summon *pSummon, Player *pPlayer)
{
    Unit::EnterPacket(pEnterPct, pSummon, pPlayer);
    pEnterPct << pSummon->m_pMaster->GetHandle();
    Messages::GetEncodedInt(pEnterPct, pSummon->GetSummonCode());
    pEnterPct.fill(pSummon->GetName(), 19);
};

Summon::Summon(uint pHandle, uint pIdx) : Unit(true)
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

Summon *Summon::AllocSummon(Player *pMaster, uint pCode)
{
    Summon *summon = sMemoryPool.AllocSummon(pCode);
    summon->m_pMaster = pMaster;
    return summon;
}

void Summon::SetSummonInfo(int idx)
{
    m_tSummonBase = sObjectMgr.GetSummonBase(idx);
    if (m_tSummonBase == nullptr)
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
    stmt->setInt32(i++, pSummon->GetHealth());
    stmt->setInt32(i++, pSummon->GetMana());
    stmt->setInt32(i, pSummon->GetUInt32Value(UNIT_FIELD_UID));
    CharacterDatabase.Execute(stmt);
}

void Summon::DB_InsertSummon(Player *pMaster, Summon *pSummon)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_SUMMON);
    stmt->setInt32(0, pSummon->GetUInt32Value(UNIT_FIELD_UID)); // handle
    stmt->setInt32(1, 0);                                       // account_id
    stmt->setInt32(2, pMaster->GetUInt32Value(UNIT_FIELD_UID)); // owner_id
    stmt->setInt64(3, pSummon->GetSummonCode());                // summon_id
    stmt->setInt64(4, pSummon->m_pItem->m_Instance.UID);        // card_uid
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
    stmt->setInt32(18, 0); // sp
    stmt->setInt32(19, pSummon->GetMaxHealth());
    stmt->setInt32(20, pSummon->GetMaxMana());
    CharacterDatabase.Execute(stmt);
}

void Summon::processWalk(uint t)
{
    // Do Ride check here
    ArMoveVector tmp_mv{*dynamic_cast<ArMoveVector *>(this)};
    tmp_mv.Step(t);
    if ((tmp_mv.GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)) != (GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)) ||
        (tmp_mv.GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)) != (GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)) ||
        !tmp_mv.bIsMoving)
    {
        if (bIsMoving && IsInWorld())
        {
            sWorld.onRegionChange(this, t - lastStepTime, !tmp_mv.bIsMoving);
        }
    }
}

void Summon::OnAfterReadSummon()
{
}

void Summon::onExpChange()
{
    int level = 1;
    int lvl = 0;
    int oblv = 0;
    int jp = 0;
    switch (m_tSummonBase->form)
    {
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
    if (lvl > 1)
    {
        do
        {
            auto need = sObjectMgr.GetNeedSummonExp(level);
            if (need == 0 || need > GetEXP())
                break;
            ++level;
            if (GetLevel() < level)
            {
                ++jp;
                if (level > oblv) /// @todo add max level reached
                    ++jp;
            }
        } while (level < oblv);
    }
    if (m_pMaster != nullptr)
        Messages::SendEXPMessage(m_pMaster, this);

    if (level != 0)
    {
        if (level != GetLevel())
        {
            uint64_t uid{0};
            if (m_pItem != nullptr)
                uid = m_pItem->m_Instance.UID;
            int ljp{0};
            if (level <= GetLevel())
                ljp = 0;
            else
                ljp = jp;

            int levelchange = level - GetLevel();
            SetCurrentJLv(level);
            SetInt32Value(UNIT_FIELD_LEVEL, level);
            if (levelchange <= 0)
            {
                CalculateStat();
            }
            else
            {
                auto old_hp = GetHealth();
                auto old_mp = GetMana();
                SetJP(GetJP() + jp);
                CalculateStat();
                if (GetHealth() != 0)
                {
                    SetHealth(GetMaxHealth());
                    SetMana(GetMaxMana());
                }
                if (IsInWorld())
                {
                    Messages::BroadcastHPMPMessage(this, GetHealth() - old_hp, GetMana() - old_mp, false);
                }
                else
                {
                    if (m_pMaster != nullptr)
                    {
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

    if (this->m_tSummonBase->form < 3)
    {
        // @TODO Ride
        if (false)
        {
            return false;
        }
        else
        {
            auto nTargetCode = m_tSummonBase->evolve_target;
            SetSummonInfo(nTargetCode);
            CalculateStat();
            m_pMaster->Save(false);

            TS_SC_SUMMON_EVOLUTION evoPct{};
            evoPct.card_handle = m_pItem->GetHandle();
            evoPct.summon_handle = GetHandle();
            evoPct.name = GetName();
            evoPct.code = m_tSummonBase->id;
            if (IsInWorld())
            {
                sWorld.Broadcast((uint)(GetPositionX() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), (uint)(GetPositionY() / sWorld.getIntConfig(CONFIG_MAP_REGION_SIZE)), GetLayer(), evoPct);
            }
            else
            {
                if (m_pMaster != nullptr)
                    m_pMaster->SendPacket(evoPct);
            }

            if (sRegion.IsVisibleRegion(this, GetMaster()) == 0)
            {
                m_pMaster->SendPacket(evoPct);
            }
            Messages::SendStatInfo(m_pMaster, this);
            Messages::SendHPMPMessage(m_pMaster, this, GetHealth() - prev_hp, GetMana() - prev_mp, false);
            Messages::SendLevelMessage(m_pMaster, this);
            Messages::SendEXPMessage(m_pMaster, this);

            if (m_pItem != nullptr)
            {
                /*int i = 0;
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

bool Summon::TranslateWearPosition(ItemWearType &pos, Item *pItem, std::vector<int> *ItemList)
{
    if (!Unit::TranslateWearPosition(pos, pItem, ItemList))
        return false;

    if ((pItem->m_Instance.Flag & 1) == 0)
        return false;

    if (pos > static_cast<ItemWearType>(MAX_ITEM_WEAR) || pos < 0)
        return false;

    if (pos == static_cast<ItemWearType>(MAX_ITEM_WEAR))
    {
        int32_t startPos{0};
        int32_t endPos{static_cast<int32_t>(SUMMON_DEFAULTS::SUMMON_MAX_NON_ARTIFACT_ITEM_WEAR)};

        ///- Later epics: Accessory + Artifact slots, ain't gonna implement that yet

        for (int i = startPos; i < endPos; ++i)
        {
            auto pWornItem = m_anWear[i];
            if (!pItem->IsArtifact() && pWornItem != nullptr && pWornItem->GetItemGroup() == pItem->GetItemGroup())
                return false;

            if (pWornItem == pItem)
                return false;

            if (pWornItem != nullptr && pos == MAX_ITEM_WEAR)
                pos = static_cast<ItemWearType>(i);
        }

        if (pos == MAX_ITEM_WEAR)
            return false;
    }
    else if (pos >= static_cast<int32_t>(SUMMON_DEFAULTS::SUMMON_MAX_NON_ARTIFACT_ITEM_WEAR)) ///- Replace with m_nMaxItemWear w/ staged pets
    {
        return false;
    }
    else if (ItemList != nullptr)
    {
        if (m_anWear[pos] != nullptr)
            ItemList->emplace_back(pos);
    }

    return true;
}

CreatureStat *Summon::GetBaseStat() const
{
    return sObjectMgr.GetStatInfo((uint)m_tSummonBase->stat_id);
}

Summon::~Summon()
{
    if (IsInWorld())
    {
        sWorld.RemoveObjectFromWorld(this);
    }
}

void Summon::onRegisterSkill(int64_t skillUID, int skill_id, int prev_level, int skill_level)
{
    Skill::DB_InsertSkill(this, skillUID, skill_id, skill_level, GetRemainCoolTime(skill_id));
    Messages::SendSkillList(GetMaster(), this, skill_id);
}

void Summon::Update(uint /*diff*/)
{
    if (!IsInWorld())
        return;

    uint ct = sWorld.GetArTime();
    if (GetHealth() == 0)
    {
        if (GetUInt32Value(UNIT_FIELD_DEAD_TIME) + 6000 < ct)
        {
            GetMaster()->DoUnSummon(this);
        }
        return;
    }

    if (bIsMoving && IsInWorld())
    {
        processWalk(ct);
        return;
    }
    if (GetTargetHandle() != 0 || m_castingSkill != nullptr)
    {
        onAttackAndSkillProcess();
        return;
    }

    if (HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_PENDED))
    {
        processPendingMove();
    }
}

uint16_t Summon::putonItem(ItemWearType pos, Item *pItem)
{
    auto pMaster = GetMaster();
    if (pMaster == nullptr || !pMaster->IsInWorld() || (pItem->m_Instance.Flag & 1) == 0)
        return TS_RESULT_ACCESS_DENIED;

    if (auto nRet = Unit::putonItem(pos, pItem); nRet != TS_RESULT_SUCCESS)
        return nRet;

    pItem->SetOwnSummonInfo(GetHandle(), GetUInt32Value(UNIT_FIELD_UID));

    ///- @todo: Item on_equip_item
    if (m_anWear[pos] == pItem)
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

    if (m_anWear[pos] == nullptr)
        return TS_RESULT_NOT_EXIST;

    auto pItem = m_anWear[pos];
    pItem->SetOwnSummonInfo(0, 0);
    auto nRet = Unit::putoffItem(pos);

    if (m_anWear[pos] == nullptr)
        pMaster->GetInventory()->AddWeightModifier(pItem->GetWeight());

    pMaster->UpdateWeightWithInventory();
    pMaster->UpdateQuestStatusByItemUpgrade();

    return nRet;
}

void Summon::onCompleteCalculateStat()
{
    /* todo: riding mount*/
    m_Attribute.nAttackRange = m_tSummonBase->attack_range;
}

void Summon::onModifyStatAndAttribute()
{
    if (GetMaster() != nullptr)
    {
        Messages::SendStatInfo(GetMaster(), this);
    }
}

void Summon::onItemWearEffect(Item *pItem, bool bIsBaseVar, int type, float var1, float var2, float fRatio)
{
    /*
   StructSummon *v7; // esi@1
  c_fixed<10000> *v8; // eax@6
  double v9; // st7@6
  unsigned int v10; // edi@9
  unsigned int v11; // ebx@9
  float n; // ST1C_4@9
  c_fixed<10000> *v13; // eax@9
  unsigned int v14; // ecx@9
  unsigned int v15; // eax@9
  c_fixed<10000> result; // [sp+10h] [bp-8h]@6

  v7 = this;
  if ( !bIsBaseVar )
    goto LABEL_15;
  if ( type == 11 )
  {
    v8 = c_fixed<10000>::operator*<float>(&var1, &result, this->m_fBaseAttackPointRatio);
    v9 = v7->m_fBaseAttackPointRatio;
    goto LABEL_9;
  }
  if ( type == 12 )
  {
    v8 = c_fixed<10000>::operator*<float>(&var1, &result, this->m_fBaseMagicPointRatio);
    v9 = v7->m_fBaseMagicPointRatio;
    goto LABEL_9;
  }
  if ( type == 15 )
  {
    v8 = c_fixed<10000>::operator*<float>(&var1, &result, this->m_fBaseDefenceRatio);
    v9 = v7->m_fBaseDefenceRatio;
    goto LABEL_9;
  }
  if ( type != 16 )
  {
LABEL_15:
    v15 = HIDWORD(var2.value);
    v14 = var2.value;
    v10 = HIDWORD(var1.value);
    v11 = var1.value;
    goto LABEL_12;
  }
  v8 = c_fixed<10000>::operator*<float>(&var1, &result, this->m_fBaseMagicDefenceRatio);
  v9 = v7->m_fBaseMagicDefenceRatio;
LABEL_9:
  v10 = HIDWORD(v8->value);
  v11 = v8->value;
  n = v9;
  v13 = c_fixed<10000>::operator*<float>(&var2, &var1, n);
  v14 = v13->value;
  v15 = HIDWORD(v13->value);
LABEL_12:
  StructCreature::onItemWearEffect(
    (StructCreature *)&v7->vfptr,
    pItem,
    bIsBaseVar,
    type,
    (c_fixed<10000>)__PAIR__(v10, v11),
    (c_fixed<10000>)__PAIR__(v15, v14),
    fRatio);
*/
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
    if (GetMaster() != nullptr && (skill = GetMaster()->GetSkill(SKILL_CREATURE_MASTERY)) != nullptr)
    {
        return skill->m_nSkillLevel * 0.03f + 0.69999999f;
    }
    return 0.69999999f;
}

void Summon::onBeforeCalculateStat()
{
    // @todo Epic > 4: Item Mastery (EffectType 10046)
    // @todo: if ( StructState::GetEffectType((StructState *)v6) == 112 ) -> EF_PHYSICAL_MULTIPLE_REGION_DAMAGE_OLD ???

    if (!HasFlag(UNIT_FIELD_STATUS, STATUS_MOVE_SPEED_FIXED))
        m_Attribute.nMoveSpeed += m_tSummonBase->run_speed - 120;
}

float Summon::GetSize() const
{
    return m_tSummonBase->size;
}

float Summon::GetScale() const
{
    return m_tSummonBase->scale;
}

void Summon::onCantAttack(uint handle, uint t)
{
    if (GetUInt32Value(UNIT_LAST_CANT_ATTACK_TIME) + 100 < t)
    {
        SetUInt32Value(UNIT_LAST_CANT_ATTACK_TIME, t);
        Messages::SendCantAttackMessage(GetMaster(), GetHandle(), handle, TS_RESULT_TOO_FAR);
    }
}

bool Summon::IsAlly(const Unit *pTarget)
{
    return GetMaster()->IsAlly(pTarget);
}

void Summon::onAfterRemoveState(State *state)
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

struct applyPassiveSkillEffectFunctor : SkillFunctor
{
    applyPassiveSkillEffectFunctor(Summon *pSummon) : m_pSummon(pSummon)
    {
    }

    void onSkill(const Skill *pSkill) override
    {
        switch (pSkill->GetSkillBase()->GetSkillEffectType())
        {
        case EF_INCREASE_SUMMON_HP_MP_SP:
        {
            int nMaxHPInc = pSkill->GetVar(0) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(1);
            int nMaxMPInc = pSkill->GetVar(2) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(3);
            //int nMaxSPInc = pSkill->GetVar(4) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(5);
            int nHPRegenInc = pSkill->GetVar(6) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(7);
            int nMPRegenInc = pSkill->GetVar(8) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(9);
            //int nSPRegenInc = pSkill->GetVar(10) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(11);

            m_pSummon->SetMaxHealth(m_pSummon->GetMaxHealth() + nMaxHPInc);
            m_pSummon->SetMaxMana(m_pSummon->GetMaxMana() + nMaxMPInc);
            m_pSummon->GetAttribute().nHPRegenPoint += nHPRegenInc;
            m_pSummon->GetAttribute().nMPRegenPoint += nMPRegenInc;
        }
        break;
        default:
            break;
        }
    }

    Summon *m_pSummon{nullptr};
};

void Summon::applyPassiveSkillEffect()
{
    Unit::applyPassiveSkillEffect();

    if (GetMaster() != nullptr)
    {
        applyPassiveSkillEffectFunctor fn(this);
        GetMaster()->EnumSummonPassiveSkill(fn);
    }
}

struct applyPassiveSkillAmplifyEffectFunctor : SkillFunctor
{
    applyPassiveSkillAmplifyEffectFunctor(Summon *pSummon) : m_pSummon(pSummon)
    {
    }

    void onSkill(const Skill *pSkill) override
    {
        switch (pSkill->GetSkillBase()->GetSkillEffectType())
        {
        case EF_AMPLIFY_SUMMON_HP_MP_SP:
        {
            float fMaxHPInc = pSkill->GetVar(0) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(1);
            float fMaxMPInc = pSkill->GetVar(2) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(3);
            //float fMaxSPInc = pSkill->GetVar(4) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(5);
            float fHPRegenInc = pSkill->GetVar(6) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(7);
            float fMPRegenInc = pSkill->GetVar(8) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(9);
            //float fSPRegenInc = pSkill->GetVar(10) + pSkill->GetCurrentSkillLevel() * pSkill->GetVar(11);

            m_pSummon->SetFloatValue(UNIT_FIELD_MAX_HEALTH_MODIFIER, m_pSummon->GetFloatValue(UNIT_FIELD_MAX_HEALTH_MODIFIER) + fMaxHPInc);
            m_pSummon->SetFloatValue(UNIT_FIELD_MAX_MANA_MODIFIER, m_pSummon->GetFloatValue(UNIT_FIELD_MAX_MANA_MODIFIER) + fMaxMPInc);
            m_pSummon->SetFloatValue(UNIT_FIELD_HP_REGEN_MOD, m_pSummon->GetFloatValue(UNIT_FIELD_HP_REGEN_MOD) + fHPRegenInc);
            m_pSummon->SetFloatValue(UNIT_FIELD_MP_REGEN_MOD, m_pSummon->GetFloatValue(UNIT_FIELD_MP_REGEN_MOD) + fMPRegenInc);
        }
        break;
        default:
            break;
        }
    }

    Summon *m_pSummon{nullptr};
};

void Summon::applyPassiveSkillAmplifyEffect()
{
    Unit::applyPassiveSkillAmplifyEffect();

    if (GetMaster() != nullptr)
    {
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

struct onApplyStatFunctor : SkillFunctor
{
    onApplyStatFunctor(Summon *pSummon) : m_pSummon(pSummon)
    {
    }

    void onSkill(const Skill *pSkill) override
    {
        if (pSkill->GetSkillBase()->GetSkillEffectType() == EF_INCREASE_STAT)
        {
            int nSkillLevel = pSkill->GetCurrentSkillLevel();

            m_pSummon->GetCreatureStat().strength += pSkill->GetVar(0) + pSkill->GetVar(1) * nSkillLevel;
            m_pSummon->GetCreatureStat().vital += pSkill->GetVar(2) + pSkill->GetVar(3) * nSkillLevel;
            m_pSummon->GetCreatureStat().agility += pSkill->GetVar(4) + pSkill->GetVar(5) * nSkillLevel;
            m_pSummon->GetCreatureStat().dexterity += pSkill->GetVar(6) + pSkill->GetVar(7) * nSkillLevel;
            m_pSummon->GetCreatureStat().intelligence += pSkill->GetVar(8) + pSkill->GetVar(9) * nSkillLevel;
            m_pSummon->GetCreatureStat().mentality += pSkill->GetVar(10) + pSkill->GetVar(11) * nSkillLevel;
            m_pSummon->GetCreatureStat().luck += pSkill->GetVar(12) + pSkill->GetVar(13) * nSkillLevel;
        }
        else if (pSkill->GetSkillBase()->GetSkillEffectType() == EF_AMPLIFY_STAT)
        {
            int nSkillLevel = pSkill->GetCurrentSkillLevel();

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