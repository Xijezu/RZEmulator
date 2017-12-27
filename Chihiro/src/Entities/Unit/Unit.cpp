#include "Unit/Unit.h"
#include "ObjectMgr.h"
#include "World.h"
#include "GameRule.h"
#include "Messages.h"
#include "ClientPackets.h"
#include "Skill.h"
// we can disable this warning for this since it only
// causes undefined behavior when passed to the base class constructor
#ifdef _MSC_VER
#pragma warning(disable:4355)
#endif

Unit::Unit(bool isWorldObject) : WorldObject(isWorldObject), m_unitTypeMask(0)
{
#ifdef _MSC_VER
#pragma warning(default:4355)
#endif
    _mainType = MT_StaticObject;
    _objType = OBJ_STATIC;
    _subType = ST_Object;
    _bIsInWorld = false;
}

Unit::~Unit()
{
    for(auto sk : m_vSkillList) {
        //delete sk;
    }
    m_vSkillList.clear();
}

void Unit::AddToWorld()
{
    if (!IsInWorld()) {
        WorldObject::AddToWorld();
    }
}

void Unit::RemoveFromWorld()
{
    // cleanup
            ASSERT(GetHandle());

    if (IsInWorld())
        WorldObject::RemoveFromWorld();
}

void Unit::CleanupsBeforeDelete(bool finalCleanup)
{
}

void Unit::Update(uint32 p_time)
{
    // WARNING! Order of execution here is important, do not change.
    _Events.Update(p_time);

    if (!IsInWorld())
        return;
}

bool Unit::UpdatePosition(float x, float y, float z, float orientation, bool teleport)
{
    return true;
}

void Unit::EnterPacket(XPacket &pEnterPct, Unit *pUnit)
{
    pEnterPct << (uint32_t)0;// pUnit->GetUInt32Value(UNIT_FIELD_STATUS); // TODO: status
    pEnterPct << pUnit->GetOrientation();
    pEnterPct << (int32_t) pUnit->GetHealth();
    pEnterPct << (int32_t) pUnit->GetMaxHealth();
    pEnterPct << (int32_t) pUnit->GetMana();
    pEnterPct << (int32_t) pUnit->GetMaxMana();
    pEnterPct << (int32_t) pUnit->getLevel();
    pEnterPct << (uint8_t) 0;pUnit->GetUInt32Value(UNIT_FIELD_RACE);
    pEnterPct << (uint32_t) 0;pUnit->GetUInt32Value(UNIT_FIELD_SKIN_COLOR);
    pEnterPct << (uint8_t) (pUnit->HasFlag(UNIT_FIELD_STATUS, StatusFlags::FirstEnter) == 1 ? 1 : 0);
    pEnterPct << (int32_t) 0;
}

void Unit::setDeathState(DeathState s)
{
    if (s != ALIVE && s != JUST_RESPAWNED) {

    }

    if (s == JUST_DIED) {

        if (IsInWorld()) {

        }
        // without this when removing IncreaseMaxHealth aura player may stuck with 1 hp
        // do not why since in IncreaseMaxHealth currenthealth is checked
        SetHealth(0);
    }

    _deathState = s;
}

Item *Unit::GetWornItem(ItemWearType idx)
{
    if ((uint) idx >= Item::MAX_ITEM_WEAR || idx < 0)
        return nullptr;

    return m_anWear[idx];
}

void Unit::SetHealth(uint32 hp)
{
    int old_hp = GetHealth();
    SetUInt32Value(UNIT_FIELD_HEALTH, hp);
    if (hp > GetMaxHealth())
        SetUInt32Value(UNIT_FIELD_HEALTH, GetMaxHealth());
    //if (old_hp != GetHealth())
        //this.onHPChange(old_hp);
}

void Unit::SetMana(uint32 mp)
{
    int old_np = GetMana();
    SetUInt32Value(UNIT_FIELD_MANA, mp);
    if (mp > GetMaxMana())
        SetUInt32Value(UNIT_FIELD_MANA, GetMaxMana());
}

///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void Unit::incParameter(uint nBitset, float nValue, bool bStat)
{
    if (bStat) {
        if ((nBitset & 1) != 0)
            m_cStat.strength += nValue;
        if ((nBitset & 2) != 0)
            m_cStat.vital += nValue;
        if ((nBitset & 4) != 0)
            m_cStat.agility += nValue;
        if ((nBitset & 8) != 0)
            m_cStat.dexterity += nValue;
        if ((nBitset & 0x10) != 0)
            m_cStat.intelligence += nValue;
        if ((nBitset & 0x20) != 0)
            m_cStat.mentality += nValue;
        if ((nBitset & 0x40) != 0)
            m_cStat.luck += nValue;
    } else {
        if ((nBitset & 0x80) != 0) {
            m_cAtribute.nAttackPointRight += nValue;
            /*this.m_nAttackPointRightWithoutWeapon += (short)nValue;
            if ((this.m_StatusFlag & StatusFlags.UsingDoubleWeapon) != 0)
            {
                m_cAtribute.nAttackPointLeft += nValue;
                this.m_nAttackPointLeftWithoutWeapon += (short)nValue;
            }*/
        }
        if ((nBitset & 0x100) != 0)
            m_cAtribute.nMagicPoint += nValue;
        if ((nBitset & 0x200) != 0)
            m_cAtribute.nDefence += nValue;
        if ((nBitset & 0x400) != 0)
            m_cAtribute.nMagicDefence += nValue;
        if ((nBitset & 0x800) != 0) {
            m_cAtribute.nAttackSpeedRight += nValue;
            /*if ((this.m_StatusFlag & StatusFlags.UsingDoubleWeapon) != 0)
                m_cAtribute.nAttackSpeedLeft += nValue;*/
        }
        if ((nBitset & 0x1000) != 0)
            m_cAtribute.nCastingSpeed += nValue;
        /*if ((nBitset & 0x2000) != 0 && (this.m_StatusFlag & StatusFlags.MoveSpeedFixed) == 0)
        {
            Player p = this as Player;
            if (p != null || p.m_nRidingStateUid == 0)
                m_cAtribute.nMoveSpeed += nValue;
        }*/
        if ((nBitset & 0x4000) != 0) {
            m_cAtribute.nAccuracyRight += nValue;
            //if ((this.m_StatusFlag & StatusFlags.UsingDoubleWeapon) != 0)
            //    m_cAtribute.nAccuracyLeft += nValue;
        }
        if ((nBitset & 0x8000) != 0)
            m_cAtribute.nMagicAccuracy += nValue;
        if ((nBitset & 0x10000) != 0)
            m_cAtribute.nCritical += (short) nValue;
        if ((nBitset & 0x20000) != 0)
            m_cAtribute.nBlockChance += nValue;
        if ((nBitset & 0x40000) != 0)
            m_cAtribute.nBlockDefence += nValue;
        if ((nBitset & 0x80000) != 0)
            m_cAtribute.nAvoid += nValue;
        if ((nBitset & 0x100000) != 0)
            m_cAtribute.nMagicAvoid += nValue;
        if ((nBitset & 0x200000) != 0)
            SetInt32Value(UNIT_FIELD_MAX_HEALTH, GetInt32Value(UNIT_FIELD_MAX_HEALTH) + (int) nValue);
        if ((nBitset & 0x400000) != 0)
            SetInt32Value(UNIT_FIELD_MAX_MANA, GetInt32Value(UNIT_FIELD_MAX_MANA) + (int) nValue);
        if ((nBitset & 0x1000000) != 0)
            m_cAtribute.nHPRegenPoint += nValue;
        if ((nBitset & 0x2000000) != 0)
            m_cAtribute.nMPRegenPoint += nValue;
        if ((nBitset & 0x8000000) != 0)
            m_cAtribute.nHPRegenPercentage += nValue;
        if ((nBitset & 0x10000000) != 0)
            m_cAtribute.nMPRegenPercentage += nValue;
        if ((nBitset & 0x40000000) != 0)
            m_cAtribute.nMaxWeight += nValue;
    }
}

void Unit::incParameter2(uint nBitset, float fValue)
{
    /*
    if ((nBitset & 1) != 0)
    {
        this.m_Resist.nResist[0] += (short)fValue;
    }
    if ((nBitset & 2) != 0)
    {
        this.m_Resist.nResist[1] += (short)fValue;
    }
    if ((nBitset & 4) != 0)
    {
        this.m_Resist.nResist[2] += (short)fValue;
    }
    if ((nBitset & 8) != 0)
    {
        this.m_Resist.nResist[3] += (short)fValue;
    }
    if ((nBitset & 0x10) != 0)
    {
        this.m_Resist.nResist[4] += (short)fValue;
    }
    if ((nBitset & 0x20) != 0)
    {
        this.m_Resist.nResist[5] += (short)fValue;
    }
    if ((nBitset & 0x40) != 0)
    {
        this.m_Resist.nResist[6] += (short)fValue;
    }
    if ((nBitset & 0x200000) != 0)
    {
        this.m_vNormalAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeNone, fValue));
        this.m_vRangeAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeNone, fValue));
    }
    if ((nBitset & 0x400000) != 0)
    {
        this.m_vNormalAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeFire, fValue));
        this.m_vRangeAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeFire, fValue));
    }
    if ((nBitset & 0x800000) != 0)
    {
        this.m_vNormalAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeWater, fValue));
        this.m_vRangeAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeWater, fValue));
    }
    if ((nBitset & 0x1000000) != 0)
    {
        this.m_vNormalAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeWind, fValue));
        this.m_vRangeAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeWind, fValue));
    }
    if ((nBitset & 0x2000000) != 0)
    {
        this.m_vNormalAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeEarth, fValue));
        this.m_vRangeAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeEarth, fValue));
    }
    if ((nBitset & 0x4000000) != 0)
    {
        this.m_vNormalAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeLight, fValue));
        this.m_vRangeAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeLight, fValue));
    }
    if ((nBitset & 0x8000000) != 0)
    {
        this.m_vNormalAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeDark, fValue));
        this.m_vRangeAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeDark, fValue));
    }
    if ((nBitset & 0x10000000) != 0)
        this.m_Attribute.nCriticalPower += fValue;
    if ((nBitset & 0x20000000) != 0)
        this.m_StatusFlag |= StatusFlags.HPRegenStopped;
    if ((nBitset & 0x40000000) != 0)
        this.m_StatusFlag |= StatusFlags.MPRegenStopped;*/
}

void Unit::CalculateStat()
{
    CreatureAtributeServer stateAttr{ };
    CreatureStat           stateStat{ };
    int                    prev_max_hp = GetMaxHealth();
    int                    prev_max_mp = GetMaxMana();
    int                    prev_hp     = GetHealth();
    int                    prev_mp     = GetMana();

    SetFloatValue(UNIT_FIELD_HP_REGEN_MOD, 1.0f);
    SetFloatValue(UNIT_FIELD_MP_REGEN_MOD, 1.0f);

    SetFloatValue(UNIT_FIELD_MAX_HEALTH_MODIFIER, 0.0f);
    SetFloatValue(UNIT_FIELD_MAX_MANA_MODIFIER, 0.0f);

    SetFloatValue(UNIT_FIELD_HEAL_RATIO, 1.0f);
    SetFloatValue(UNIT_FIELD_MP_HEAL_RATIO, 1.0f);

    SetFloatValue(UNIT_FIELD_HEAL_RATIO_BY_ITEM, 1.0f);
    SetFloatValue(UNIT_FIELD_MP_HEAL_RATIO_BY_ITEM, 1.0f);

    SetFloatValue(UNIT_FIELD_HEAL_RATIO_BY_REST, 1.0f);
    SetFloatValue(UNIT_FIELD_MP_HEAL_RATIO_BY_REST, 2.0f);

    SetInt32Value(UNIT_FIELD_MAX_HEALTH, 1);
    SetInt32Value(UNIT_FIELD_MAX_MANA, 1);

    SetInt32Value(UNIT_FIELD_ADDITIONAL_HEAL, 0);
    SetInt32Value(UNIT_FIELD_ADDITIONAL_MP_HEAL, 0);

    m_cStatByState.Reset(0);
    m_StatAmplifier.Reset(0.0f);
    m_cAtributeByState.Reset(0);
    //m_AttributeAplifier.Reset(0);
    m_cAtribute.Reset(0);
    //m_Resist.Reset(0);
    m_ResistAmplifier.Reset(0.0f);

    CreatureStat basestat = sObjectMgr->GetStatInfo(GetInt32Value(UNIT_FIELD_JOB));
    m_cStat.Copy(basestat);
    // TODO onBeforeCalculateStat(); -> Reset in Player
    // TODO checkAdditionalItemEffect(); -> Nonexistant
    applyStatByItem();
    applyJobLevelBonus();
    stateStat.Copy(basestat);
    /*m_cStatByState.strength += (m_cStat.strength - stateStat.strength);
    m_cStatByState.vital += (m_cStat.vital - stateStat.vital);
    m_cStatByState.dexterity += (m_cStat.dexterity - stateStat.dexterity);
    m_cStatByState.agility += (m_cStat.agility - stateStat.agility);
    m_cStatByState.intelligence += (m_cStat.intelligence - stateStat.intelligence);
    m_cStatByState.mentality += (m_cStat.mentality - stateStat.mentality);
    m_cStatByState.luck += (m_cStat.luck - stateStat.luck);*/
    // TODO onApplyStat -> Beltslots
    // TODO amplifyStatByItem -> Nonexistant
    stateStat.Copy(m_cStat);
    getAmplifiedStatByAmplifier(stateStat);
    // TODO amplifyStatByState();
    getAmplifiedStatByAmplifier(m_cStat);
    m_cStatByState.strength += (m_cStat.strength - stateStat.strength);
    m_cStatByState.vital += (m_cStat.vital - stateStat.vital);
    m_cStatByState.dexterity += (m_cStat.dexterity - stateStat.dexterity);
    m_cStatByState.agility += (m_cStat.agility - stateStat.agility);
    m_cStatByState.intelligence += (m_cStat.intelligence - stateStat.intelligence);
    m_cStatByState.mentality += (m_cStat.mentality - stateStat.mentality);
    m_cStatByState.luck += (m_cStat.luck - stateStat.luck);
    // TODO onAfterApplyStat -> Nonexistant
    float b1  = getLevel();
    float fcm = 1.0f;
    SetMaxHealth((uint32_t)(GetMaxHealth() + (m_cStat.vital * 33.0f) + (b1 * 20.0f)));
    SetMaxMana((uint32_t)(GetMaxMana() + (m_cStat.intelligence * 33.0f) + (b1 * 20.0f)));
    calcAttribute(m_cAtribute);
    // TODO onAfterCalcAttrivuteByStat -> Nonexistant
    applyItemEffect();
    // TODO ApplyPassiveSkillEffect
    // TODO applyStateEffect
    // TODO applyPassiveSkillAmplifyEffect
    // TODO this.onApplyAttributeAdjustment
    // TODO this.getAmplifiedAttributeByAmplifier(stateAttr);
    // TODO this.applyStateAmplifyEffect();
    // TODO this.getAmplifiedAttributeByAmplifier(m_Attribute);
    // TODO this.applyDoubleWeaponEffect();
    SetMaxHealth((uint32_t) (GetInt32Value(UNIT_FIELD_MAX_HEALTH_MODIFIER) + 1.0f) * GetMaxHealth());
    SetMaxMana((uint32_t)(GetInt32Value(UNIT_FIELD_MAX_MANA_MODIFIER) + 1.0f) * GetMaxMana());
    // TODO this.getAmplifiedResistByAmplifier(m_Resist);
    auto hp = GetMaxHealth();
    auto mp = GetMaxMana();
    // TODO this.onCompleteCalculateStat();
    SetHealth(GetHealth());
    SetMana(GetMana());
    if(IsInWorld() && (prev_max_hp != GetMaxHealth() || prev_max_mp != GetMaxMana() || prev_hp != GetHealth() || prev_mp != GetMana()))
    {
        Messages::BroadcastHPMPMessage(this, GetHealth() - prev_hp, GetMana() - prev_mp, false);
    } else {
        if(GetSubType() == ST_Summon && !IsInWorld() && (prev_max_hp != GetMaxHealth() || prev_max_mp != GetMaxMana() || prev_hp != GetHealth() || prev_mp != GetMana()))
        {
            auto s = dynamic_cast<Summon*>(this);
            //if(s != nullptr)
                //Messages::SendHPMPMessage()
        }
    }
}

void Unit::applyItemEffect()
{
    Item *curItem = GetWornItem(ItemWearType::WearWeapon);
    if (curItem != nullptr)
        m_cAtribute.nAttackRange = sObjectMgr->GetItemBase(curItem->m_nItemID).range;

    for (int i = 0; i < 24; i++) {
        curItem = GetWornItem((ItemWearType) i);
        if (curItem != nullptr) {
            if (true) { // TODO TranslateWearPosition
                float        fItemRatio = 1.0f;
                // TODO fItemRatio = 0.4f
                ItemTemplate ib         = sObjectMgr->GetItemBase(curItem->m_Instance.Code);

                for (int ol = 0; ol < Item::MAX_OPTION_NUMBER; ol++) {
                    if (ib.base_type[ol] != 0) {
                        onItemWearEffect(*curItem, true, ib.base_type[ol], ib.base_var[ol][0], ib.base_var[ol][1], fItemRatio);
                    }
                }

                for (int ol = 0; ol < Item::MAX_OPTION_NUMBER; ol++) {
                    if (ib.opt_var[ol] != 0) {
                        onItemWearEffect(*curItem, false, ib.opt_type[ol], ib.opt_var[ol][0], ib.opt_var[ol][1], fItemRatio);
                    }
                }

                float fAddPoint    = 0.0f;
                float fTotalPoints = 0.0f;

                for (int ol = 0; ol < 2; ol++) {
                    if (ib.enhance_id[ol] != 0) {
                        int curEnhance  = curItem->m_Instance.nEnhance;
                        int realEnhance = curEnhance;

                        if (realEnhance >= 1) {
                            fTotalPoints = ib._enhance[0][ol] * curEnhance;

                            if (realEnhance > 4) {
                                fTotalPoints += (ib._enhance[1][ol] * (float) (realEnhance - 4));
                            }
                            if (realEnhance > 8) {
                                fTotalPoints += (ib._enhance[2][ol] * (float) (realEnhance - 8));
                            }
                            if (realEnhance > 12) {
                                fTotalPoints += (ib._enhance[3][ol] * (float) (realEnhance - 12));
                            }
                            onItemWearEffect(*curItem, false, ib.enhance_id[ol], fTotalPoints, fTotalPoints, fItemRatio);
                        }
                    }
                }
            }
        }
    }
}

void Unit::ampParameter2(uint nBitset, float fValue)
{

    if ((nBitset & 1) != 0) {
        m_ResistAmplifier.fResist[0] += fValue;
    }
    if ((nBitset & 2) != 0) {
        m_ResistAmplifier.fResist[1] += fValue;
    }
    if ((nBitset & 4) != 0) {
        m_ResistAmplifier.fResist[2] += fValue;
    }
    if ((nBitset & 8) != 0) {
        m_ResistAmplifier.fResist[3] += fValue;
    }
    if ((nBitset & 0x10) != 0) {
        m_ResistAmplifier.fResist[4] += fValue;
    }
    if ((nBitset & 0x20) != 0) {
        m_ResistAmplifier.fResist[5] += fValue;
    }
    if ((nBitset & 0x40) != 0) {
        m_ResistAmplifier.fResist[6] += fValue;
    }
    /*if ((nBitset & 0x200000) != 0)
    {
        this.m_vNormalAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeNone, fValue));
        this.m_vRangeAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeNone, fValue));
    }
    if ((nBitset & 0x400000) != 0)
    {
        this.m_vNormalAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeFire, fValue));
        this.m_vRangeAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeFire, fValue));
    }
    if ((nBitset & 0x800000) != 0)
    {
        this.m_vNormalAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeWater, fValue));
        this.m_vRangeAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeWater, fValue));
    }
    if ((nBitset & 0x1000000) != 0)
    {
        this.m_vNormalAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeWind, fValue));
        this.m_vRangeAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeWind, fValue));
    }
    if ((nBitset & 0x2000000) != 0)
    {
        this.m_vNormalAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeEarth, fValue));
        this.m_vRangeAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeEarth, fValue));
    }
    if ((nBitset & 0x4000000) != 0)
    {
        this.m_vNormalAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeLight, fValue));
        this.m_vRangeAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeLight, fValue));
    }
    if ((nBitset & 0x8000000) != 0)
    {
        this.m_vNormalAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeDark, fValue));
        this.m_vRangeAdditionalDamage.Add(new AdditionalDamageInfo(100, Elemental.Type.TypeNone, Elemental.Type.TypeDark, fValue));
    }*/
    if ((nBitset & 0x10000000) != 0) {
        m_AttributeAmplifier.fCriticalPower += fValue;
    }/*
    if ((nBitset & 0x20000000) != 0)
        this.m_StatusFlag |= StatusFlags.HPRegenStopped;
    if ((nBitset & 0x40000000) != 0)
        this.m_StatusFlag |= StatusFlags.MPRegenStopped;
    */
}

void Unit::ampParameter(uint nBitset, float fValue, bool bStat)
{

    if (bStat) {
        if ((nBitset & 1) != 0) {
            m_StatAmplifier.strength += fValue;
        }
        if ((nBitset & 2) != 0) {
            m_StatAmplifier.vital += fValue;
        }
        if ((nBitset & 4) != 0) {
            m_StatAmplifier.agility += fValue;
        }
        if ((nBitset & 8) != 0) {
            m_StatAmplifier.dexterity += fValue;
        }
        if ((nBitset & 0x10) != 0) {
            m_StatAmplifier.intelligence += fValue;
        }
        if ((nBitset & 0x20) != 0) {
            m_StatAmplifier.mentality += fValue;
        }
        if ((nBitset & 0x40) != 0) {
            m_StatAmplifier.luck += fValue;
        }
    } else {
        //Player p = this as Player;
        if ((nBitset & 0x80) != 0) {
            m_AttributeAmplifier.fAttackPointRight += fValue;
            /*if ((this.m_StatusFlag & StatusFlags.UsingDoubleWeapon) != 0)
            {*/
            m_AttributeAmplifier.fAttackPointLeft += fValue;
            //}
        }
        if ((nBitset & 0x100) != 0) {
            m_AttributeAmplifier.fMagicPoint += fValue;
        }
        if ((nBitset & 0x200) != 0) {
            m_AttributeAmplifier.fDefence += fValue;
        }
        if ((nBitset & 0x400) != 0) {
            m_AttributeAmplifier.fMagicDefence += fValue;
        }
        if ((nBitset & 0x800) != 0) {
            m_AttributeAmplifier.fAttackSpeedRight += fValue;
            /*if ((m_StatusFlag & StatusFlags.UsingDoubleWeapon) != 0)
            {*/
            m_AttributeAmplifier.fAttackSpeedLeft += fValue;
            //}
        }
        if ((nBitset & 0x1000) != 0) {
            m_AttributeAmplifier.fCastingSpeed += fValue;
        }
        if ((nBitset & 0x2000) != 0) {
            /*if (p == null || p.m_nRidingStateUid == 0)
            {
                this.m_AttributeAmplifier.fMoveSpeed += fValue;
            }*/
        }
        if ((nBitset & 0x4000) != 0) {
            m_AttributeAmplifier.fAccuracyRight += fValue;
            /*if ((m_StatusFlag & StatusFlags.UsingDoubleWeapon) != 0)
            {*/
            m_AttributeAmplifier.fAccuracyLeft += fValue;
            //}
        }
        if ((nBitset & 0x8000) != 0) {
            m_AttributeAmplifier.fMagicAccuracy += fValue;
        }
        if ((nBitset & 0x10000) != 0) {
            m_AttributeAmplifier.fCritical += fValue;
        }
        if ((nBitset & 0x20000) != 0) {
            m_AttributeAmplifier.fBlockChance += fValue;
        }
        if ((nBitset & 0x40000) != 0) {
            m_AttributeAmplifier.fBlockDefence += fValue;
        }
        if ((nBitset & 0x100000) != 0) {
            m_AttributeAmplifier.fMagicAvoid += fValue;
        }
        /*if ((nBitset & 0x200000) != 0)
            m_fMaxHPAmplifier += fValue;

        if ((nBitset & 0x400000) != 0)
            m_fMaxMPAmplifier += fValue;*/
        if ((nBitset & 0x1000000) != 0) {
            m_AttributeAmplifier.fHPRegenPoint += fValue;
        }
        if ((nBitset & 0x2000000) != 0) {
            m_AttributeAmplifier.fMPRegenPoint += fValue;
        }
        if ((nBitset & 0x8000000) != 0) {
            m_AttributeAmplifier.fHPRegenPercentage += fValue;
        }
        if ((nBitset & 0x10000000) != 0) {
            m_AttributeAmplifier.fMPRegenPercentage += fValue;
        }
        if ((nBitset & 0x40000000) != 0) {
            m_AttributeAmplifier.fMaxWeight += fValue;
        }
    }
}

void Unit::onItemWearEffect(Item pItem, bool bIsBaseVar, int type, float var1, float var2, float fRatio)
{
    float        result;
    float        item_var_penalty;
    ItemTemplate tpl = sObjectMgr->GetItemBase(pItem.m_nItemID);

    if (type == 14)
        item_var_penalty = var1;
    else
        item_var_penalty = var1 * fRatio;

    if (type != 14) {
        if (pItem.m_nItemID != 0 && bIsBaseVar) {
            item_var_penalty += (float) (var2 * (float) tpl.level);
            result           = var1;
            item_var_penalty = GameRule::GetItemValue(item_var_penalty, (int) var1, getLevel(), tpl.rank, pItem.m_Instance.nLevel);
        }
        // TODO m_fItemMod = item_var_penalty
    }

    if (type > 96) {
        int t1 = type - 97;
        if (t1 != 0) {
            int t2 = t1 - 1;
            if (t2 != 0) {
                if (t2 == 1)
                    ampParameter2((uint) var1, var2);
            } else {
                ampParameter((uint) var1, var2, false);
            }
        } else {
            incParameter2((uint) var1, var2);
        }
    } else {
        if (type == 96) {
            incParameter((uint) var1, (int) var2, false);
        } else {
            switch (type) {
                case 12:
                    m_cAtribute.nMagicPoint += item_var_penalty;
                    return;
                case 11:
                    /*if ((this.m_StatusFlag & StatusFlags.UsingDoubleWeapon) == 0)
                    {
                        this.m_Attribute.nAttackPointRight += item_var_penalty;
                    }
                    else if (pItem == null || pItem.m_pItemBase.nGroup != 1)
                    {
                        this.m_Attribute.nAttackPointLeft += item_var_penalty;
                        this.m_Attribute.nAttackPointRight += item_var_penalty;
                    }
                    else if (pItem.m_Instance.nWearInfo != ItemBase.ItemWearType.WearShield)
                    {
                        this.m_Attribute.nAttackPointRight += item_var_penalty;
                    }
                    else
                    {
                        this.m_Attribute.nAttackPointLeft += item_var_penalty;
                    }*/
                    return;
                case 21:
                    m_cAtribute.nBlockDefence += item_var_penalty;
                    return;
                case 15:
                    m_cAtribute.nDefence += item_var_penalty;
                    return;
                case 13:
                    /*if ((this.m_StatusFlag & StatusFlags.UsingDoubleWeapon) == 0)
                    {
                        this.m_Attribute.nAccuracyRight += item_var_penalty;
                    }
                    else if (pItem == null || pItem.m_pItemBase.nGroup != 1)
                    {
                        this.m_Attribute.nAccuracyLeft += item_var_penalty;
                        this.m_Attribute.nAccuracyRight += item_var_penalty;
                    }
                    else if (pItem.m_Instance.nWearInfo != ItemBase.ItemWearType.WearShield)
                    {
                        this.m_Attribute.nAccuracyRight += item_var_penalty;
                    }
                    else
                    {
                        this.m_Attribute.nAccuracyLeft += item_var_penalty;
                    }*/
                    return;
                case 14:
                    /*if ((this.m_StatusFlag & StatusFlags.UsingDoubleWeapon) == 0)
                    {
                        this.m_Attribute.nAttackSpeedRight += item_var_penalty;
                    }
                    else if (pItem == null || pItem.m_pItemBase.nGroup != 1)
                    {
                        this.m_Attribute.nAttackSpeedRight += item_var_penalty;
                        this.m_Attribute.nAttackSpeedLeft += item_var_penalty;
                    }
                    else if ( pItem.m_Instance.nWearInfo != ItemBase.ItemWearType.WearShield)
                    {
                        this.m_Attribute.nAttackSpeedRight += item_var_penalty;
                    }
                    else
                    {
                        this.m_Attribute.nAttackSpeedLeft += item_var_penalty;
                    }*/
                    return;
                case 16:
                    m_cAtribute.nMagicDefence += item_var_penalty;
                    return;
                case 17:
                    m_cAtribute.nAvoid += item_var_penalty;
                    return;
                case 18:
                    if (true) // TODO if ((this.m_StatusFlag & StatusFlags.MoveSpeedFixed) == 0 && (p == null || p.m_nRidingStateUid == 0))
                        m_cAtribute.nMoveSpeed += item_var_penalty;
                    return;
                case 19:
                    m_cAtribute.nBlockChance += item_var_penalty;
                    return;
                case 20:
                    m_cAtribute.nMaxWeight += item_var_penalty;
                    return;
                case 22:
                    m_cAtribute.nCastingSpeed += item_var_penalty;
                    return;
                case 23:
                    m_cAtribute.nMagicAccuracy += item_var_penalty;
                    break;
                case 24:
                    m_cAtribute.nMagicAvoid += item_var_penalty;
                    break;
                case 25:
                    m_cAtribute.nCoolTimeSpeed += item_var_penalty;
                    break;
                case 30:
                    SetMaxHealth(GetMaxHealth() + (short) item_var_penalty);
                    break;
                case 31:
                    SetInt32Value(UNIT_FIELD_MAX_MANA, GetMaxMana() + (short) item_var_penalty);;
                    break;
                case 33:
                    m_cAtribute.nMPRegenPoint += item_var_penalty;
                    break;
                case 34:
                    // m_fBowInterval = var1;
                    break;
                default:
                    return;
            }
        }
    }
}

void Unit::calcAttribute(CreatureAtributeServer &attribute)
{
    float v;
    attribute.nCriticalPower = 80;
    attribute.nCritical += ((m_cStat.luck * 0.2f) + 3.0f);

    float b1                   = getLevel();
    float fcm                  = 1.0f;
    float d1                   = (fcm * 5.0f);

    if (false) {
        // isUsingBow | IsUsingCrossBow
    } else {
        v = (2.8f * m_cStat.strength) + (fcm * b1);
        attribute.nAttackPointRight += v;
        //if ((this.m_StatusFlag & StatusFlags.UsingDoubleWeapon) != 0)
        if (false) {
            attribute.nAttackPointLeft += v;// ((2 * this.m_Stat.strength) * fcm + bl);
        }
    }

    attribute.nMagicPoint += ((2 * m_cStat.intelligence) + (fcm * b1));
    attribute.nItemChance += (m_cStat.luck * 0.2f);
    attribute.nDefence += ((1.6f * m_cStat.vital) + (fcm * b1));
    attribute.nMagicDefence += ((2 * m_cStat.mentality) + (fcm + b1));
    attribute.nMaxWeight += 10 * (getLevel() + m_cStat.strength);
    attribute.nAccuracyRight += ((m_cStat.dexterity) * 0.5f + (fcm + b1));
    // TODO Add left accu
    attribute.nMagicAccuracy += ((m_cStat.mentality * 0.4f + m_cStat.dexterity * 0.1f) * fcm + b1);
    attribute.nAvoid += (m_cStat.agility * 0.5f * fcm + b1);
    attribute.nMagicAvoid += (m_cStat.mentality * 0.5f * fcm + b1);
    attribute.nAttackSpeedRight += (100 + (m_cStat.agility * 0.1f));
    // TODO Add left attackspeed
    attribute.nMoveSpeed += (120 * fcm);
    attribute.nCastingSpeed += (100 * fcm);
    attribute.nCoolTimeSpeed   = 100;

    attribute.nHPRegenPercentage += d1;
    attribute.nHPRegenPoint += ((2 * fcm * b1) + 48.0f);
    attribute.nMPRegenPercentage += d1;
    attribute.nMPRegenPoint += ((2 * fcm * b1) + 48.0f + (4.1f * m_cStat.mentality));
    if (attribute.nAttackRange == 0)
        attribute.nAttackRange = 50;
}

void Unit::finalizeStat()
{
    if (m_cStat.strength < 0)
        m_cStat.strength = 0;

    if (m_cStat.vital < 0)
        m_cStat.vital = 0;

    if (m_cStat.dexterity < 0)
        m_cStat.dexterity = 0;

    if (m_cStat.agility < 0)
        m_cStat.agility = 0;

    if (m_cStat.intelligence < 0)
        m_cStat.intelligence = 0;

    if (m_cStat.mentality < 0)
        m_cStat.mentality = 0;

    if (m_cStat.luck < 0)
        m_cStat.luck = 0;
}

void Unit::getAmplifiedStatByAmplifier(CreatureStat &stat)
{
    stat.strength     = ((m_StatAmplifier.strength * stat.strength) + stat.strength);
    stat.vital        = ((m_StatAmplifier.vital * stat.vital) + stat.vital);
    stat.dexterity    = ((m_StatAmplifier.dexterity * stat.dexterity) + stat.dexterity);
    stat.agility      = ((m_StatAmplifier.agility * stat.agility) + stat.agility);
    stat.intelligence = ((m_StatAmplifier.intelligence * stat.intelligence) + stat.intelligence);
    stat.mentality    = ((m_StatAmplifier.mentality * stat.mentality) + stat.mentality);
    stat.luck         = ((m_StatAmplifier.luck * stat.luck) + stat.luck);
}

void Unit::applyStatByItem()
{
    std::vector<int> ref_list{ };

    for (int i = 0; i < 24; i++) {
        auto item = m_anWear[i];
        if (item != nullptr) {
            if (item->m_Instance.nWearInfo != ItemWearType::WearNone) {
                if (true) { // TODO TranslateWearPosition
                    for (int j = 0; j < 4; j++) {
                        short ot = item->m_pItemBase.opt_type[j];
                        uint  bs = (uint32_t) item->m_pItemBase.opt_var[0][j];
                        int   fv = (int32_t) item->m_pItemBase.opt_var[1][j];
                        if (ot == 96)
                            incParameter(bs, fv, true);
                    }
                }
            }
        }
    }
}

void Unit::CleanupBeforeRemoveFromMap(bool finalCleanup)
{

}

void Unit::SetMultipleMove(std::vector<Position> _to, uint8_t _speed, uint _start_time)
{
    ArMoveVector::SetMultipleMove(_to, _speed, _start_time, sWorld->GetArTime());
    lastStepTime = start_time;
}

void Unit::processPendingMove()
{
    uint     ct = sWorld->GetArTime();
    Position pos{ };//             = ArPosition ptr -10h


    if (HasFlag(UNIT_FIELD_STATUS, MovePending)) {
        if (m_nMovableTime < ct) {
            RemoveFlag(UNIT_FIELD_STATUS, MovePending);
            //if (this.IsActable() && this.IsMovable())
            if (true) {
                pos = GetCurrentPosition(ct);
                sWorld->SetMultipleMove(this, pos, m_PendingMovePos, m_nPendingMoveSpeed, true, ct, true);
                if (GetSubType() == ST_Player) {
                    //auto p = dynamic_cast<Player*>(this);
                    /*if (p.m_nRideIdx != 0)
                    {
                        Summon ro = p.GetRideObject();
                        RappelzServer.Instance.SetMultipleMove(ro, pos, this.m_PendingMovePos, (sbyte)this.m_nPendingMoveSpeed, true, ct, true);
                    }*/
                }
            }
        }
    }
}

void Unit::OnUpdate()
{
    uint ct = sWorld->GetArTime();
    if(HasFlag(UNIT_FIELD_STATUS, NeedToCalculateStat)) {
        CalculateStat();
        RemoveFlag(UNIT_FIELD_STATUS, NeedToCalculateStat);
    }
    this->regenHPMP(ct);
}

void Unit::regenHPMP(uint t)
{
    float prev_mp;
    int prev_hp;
    float pt;

    uint et = t - m_nLastUpdatedTime;
    if (et >= 300) {
        float etf = (float) et / 6000.0f;
        //prev_mp = et;

        if (GetHealth() != 0) {
            prev_mp = GetHealth();
            prev_hp = GetMana();
            if (!HasFlag(UNIT_FIELD_STATUS, HPRegenStopped)) {
                pt     = GetMaxHealth() * m_cAtribute.nHPRegenPercentage;
                pt     = pt * 0.01f * etf;// + 0.0;
                pt     = pt + m_cAtribute.nHPRegenPoint * etf;
                /*if (this.IsSitDown()) {
                    pt *= this.m_fHealRatioByRest;
                    pt += (float) this.m_nAdditionalHealByRest;
                }*/
                if (pt < 1.0f)
                    pt = 1.0f;
                pt *= GetFloatValue(UNIT_FIELD_HP_REGEN_MOD);
                int pti = static_cast<int>(pt);
                if (pti != 0.0) {
                    AddHealth(pti);
                }
                /*this.m_nHPDecPart = (int) ((pt - (double) pti) * 100.0 + (double) this.m_nHPDecPart);
                int part = this.m_nHPDecPart / 100;
                if (part != 0) {
                    this.AddHP(part);
                    this.m_nHPDecPart = this.m_nHPDecPart % 100;
                }*/
            }
            if (!HasFlag(UNIT_FIELD_STATUS, MPRegenStopped)) {
                pt = GetMaxMana() * m_cAtribute.nMPRegenPercentage;
                pt = pt * 0.01f * etf;// +0.0;
                pt = pt + (float) m_cAtribute.nMPRegenPoint * etf;

                /*if (this.IsSitDown())
                    pt = this.m_fMPHealRatioByRest * pt;*/
                if (pt < 1.0f)
                    pt = 1.0f;
                pt     = GetFloatValue(UNIT_FIELD_MP_REGEN_MOD);
                if (pt != 0.0)
                    AddMana(pt);
            }
            if (prev_hp != GetHealth() || prev_mp != GetMana()) {
                this->m_fRegenMP += (GetMana() - prev_mp);
                this->m_nRegenHP += GetHealth() - prev_hp;
                if (GetMaxHealth() == GetHealth() || GetMaxMana() == GetMana() || 100 * m_nRegenHP / GetMaxHealth() > 3 || 100 * m_fRegenMP / GetMaxMana() > 3) {
                    XPacket hpmpPct(TS_SC_REGEN_HPMP);
                    hpmpPct << (uint) GetHandle();
                    hpmpPct << (int16) m_nRegenHP;
                    hpmpPct << (int16) m_fRegenMP;
                    hpmpPct << (int32) GetHealth();
                    hpmpPct << (int16) GetMana();

                    this->m_nRegenHP = 0;
                    this->m_fRegenMP = 0;
                    if (IsInWorld()) {
                        sWorld->Broadcast((uint) (GetPositionX() / g_nRegionSize), (uint) (GetPositionY() / g_nRegionSize), GetLayer(), hpmpPct);
                    }
                    /*if (this.IsSummon())
                    {
                        Summon s = (Summon) this;
                        Player player = s.m_master;
                        if (player != null)
                        {
                            if (player.bIsInWorld && (player.m_StatusFlag & StatusFlags.LoginComplete) != 0)
                            {
                                if (player.m_nLogoutTime == 0)
                                    player.Connection.SendTCP(pak);
                            }
                        }
                    }*/
                }
            }
        }
        m_nLastUpdatedTime = t;
    }
}

int Unit::GetBaseSkillLevel(int skill_id)
{
    Skill* s = GetSkill(skill_id);
    return s == nullptr ? 0 : s->skill_level;
}

Skill* Unit::GetSkill(int skill_id)
{
    for(auto s : m_vSkillList) {
        if(s->skill_id == skill_id)
            return s;
    }
    return nullptr;
}

Skill* Unit::RegisterSkill(int skill_id, int skill_level, uint remain_cool_time, int nJobID)
{
    Skill* pSkill = nullptr;
    int nNeedJP = sObjectMgr->GetNeedJpForSkillLevelUp(skill_id, skill_level, nJobID);
    if(GetJP() >= nNeedJP) {
        SetJP(GetJP() - nNeedJP);
        if(GetJP() < 0)
            SetJP(0);

        uint64_t nSkillUID = 0;
        int nPrevLevel = GetBaseSkillLevel(skill_id);
        if(nPrevLevel == 0) {
            nSkillUID = sWorld->getSkillIndex();
            pSkill = new Skill(this, nSkillUID, skill_id);
        } else {
            pSkill = GetSkill(skill_id);
            nSkillUID = pSkill == nullptr ? 0 : pSkill->sid;
        }
        if(pSkill != nullptr) {
            pSkill->skill_level = skill_level;
            m_vSkillList.emplace_back(pSkill);

            onRegisterSkill(nSkillUID, skill_id, nPrevLevel, skill_level);
        }
    }
    return pSkill;
}

int Unit::GetCurrentSkillLevel(int skill_id)
{
    auto s = GetSkill(skill_id);
    return s == nullptr ? 0 : s->skill_level + 0;
}
