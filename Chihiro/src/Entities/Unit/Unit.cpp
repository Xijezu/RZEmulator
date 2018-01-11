#include "Unit.h"
#include "ObjectMgr.h"
#include "World.h"
#include "GameRule.h"
#include "Messages.h"
#include "ClientPackets.h"
#include "Skill.h"
#include "MemPool.h"
#include "Item.h"
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
    _mainType   = MT_StaticObject;
    _objType    = OBJ_STATIC;
    _subType    = ST_Object;
    _bIsInWorld = false;
}

Unit::~Unit()
{
    for (auto& sk : m_vSkillList) {
        delete sk;
        sk = nullptr;
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

void Unit::EnterPacket(XPacket &pEnterPct, Unit *pUnit, Player* pPlayer)
{
    pEnterPct << (uint32_t) Messages::GetStatusCode(pUnit, pPlayer);
    pEnterPct << pUnit->GetOrientation();
    pEnterPct << (int32_t) pUnit->GetHealth();
    pEnterPct << (int32_t) pUnit->GetMaxHealth();
    pEnterPct << (int32_t) pUnit->GetMana();
    pEnterPct << (int32_t) pUnit->GetMaxMana();
    pEnterPct << (int32_t) pUnit->GetLevel();
    pEnterPct << (uint8_t) pUnit->GetUInt32Value(UNIT_FIELD_RACE);
    pEnterPct << (uint32_t) pUnit->GetUInt32Value(UNIT_FIELD_SKIN_COLOR);
    pEnterPct << (uint8_t) (pUnit->HasFlag(UNIT_FIELD_STATUS, StatusFlags::FirstEnter) == 1 ? 1 : 0);
    pEnterPct << (int32_t) 0;
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
            m_Attribute.nAttackPointRight += nValue;
            //m_nAttackPointRightWithoutWeapon += (short)nValue;
            if (HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
            {
                m_Attribute.nAttackPointLeft += nValue;
                //this.m_nAttackPointLeftWithoutWeapon += (short)nValue;
            }
        }
        if ((nBitset & 0x100) != 0)
            m_Attribute.nMagicPoint += nValue;
        if ((nBitset & 0x200) != 0)
            m_Attribute.nDefence += nValue;
        if ((nBitset & 0x400) != 0)
            m_Attribute.nMagicDefence += nValue;
        if ((nBitset & 0x800) != 0) {
            m_Attribute.nAttackSpeedRight += nValue;
            if (HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
                m_Attribute.nAttackSpeedLeft += nValue;
        }
        if ((nBitset & 0x1000) != 0)
            m_Attribute.nCastingSpeed += nValue;
        if ((nBitset & 0x2000) != 0 && !HasFlag(UNIT_FIELD_STATUS, StatusFlags::MoveSpeedFixed))
        {
            auto p = dynamic_cast<Player*>(this);
            if (p != nullptr /*|| p.m_nRidingStateUid == 0*/)
                m_Attribute.nMoveSpeed += nValue;
        }
        if ((nBitset & 0x4000) != 0) {
            m_Attribute.nAccuracyRight += nValue;
            if (HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
                m_Attribute.nAccuracyLeft += nValue;
        }
        if ((nBitset & 0x8000) != 0)
            m_Attribute.nMagicAccuracy += nValue;
        if ((nBitset & 0x10000) != 0)
            m_Attribute.nCritical += (short) nValue;
        if ((nBitset & 0x20000) != 0)
            m_Attribute.nBlockChance += nValue;
        if ((nBitset & 0x40000) != 0)
            m_Attribute.nBlockDefence += nValue;
        if ((nBitset & 0x80000) != 0)
            m_Attribute.nAvoid += nValue;
        if ((nBitset & 0x100000) != 0)
            m_Attribute.nMagicAvoid += nValue;
        if ((nBitset & 0x200000) != 0)
            SetInt32Value(UNIT_FIELD_MAX_HEALTH, GetInt32Value(UNIT_FIELD_MAX_HEALTH) + (int) nValue);
        if ((nBitset & 0x400000) != 0)
            SetInt32Value(UNIT_FIELD_MAX_MANA, GetInt32Value(UNIT_FIELD_MAX_MANA) + (int) nValue);
        if ((nBitset & 0x1000000) != 0)
            m_Attribute.nHPRegenPoint += nValue;
        if ((nBitset & 0x2000000) != 0)
            m_Attribute.nMPRegenPoint += nValue;
        if ((nBitset & 0x8000000) != 0)
            m_Attribute.nHPRegenPercentage += nValue;
        if ((nBitset & 0x10000000) != 0)
            m_Attribute.nMPRegenPercentage += nValue;
        if ((nBitset & 0x40000000) != 0)
            m_Attribute.nMaxWeight += nValue;
    }
}

void Unit::incParameter2(uint nBitset, float fValue)
{

    if ((nBitset & 1) != 0)
    {
        m_Resist.nResist[0] += (short)fValue;
    }
    if ((nBitset & 2) != 0)
    {
        m_Resist.nResist[1] += (short)fValue;
    }
    if ((nBitset & 4) != 0)
    {
        m_Resist.nResist[2] += (short)fValue;
    }
    if ((nBitset & 8) != 0)
    {
        m_Resist.nResist[3] += (short)fValue;
    }
    if ((nBitset & 0x10) != 0)
    {
        m_Resist.nResist[4] += (short)fValue;
    }
    if ((nBitset & 0x20) != 0)
    {
        m_Resist.nResist[5] += (short)fValue;
    }
    if ((nBitset & 0x40) != 0)
    {
        m_Resist.nResist[6] += (short)fValue;
    }/*
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
    }*/
    if ((nBitset & 0x10000000) != 0)
        m_Attribute.nCriticalPower += fValue;
    if ((nBitset & 0x20000000) != 0)
        SetFlag(UNIT_FIELD_STATUS, StatusFlags::HPRegenStopped);
    if ((nBitset & 0x40000000) != 0)
        SetFlag(UNIT_FIELD_STATUS, StatusFlags::MPRegenStopped);
}

void Unit::CalculateStat()
{
    CreatureAtributeServer stateAttr{ };
    CreatureStat           stateStat{ };

    int prev_max_hp = GetMaxHealth();
    int prev_max_mp = GetMaxMana();
    int prev_hp     = GetHealth();
    int prev_mp     = GetMana();

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
    m_AttributeByState.Reset(0);
    m_AttributeAmplifier.Reset(0);
    m_Attribute.Reset(0);
    m_Resist.Reset(0);
    m_ResistAmplifier.Reset(0.0f);

    auto statptr = GetBaseStat();
    CreatureStat basestat{};
    if(statptr != nullptr)
        basestat.Copy(*statptr);
    m_cStat.Copy(basestat);
    onBeforeCalculateStat(); // TODO: Reset in Player
    // TODO checkAdditionalItemEffect(); -> Nonexistant
    applyStatByItem();
    applyJobLevelBonus();
    stateStat.Copy(m_cStat);
    applyStatByState();
    m_cStatByState.strength += (m_cStat.strength - stateStat.strength);
    m_cStatByState.vital += (m_cStat.vital - stateStat.vital);
    m_cStatByState.dexterity += (m_cStat.dexterity - stateStat.dexterity);
    m_cStatByState.agility += (m_cStat.agility - stateStat.agility);
    m_cStatByState.intelligence += (m_cStat.intelligence - stateStat.intelligence);
    m_cStatByState.mentality += (m_cStat.mentality - stateStat.mentality);
    m_cStatByState.luck += (m_cStat.luck - stateStat.luck);
    // TODO onApplyStat -> Beltslots
    // TODO amplifyStatByItem -> Nonexistant
    stateStat.Copy(m_cStat);
    getAmplifiedStatByAmplifier(stateStat);
    amplifyStatByState();
    getAmplifiedStatByAmplifier(m_cStat);
    m_cStatByState.strength += (m_cStat.strength - stateStat.strength);
    m_cStatByState.vital += (m_cStat.vital - stateStat.vital);
    m_cStatByState.dexterity += (m_cStat.dexterity - stateStat.dexterity);
    m_cStatByState.agility += (m_cStat.agility - stateStat.agility);
    m_cStatByState.intelligence += (m_cStat.intelligence - stateStat.intelligence);
    m_cStatByState.mentality += (m_cStat.mentality - stateStat.mentality);
    m_cStatByState.luck += (m_cStat.luck - stateStat.luck);
    // TODO onAfterApplyStat -> Nonexistant
    finalizeStat();
    float b1  = GetLevel();
    float fcm = 1.0f;
    SetMaxHealth((uint32_t) (GetMaxHealth() + (m_cStat.vital * 33.0f) + (b1 * 20.0f)));
    SetMaxMana((uint32_t) (GetMaxMana() + (m_cStat.intelligence * 33.0f) + (b1 * 20.0f)));
    calcAttribute(m_Attribute);
    // TODO onAfterCalcAttrivuteByStat -> Nonexistant
    applyItemEffect();
    applyPassiveSkillEffect();
    stateAttr.Copy(m_Attribute);
    applyStateEffect();
    m_AttributeByState.nAttackPointRight += (m_Attribute.nAttackPointRight - stateAttr.nAttackPointRight);
    m_AttributeByState.nAttackPointLeft += (m_Attribute.nAttackPointLeft - stateAttr.nAttackPointLeft);
    m_AttributeByState.nCritical += (m_Attribute.nCritical - stateAttr.nCritical);
    m_AttributeByState.nDefence += (m_Attribute.nDefence - stateAttr.nDefence);
    m_AttributeByState.nBlockDefence += (m_Attribute.nBlockDefence - stateAttr.nBlockDefence);
    m_AttributeByState.nMagicPoint += (m_Attribute.nMagicPoint - stateAttr.nMagicPoint);
    m_AttributeByState.nMagicDefence += (m_Attribute.nMagicDefence - stateAttr.nMagicDefence);
    m_AttributeByState.nAccuracyRight += (m_Attribute.nAccuracyRight - stateAttr.nAccuracyRight);
    m_AttributeByState.nAccuracyLeft += (m_Attribute.nAccuracyLeft- stateAttr.nAccuracyLeft);
    m_AttributeByState.nMagicAccuracy += (m_Attribute.nMagicAccuracy - stateAttr.nMagicAccuracy);
    m_AttributeByState.nAvoid += (m_Attribute.nAvoid - stateAttr.nAvoid);
    m_AttributeByState.nMagicAvoid += (m_Attribute.nMagicAvoid - stateAttr.nMagicAvoid);
    m_AttributeByState.nBlockChance += (m_Attribute.nBlockChance - stateAttr.nBlockChance);
    m_AttributeByState.nMoveSpeed += (m_Attribute.nMoveSpeed - stateAttr.nMoveSpeed);
    m_AttributeByState.nAttackSpeedRight += (m_Attribute.nAttackSpeedRight - stateAttr.nAttackSpeedRight);
    m_AttributeByState.nAttackSpeedLeft += (m_Attribute.nAttackSpeedLeft - stateAttr.nAttackSpeedLeft);
    m_AttributeByState.nDoubleAttackRatio += (m_Attribute.nDoubleAttackRatio - stateAttr.nDoubleAttackRatio);
    m_AttributeByState.nAttackRange += (m_Attribute.nAttackRange - stateAttr.nAttackRange);
    m_AttributeByState.nMaxWeight += (m_Attribute.nMaxWeight - stateAttr.nMaxWeight);
    m_AttributeByState.nCastingSpeed += (m_Attribute.nCastingSpeed - stateAttr.nCastingSpeed);
    m_AttributeByState.nCoolTimeSpeed += (m_Attribute.nCoolTimeSpeed - stateAttr.nCoolTimeSpeed);
    m_AttributeByState.nHPRegenPercentage += (m_Attribute.nHPRegenPercentage - stateAttr.nHPRegenPercentage);
    m_AttributeByState.nHPRegenPoint += (m_Attribute.nHPRegenPoint - stateAttr.nHPRegenPoint);
    m_AttributeByState.nMPRegenPercentage += (m_Attribute.nMPRegenPercentage - stateAttr.nMPRegenPercentage);
    m_AttributeByState.nMPRegenPoint += (m_Attribute.nMPRegenPoint - stateAttr.nMPRegenPoint);
    // TODO applyPassiveSkillAmplifyEffect
    onApplyAttributeAdjustment();
    stateAttr.Copy(m_Attribute);
    getAmplifiedAttributeByAmplifier(stateAttr);
    applyStateAmplifyEffect();
    getAmplifiedAttributeByAmplifier(m_Attribute);
    applyDoubeWeaponEffect();
    SetMaxHealth((uint32_t) (GetInt32Value(UNIT_FIELD_MAX_HEALTH_MODIFIER) + 1.0f) * GetMaxHealth());
    SetMaxMana((uint32_t) (GetInt32Value(UNIT_FIELD_MAX_MANA_MODIFIER) + 1.0f) * GetMaxMana());
    // TODO this.getAmplifiedResistByAmplifier(m_Resist);
    auto hp = GetMaxHealth();
    auto mp = GetMaxMana();
    // TODO this.onCompleteCalculateStat();
    SetHealth(GetHealth());
    SetMana(GetMana());
    onModifyStatAndAttribute();
    if (IsInWorld() && (prev_max_hp != GetMaxHealth() || prev_max_mp != GetMaxMana() || prev_hp != GetHealth() || prev_mp != GetMana())) {
        Messages::BroadcastHPMPMessage(this, GetHealth() - prev_hp, GetMana() - prev_mp, false);
    } else {
        if (GetSubType() == ST_Summon && !IsInWorld() && (prev_max_hp != GetMaxHealth() || prev_max_mp != GetMaxMana() || prev_hp != GetHealth() || prev_mp != GetMana())) {
            auto s = dynamic_cast<Summon *>(this);
            if(s != nullptr)
                Messages::SendHPMPMessage(s->GetMaster(), s, hp, mp, false);
        }
    }
}

void Unit::amplifyStatByState()
{
    if(m_vStateList.empty())
        return;

    std::vector<std::pair<int,int>> vDecreaseList{};
    for(auto& s : m_vStateList) {
        if(s.GetEffectType() == 84) {// Strong Spirit?
            auto nDecreaseLevel = (int)(s.GetValue(0) + (s.GetValue(1) * s.GetLevel()));
            for(int i = 2; i < 11; ++i) {
                if(s.GetValue(i) == 0.0f) {
                    break;
                    vDecreaseList.emplace_back(std::make_pair<int,int>((int)s.GetValue(1), (int)nDecreaseLevel));
                }
            }
        }
    }
    for(auto& s :  m_vStateList) {
        uint16 nOriginalLevel[3]{0};

        for (int i = 0; i < 3; i++)
            nOriginalLevel[i] = s.m_nLevel[i];
        for(auto& rp : vDecreaseList) {
            if(rp.first == (int)s.m_nCode) {
//                             *(v1 + 44) = 0;
//                             *(v1 + 48) = 0;
//                             *(v1 + 52) = 0;
//                             *(v1 + 100) = 0;
//                             do
//                             {
//                                 v17 = *(v1 + *(v1 + 100) + 40)
//                                     - LODWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 80))->end.y);
//                                 *(v1 + *(v1 + 100) + 52) = v17;
//                                 if ( v17 < 0 )
//                                 {
//                                     LODWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 80))->end.y) = -v17;
//                                     *(v1 + *(v1 + 100) + 52) = 0;
//                                 }
//                                 *(v1 + 100) -= 4;
//                             }
//                             while ( *(v1 + 100) >= -8 );
//                             LOWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end.face) = *(v1 + 44);
//                             HIWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end.face) = *(v1 + 48);
//                             LOWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end_time) = *(v1 + 52);
//                             break;
            }
        }

        if(s.GetLevel() != 0 && s.GetEffectType() == 2) {
            // format is 0 = bitset, 1 = base, 2 = add per level
            ampParameter((uint) s.GetValue(0), (int) (s.GetValue(1) + (s.GetValue(2) * s.GetLevel())), true);
            ampParameter((uint) s.GetValue(3), (int) (s.GetValue(4) + (s.GetValue(5) * s.GetLevel())), true);
            ampParameter((uint) s.GetValue(12), (int) (s.GetValue(13) + (s.GetValue(14) * s.GetLevel())), true);
            ampParameter((uint) s.GetValue(15), (int) (s.GetValue(16) + (s.GetValue(17) * s.GetLevel())), true);
        }
        for(int i = 0; i < 3; i++)
            s.m_nLevel[i] = nOriginalLevel[i];
    }
}

void Unit::applyState(State &state)
{
    int effectType = state.GetEffectType();

    switch (effectType) {
        case StateBaseEffect::SEF_MISC:
            switch (state.m_nCode) {
                case StateCode::SC_SLEEP:
                case StateCode::SC_STUN:
                    //this.m_StatusFlag &= ~(StatusFlags.Movable|StatusFlags.MagicCastable|StatusFlags.ItemUsable);

                    break;
                default:
                    break;
            }
            break;
        case StateBaseEffect::SEF_PARAMETER_INC:
            incParameter((uint) state.GetValue(0), (int) (state.GetValue(1) + (state.GetValue(2) * state.GetLevel())), false);
            incParameter((uint) state.GetValue(3), (int) (state.GetValue(4) + (state.GetValue(5) * state.GetLevel())), false);
            incParameter2((uint) state.GetValue(6), (int) (state.GetValue(7) + (state.GetValue(8) * state.GetLevel())));
            incParameter2((uint) state.GetValue(9), (int) (state.GetValue(10) + (state.GetValue(11) * state.GetLevel())));
            incParameter((uint) state.GetValue(12), (int) (state.GetValue(13) + (state.GetValue(14) * state.GetLevel())), false);
            incParameter((uint) state.GetValue(15), (int) (state.GetValue(16) + (state.GetValue(17) * state.GetLevel())), false);
            break;

    }
}

void Unit::applyStateEffect()
{
    if(m_vStateList.empty())
        return;

    std::vector<std::pair<int,int>> vDecreaseList{};
    for(auto& s : m_vStateList) {
        if(s.GetEffectType() == 84) {// Strong Spirit?
            auto nDecreaseLevel = (int)(s.GetValue(0) + (s.GetValue(1) * s.GetLevel()));
            for(int i = 2; i < 11; ++i) {
                if(s.GetValue(i) == 0.0f) {
                    break;
                    vDecreaseList.emplace_back(std::make_pair<int,int>((int)s.GetValue(1), (int)nDecreaseLevel));
                }
            }
        }
    }
    for(auto& s :  m_vStateList) {
        uint16 nOriginalLevel[3]{0};

        for (int i = 0; i < 3; i++)
            nOriginalLevel[i] = s.m_nLevel[i];
        for(auto& rp : vDecreaseList) {
            if(rp.first == (int)s.m_nCode) {
//                             *(v1 + 44) = 0;
//                             *(v1 + 48) = 0;
//                             *(v1 + 52) = 0;
//                             *(v1 + 100) = 0;
//                             do
//                             {
//                                 v17 = *(v1 + *(v1 + 100) + 40)
//                                     - LODWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 80))->end.y);
//                                 *(v1 + *(v1 + 100) + 52) = v17;
//                                 if ( v17 < 0 )
//                                 {
//                                     LODWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 80))->end.y) = -v17;
//                                     *(v1 + *(v1 + 100) + 52) = 0;
//                                 }
//                                 *(v1 + 100) -= 4;
//                             }
//                             while ( *(v1 + 100) >= -8 );
//                             LOWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end.face) = *(v1 + 44);
//                             HIWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end.face) = *(v1 + 48);
//                             LOWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end_time) = *(v1 + 52);
//                             break;
            }
        }

        if(s.GetLevel() != 0) {
            applyState(s);
        }
        for(int i = 0; i < 3; i++)
            s.m_nLevel[i] = nOriginalLevel[i];
    }
}

void Unit::applyStatByState()
{
    if(m_vStateList.empty())
        return;

    std::vector<std::pair<int,int>> vDecreaseList{};
    for(auto& s : m_vStateList) {
        if(s.GetEffectType() == 84) {// Strong Spirit?
            auto nDecreaseLevel = (int)(s.GetValue(0) + (s.GetValue(1) * s.GetLevel()));
            for(int i = 2; i < 11; ++i) {
                if(s.GetValue(i) == 0.0f) {
                    break;
                    vDecreaseList.emplace_back(std::make_pair<int,int>((int)s.GetValue(1), (int)nDecreaseLevel));
                }
            }
        }
    }
    for(auto& s :  m_vStateList) {
        uint16 nOriginalLevel[3]{0};

        for (int i = 0; i < 3; i++)
            nOriginalLevel[i] = s.m_nLevel[i];
        for(auto& rp : vDecreaseList) {
            if(rp.first == (int)s.m_nCode) {
//                             *(v1 + 44) = 0;
//                             *(v1 + 48) = 0;
//                             *(v1 + 52) = 0;
//                             *(v1 + 100) = 0;
//                             do
//                             {
//                                 v17 = *(v1 + *(v1 + 100) + 40)
//                                     - LODWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 80))->end.y);
//                                 *(v1 + *(v1 + 100) + 52) = v17;
//                                 if ( v17 < 0 )
//                                 {
//                                     LODWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 80))->end.y) = -v17;
//                                     *(v1 + *(v1 + 100) + 52) = 0;
//                                 }
//                                 *(v1 + 100) -= 4;
//                             }
//                             while ( *(v1 + 100) >= -8 );
//                             LOWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end.face) = *(v1 + 44);
//                             HIWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end.face) = *(v1 + 48);
//                             LOWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end_time) = *(v1 + 52);
//                             break;
            }
        }

        if(s.GetLevel() != 0) {
            if(s.GetEffectType() == 1) {
                // format is 0 = bitset, 1 = base, 2 = add per level
                incParameter((uint)s.GetValue(0), (int)(s.GetValue(1) + (s.GetValue(2) * s.GetLevel())), true);
                incParameter((uint)s.GetValue(3), (int)(s.GetValue(4) + (s.GetValue(5) * s.GetLevel())), true);
                incParameter((uint)s.GetValue(12), (int)(s.GetValue(13) + (s.GetValue(14) * s.GetLevel())), true);
                incParameter((uint)s.GetValue(15), (int)(s.GetValue(16) + (s.GetValue(17) * s.GetLevel())), true);
            } else if(s.GetEffectType() == 3 && IsWearShield()) {
                incParameter((uint)s.GetValue(0), (int)(s.GetValue(1) + (s.GetValue(2) + s.GetLevel())), true);
                incParameter((uint)s.GetValue(3), (int)(s.GetValue(4) + (s.GetValue(5) + s.GetLevel())), true);
            }
        }
        for(int i = 0; i < 3; i++)
            s.m_nLevel[i] = nOriginalLevel[i];
    }
}


void Unit::applyItemEffect()
{
    Item *curItem = GetWornItem(ItemWearType::WearWeapon);
    if (curItem != nullptr && curItem->m_pItemBase != nullptr)
        m_Attribute.nAttackRange = curItem->m_pItemBase->range;

    m_nUnitExpertLevel = 0;
    std::vector<int> ref_list{ };
    for (int i = 0; i < 24; i++) {
        curItem = GetWornItem((ItemWearType) i);
        if (curItem != nullptr && curItem->m_pItemBase != nullptr) {
            auto iwt = (ItemWearType)i;
            if (TranslateWearPosition(iwt, curItem, ref_list)) {
                float    fItemRatio = 1.0f;
                if(curItem->GetLevelLimit() > GetLevel() && curItem->GetLevelLimit() <= m_nUnitExpertLevel)
                    fItemRatio = 0.40000001f;

                for (int ol = 0; ol < Item::MAX_OPTION_NUMBER; ol++) {
                    if (curItem->m_pItemBase->base_type[ol] != 0) {
                        onItemWearEffect(curItem, true, curItem->m_pItemBase->base_type[ol], curItem->m_pItemBase->base_var[ol][0], curItem->m_pItemBase->base_var[ol][1], fItemRatio);
                    }
                }

                for (int ol = 0; ol < Item::MAX_OPTION_NUMBER; ol++) {
                    if (curItem->m_pItemBase->opt_type[ol] != 0) {
                        onItemWearEffect(curItem, false, curItem->m_pItemBase->opt_type[ol], curItem->m_pItemBase->opt_var[ol][0], curItem->m_pItemBase->opt_var[ol][1], fItemRatio);
                    }
                }

                float fAddPoint    = 0.0f;
                float fTotalPoints = 0.0f;

                for (int ol = 0; ol < 2; ol++) {
                    if (curItem->m_pItemBase->enhance_id[ol] != 0) {
                        int curEnhance  = curItem->m_Instance.nEnhance;
                        int realEnhance = curEnhance;

                        if (realEnhance >= 1) {
                            fTotalPoints = curItem->m_pItemBase->_enhance[0][ol] * curEnhance;

                            if (realEnhance > 4) {
                                fTotalPoints += (curItem->m_pItemBase->_enhance[1][ol] * (float) (realEnhance - 4));
                            }
                            if (realEnhance > 8) {
                                fTotalPoints += (curItem->m_pItemBase->_enhance[2][ol] * (float) (realEnhance - 8));
                            }
                            if (realEnhance > 12) {
                                fTotalPoints += (curItem->m_pItemBase->_enhance[3][ol] * (float) (realEnhance - 12));
                            }
                            onItemWearEffect(curItem, false, curItem->m_pItemBase->enhance_id[ol], fTotalPoints, fTotalPoints, fItemRatio);
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
    }
    if ((nBitset & 0x20000000) != 0)
        SetFlag(UNIT_FIELD_STATUS, StatusFlags::HPRegenStopped);
    if ((nBitset & 0x40000000) != 0)
        SetFlag(UNIT_FIELD_STATUS, StatusFlags::MPRegenStopped);

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
        auto p = dynamic_cast<Player*>(this);
        if ((nBitset & 0x80) != 0) {
            m_AttributeAmplifier.fAttackPointRight += fValue;
            if (HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon)) {
                m_AttributeAmplifier.fAttackPointLeft += fValue;
            }
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
            if (HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon)) {
                m_AttributeAmplifier.fAttackSpeedLeft += fValue;
            }
        }
        if ((nBitset & 0x1000) != 0) {
            m_AttributeAmplifier.fCastingSpeed += fValue;
        }
        if ((nBitset & 0x2000) != 0) {
            if (p == nullptr || true/*|| p->m_nRidingStateUid == 0*/)
            {
                m_AttributeAmplifier.fMoveSpeed += fValue;
            }
        }
        if ((nBitset & 0x4000) != 0) {
            m_AttributeAmplifier.fAccuracyRight += fValue;
            if (HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon)) {
                m_AttributeAmplifier.fAccuracyLeft += fValue;
            }
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

void Unit::getAmplifiedAttributeByAmplifier(CreatureAtributeServer &attribute)
{
    attribute.nCritical += (m_AttributeAmplifier.fCritical * attribute.nCritical);
    attribute.nCriticalPower += (m_AttributeAmplifier.fCriticalPower * attribute.nCriticalPower);
    attribute.nAttackPointRight += (m_AttributeAmplifier.fAttackPointRight * attribute.nAttackPointRight);
    attribute.nAttackPointLeft += (m_AttributeAmplifier.fAttackPointLeft * attribute.nAttackPointLeft);
    attribute.nDefence += (m_AttributeAmplifier.fDefence * attribute.nDefence);
    attribute.nBlockDefence += (m_AttributeAmplifier.fBlockDefence * attribute.nBlockDefence);
    attribute.nMagicPoint += (m_AttributeAmplifier.fMagicPoint * attribute.nMagicPoint);
    attribute.nMagicDefence += (m_AttributeAmplifier.fMagicDefence * attribute.nMagicDefence);
    attribute.nAccuracyRight += (m_AttributeAmplifier.fAccuracyRight * attribute.nAccuracyRight);
    attribute.nAccuracyLeft += (m_AttributeAmplifier.fAccuracyLeft * attribute.nAccuracyLeft);
    attribute.nMagicAccuracy += (m_AttributeAmplifier.fMagicAccuracy * attribute.nMagicAccuracy);
    attribute.nAvoid += (m_AttributeAmplifier.fAvoid * attribute.nAvoid);;
    attribute.nMagicAvoid += (m_AttributeAmplifier.fMagicAvoid * attribute.nMagicAvoid);
    attribute.nBlockChance += (m_AttributeAmplifier.fBlockChance * attribute.nBlockChance);
    if (!HasFlag(UNIT_FIELD_STATUS, StatusFlags::MoveSpeedFixed))
        attribute.nMoveSpeed += (m_AttributeAmplifier.fMoveSpeed * attribute.nMoveSpeed);

    attribute.nAttackSpeed += (m_AttributeAmplifier.fAttackSpeed * attribute.nAttackSpeed);
    attribute.nAttackRange += (m_AttributeAmplifier.fAttackRange * attribute.nAttackRange);
    attribute.nMaxWeight += (short)(m_AttributeAmplifier.fMaxWeight * attribute.nMaxWeight);
    attribute.nCastingSpeed += (short)(m_AttributeAmplifier.fCastingSpeed * attribute.nCastingSpeed);
    attribute.nCoolTimeSpeed += (m_AttributeAmplifier.fCoolTimeSpeed * attribute.nCoolTimeSpeed);
    attribute.nItemChance += (m_AttributeAmplifier.fItemChance * attribute.nItemChance);
    attribute.nHPRegenPercentage += (m_AttributeAmplifier.fHPRegenPercentage * attribute.nHPRegenPercentage);
    attribute.nHPRegenPoint += (m_AttributeAmplifier.fHPRegenPoint * attribute.nHPRegenPoint);
    attribute.nMPRegenPercentage += (m_AttributeAmplifier.fMPRegenPercentage * attribute.nMPRegenPercentage);
    attribute.nMPRegenPoint += (m_AttributeAmplifier.fMPRegenPoint * attribute.nMPRegenPoint);
    attribute.nAttackSpeedRight += (m_AttributeAmplifier.fAttackSpeedRight * attribute.nAttackSpeedRight);
    attribute.nAttackSpeedLeft += (m_AttributeAmplifier.fAttackSpeedLeft * attribute.nAttackSpeedLeft);
    attribute.nDoubleAttackRatio += (m_AttributeAmplifier.fDoubleAttackRatio * attribute.nDoubleAttackRatio);
    attribute.nStunResistance += (m_AttributeAmplifier.fStunResistance * attribute.nStunResistance);
    attribute.nMoveSpeedDecreaseResistance += (m_AttributeAmplifier.fMoveSpeedDecreaseResistance * attribute.nMoveSpeedDecreaseResistance);
    attribute.nHPAdd += (m_AttributeAmplifier.fHPAdd * attribute.nHPAdd);
    attribute.nMPAdd += (m_AttributeAmplifier.fMPAdd * attribute.nMPAdd);
    attribute.nHPAddByItem += (m_AttributeAmplifier.fHPAddByItem * attribute.nHPAddByItem);
    attribute.nMPAddByItem += (m_AttributeAmplifier.fMPAddByItem * attribute.nMPAddByItem);
}

void Unit::applyStateAmplifyEffect()
{
    if(m_vStateList.empty())
        return;

    std::vector<std::pair<int,int>> vDecreaseList{};
    for(auto& s : m_vStateList) {
        if(s.GetEffectType() == 84) {// Strong Spirit?
            auto nDecreaseLevel = (int)(s.GetValue(0) + (s.GetValue(1) * s.GetLevel()));
            for(int i = 2; i < 11; ++i) {
                if(s.GetValue(i) == 0.0f) {
                    break;
                    vDecreaseList.emplace_back(std::make_pair<int,int>((int)s.GetValue(1), (int)nDecreaseLevel));
                }
            }
        }
    }
    for(auto& s :  m_vStateList) {
        uint16 nOriginalLevel[3]{0};

        for (int i = 0; i < 3; i++)
            nOriginalLevel[i] = s.m_nLevel[i];
        for(auto& rp : vDecreaseList) {
            if(rp.first == (int)s.m_nCode) {
//                             *(v1 + 44) = 0;
//                             *(v1 + 48) = 0;
//                             *(v1 + 52) = 0;
//                             *(v1 + 100) = 0;
//                             do
//                             {
//                                 v17 = *(v1 + *(v1 + 100) + 40)
//                                     - LODWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 80))->end.y);
//                                 *(v1 + *(v1 + 100) + 52) = v17;
//                                 if ( v17 < 0 )
//                                 {
//                                     LODWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 80))->end.y) = -v17;
//                                     *(v1 + *(v1 + 100) + 52) = 0;
//                                 }
//                                 *(v1 + 100) -= 4;
//                             }
//                             while ( *(v1 + 100) >= -8 );
//                             LOWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end.face) = *(v1 + 44);
//                             HIWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end.face) = *(v1 + 48);
//                             LOWORD(std::_Vector_const_iterator<PartyManager::PartyInfo___std::allocator<PartyManager::PartyInfo__>>::operator_((v1 + 104))->end_time) = *(v1 + 52);
//                             break;
            }
        }

        if(s.GetLevel() != 0) {
            applyStateAmplify(s);
        }
        for(int i = 0; i < 3; i++)
            s.m_nLevel[i] = nOriginalLevel[i];
    }
}

void Unit::applyStateAmplify(State &state)
{
    int effectType = state.GetEffectType();
    int level = state.GetLevel();
    if (effectType != 0)
    {
        if (effectType == StateBaseEffect::SEF_PARAMETER_AMP)
        {
            ampParameter((uint)state.GetValue(0),(state.GetValue(1) + (state.GetValue(2) * level)),false);
            ampParameter((uint)state.GetValue(3),(state.GetValue(4) + (state.GetValue(5) * level)),false);
            ampParameter2((uint)state.GetValue(6),(state.GetValue(7) + (state.GetValue(8) * level)));
            ampParameter2((uint)state.GetValue(9),(state.GetValue(10) + (state.GetValue(11) * level)));
            ampParameter((uint)state.GetValue(12),(state.GetValue(13) + (state.GetValue(14) * level)),false);
            ampParameter((uint)state.GetValue(15),(state.GetValue(16) + (state.GetValue(17) * level)),false);
        }
    }
    else
    {
        if (state.m_nCode == StateCode::SC_SQUALL_OF_ARROW)
        {
            if(state.GetValue(1) == 0.0f)
                SetFlag(UNIT_FIELD_STATUS, StatusFlags::Movable);
            else
                ToggleFlag(UNIT_FIELD_STATUS, StatusFlags::Movable);

            if ((int)GetWeaponClass() == (int)state.GetValue(0))
            {
                m_AttributeAmplifier.fAttackSpeedRight += state.GetValue(2) + (state.GetValue(3) * level);
                if (HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
                {
                    m_AttributeAmplifier.fAttackSpeedLeft += state.GetValue(2) + (state.GetValue(3) * level);
                }
            }
        }
    }
}


void Unit::onItemWearEffect(Item *pItem, bool bIsBaseVar, int type, float var1, float var2, float fRatio)
{
    float item_var_penalty{};

    Player* p{nullptr};
    if(GetSubType() == ST_Player)
        p = dynamic_cast<Player*>(this);

    if (type == 14)
        item_var_penalty = var1;
    else
        item_var_penalty = var1 * fRatio;

    if (type != 14) {
        if (pItem != nullptr && bIsBaseVar) {
            item_var_penalty += (float) (var2 * (float) (pItem->m_Instance.nLevel - 1));
            item_var_penalty = GameRule::GetItemValue(item_var_penalty, (int) var1, GetLevel(), pItem->GetItemRank(), pItem->m_Instance.nLevel);
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
                    m_Attribute.nMagicPoint += item_var_penalty;
                    return;
                case 11:
                    if (!HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
                    {
                        m_Attribute.nAttackPointRight += item_var_penalty;
                    }
                    else if (pItem == nullptr || pItem->m_pItemBase->group != 1)
                    {
                        m_Attribute.nAttackPointLeft += item_var_penalty;
                        m_Attribute.nAttackPointRight += item_var_penalty;
                    }
                    else if (pItem->m_Instance.nWearInfo != ItemWearType::WearShield)
                    {
                        m_Attribute.nAttackPointRight += item_var_penalty;
                    }
                    else
                    {
                        m_Attribute.nAttackPointLeft += item_var_penalty;
                    }
                    return;
                case 21:
                    m_Attribute.nBlockDefence += item_var_penalty;
                    return;
                case 15:
                    m_Attribute.nDefence += item_var_penalty;
                    return;
                case 13:
                    if (!HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
                    {
                        m_Attribute.nAccuracyRight += item_var_penalty;
                    }
                    else if (pItem == nullptr || pItem->m_pItemBase->group != 1)
                    {
                        m_Attribute.nAccuracyLeft += item_var_penalty;
                        m_Attribute.nAccuracyRight += item_var_penalty;
                    }
                    else if (pItem->m_Instance.nWearInfo != ItemWearType::WearShield)
                    {
                        m_Attribute.nAccuracyRight += item_var_penalty;
                    }
                    else
                    {
                        m_Attribute.nAccuracyLeft += item_var_penalty;
                    }
                    return;
                case 14:
                    if (!HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
                    {
                        m_Attribute.nAttackSpeedRight += item_var_penalty;
                    }
                    else if (pItem == nullptr || pItem->m_pItemBase->group != 1)
                    {
                        m_Attribute.nAttackSpeedRight += item_var_penalty;
                        m_Attribute.nAttackSpeedLeft += item_var_penalty;
                    }
                    else if ( pItem->m_Instance.nWearInfo != ItemWearType::WearShield)
                    {
                        m_Attribute.nAttackSpeedRight += item_var_penalty;
                    }
                    else
                    {
                        m_Attribute.nAttackSpeedLeft += item_var_penalty;
                    }
                    return;
                case 16:
                    m_Attribute.nMagicDefence += item_var_penalty;
                    return;
                case 17:
                    m_Attribute.nAvoid += item_var_penalty;
                    return;
                case 18:
                    if (!HasFlag(UNIT_FIELD_STATUS, StatusFlags::MoveSpeedFixed) && (p == nullptr || true)) // TODO: RidingStateUID
                        m_Attribute.nMoveSpeed += item_var_penalty;
                    return;
                case 19:
                    m_Attribute.nBlockChance += item_var_penalty;
                    return;
                case 20:
                    m_Attribute.nMaxWeight += item_var_penalty;
                    return;
                case 22:
                    m_Attribute.nCastingSpeed += item_var_penalty;
                    return;
                case 23:
                    m_Attribute.nMagicAccuracy += item_var_penalty;
                    break;
                case 24:
                    m_Attribute.nMagicAvoid += item_var_penalty;
                    break;
                case 25:
                    m_Attribute.nCoolTimeSpeed += item_var_penalty;
                    break;
                case 30:
                    SetMaxHealth(GetMaxHealth() + (short) item_var_penalty);
                    break;
                case 31:
                    SetInt32Value(UNIT_FIELD_MAX_MANA, GetMaxMana() + (short) item_var_penalty);;
                    break;
                case 33:
                    m_Attribute.nMPRegenPoint += item_var_penalty;
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

    float b1                   = GetLevel();
    float fcm                  = 1.0f;
    float d1                   = (fcm * 5.0f);

    if (false) {
        // isUsingBow | IsUsingCrossBow
        v = (1.2f * m_cStat.agility) + (2.2f * m_cStat.dexterity) + (fcm * b1);
        attribute.nAttackPointRight += v;
    } else {
        v = (2.0f * m_cStat.strength) + (fcm * b1);
        attribute.nAttackPointRight += v;
        if (HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
            attribute.nAttackPointLeft += v;// ((2 * this.m_Stat.strength) * fcm + bl);
    }

    attribute.nMagicPoint += ((2 * m_cStat.intelligence) + (fcm * b1));
    attribute.nItemChance += (m_cStat.luck * 0.2000000029802322f);
    attribute.nDefence += ((2.0f * m_cStat.vital) + (fcm * b1));
    attribute.nMagicDefence += ((2.0f * m_cStat.mentality) + (fcm * b1));
    attribute.nMaxWeight += 10 * (GetLevel() + m_cStat.strength);
    attribute.nAccuracyRight += ((m_cStat.dexterity) * 0.5f * fcm + b1);
    if(HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
        attribute.nAccuracyLeft += ((m_StatAmplifier.dexterity) * 0.5f * fcm + b1);
    attribute.nMagicAccuracy += ((m_cStat.mentality * 0.4f + m_cStat.dexterity * 0.1f) * fcm + b1);
    attribute.nAvoid += (m_cStat.agility * 0.5f * fcm + b1);
    attribute.nMagicAvoid += (m_cStat.mentality * 0.5f * fcm + b1);
    attribute.nAttackSpeedRight += 100; /*(100 + (m_cStat.agility * 0.1f));*/
    if(HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
        attribute.nAttackSpeedLeft += 100;/*((this->m_cStat.dexterity) * 0.5f + (fcm + b1));*/
    if(!HasFlag(UNIT_FIELD_STATUS, StatusFlags::MoveSpeedFixed))
        attribute.nMoveSpeed += (120 * fcm);
    attribute.nCastingSpeed += (100 * fcm);
    attribute.nCoolTimeSpeed   = 100;

    attribute.nHPRegenPercentage += d1;
    attribute.nHPRegenPoint += ((2 * fcm * b1) + 48.0f);
    attribute.nMPRegenPercentage += d1;
    attribute.nMPRegenPoint += ((2 * fcm * b1) + 48.0f); // + (4.1f * m_cStat.mentality));
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

    for(int i1 = 0; i1 < 24; ++i1) {
        Item* item = m_anWear[i1];
        if (item != nullptr && item->m_pItemBase != nullptr) {
            auto iwt = (ItemWearType)i1;
            if (item->m_Instance.nWearInfo != ItemWearType::WearNone) {
                if (TranslateWearPosition(iwt, item, ref_list)) {
                    for (int j = 0; j < 4; j++) {
                        short ot = item->m_pItemBase->opt_type[j];
                        auto  bs = (uint) item->m_pItemBase->opt_var[j][0];
                        auto  fv = (int) item->m_pItemBase->opt_var[j][1];
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

void Unit::SetMove(Position _to, uint8 _speed, uint _start_time)
{
    ArMoveVector::SetMove(_to, _speed, _start_time, sWorld->GetArTime());
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
    if (HasFlag(UNIT_FIELD_STATUS, NeedToCalculateStat)) {
        CalculateStat();
        RemoveFlag(UNIT_FIELD_STATUS, NeedToCalculateStat);
    }
    this->regenHPMP(ct);

    if(IsInWorld()) {
        if(!m_vStateList.empty() && m_nLastStateProcTime + 100 < ct) {
            //procStateDamage(ct);
            //procState(ct);
            if(ClearExpiredState(ct)) {
                CalculateStat();
                m_nLastStateProcTime = ct;
            }
        }
    }
}

void Unit::regenHPMP(uint t)
{
    float prev_mp;
    int   prev_hp;
    float pt;

    uint et = t - m_nLastUpdatedTime;
    if (et >= 300) {
        float etf = (float) et / 6000.0f;
        //prev_mp = et;

        if (GetHealth() != 0) {
            prev_mp = GetHealth();
            prev_hp = GetMana();
            if (!HasFlag(UNIT_FIELD_STATUS, HPRegenStopped)) {
                pt     = GetMaxHealth() * m_Attribute.nHPRegenPercentage;
                pt     = pt * 0.01f * etf;// + 0.0;
                pt     = pt + m_Attribute.nHPRegenPoint * etf;
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
                pt = GetMaxMana() * m_Attribute.nMPRegenPercentage;
                pt = pt * 0.01f * etf;// +0.0;
                pt = pt + (float) m_Attribute.nMPRegenPoint * etf;

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

int Unit::GetBaseSkillLevel(int skill_id)  const
{
    Skill *s = GetSkill(skill_id);
    return s == nullptr ? 0 : s->m_nSkillLevel;
}

Skill *Unit::GetSkill(int skill_id)  const
{
    for (auto s : m_vSkillList) {
        if (s->m_nSkillID == skill_id)
            return s;
    }
    return nullptr;
}

Skill *Unit::RegisterSkill(int skill_id, int skill_level, uint remain_cool_time, int nJobID)
{
    Skill *pSkill = nullptr;
    int   nNeedJP = sObjectMgr->GetNeedJpForSkillLevelUp(skill_id, skill_level, nJobID);
    if (GetJP() >= nNeedJP) {
        SetJP(GetJP() - nNeedJP);
        if (GetJP() < 0)
            SetJP(0);

        onExpChange();

        uint64_t nSkillUID  = 0;
        int      nPrevLevel = GetBaseSkillLevel(skill_id);
        if (nPrevLevel == 0) {
            nSkillUID = sWorld->GetSkillIndex();
            pSkill    = new Skill{this, nSkillUID, skill_id};
            m_vSkillList.emplace_back(pSkill);
        } else {
            pSkill    = GetSkill(skill_id);
            nSkillUID = pSkill == nullptr ? 0 : pSkill->m_nSkillUID;
        }
        if (pSkill != nullptr) {
            pSkill->m_nSkillLevel = skill_level;

            onRegisterSkill(nSkillUID, skill_id, nPrevLevel, skill_level);
        }
    }
    return pSkill;
}

int Unit::GetCurrentSkillLevel(int skill_id) const
{
    auto s = GetSkill(skill_id);
    return s == nullptr ? 0 : s->m_nSkillLevel + 0;
}

void Unit::SetSkill(int skill_uid, int skill_id, int skill_level, int remain_cool_time)
{
    auto skill = new Skill(this, skill_uid, skill_id);
    skill->m_nSkillID    = skill_id;
    skill->m_nSkillLevel = skill_level;
    skill->m_nSkillUID   = skill_uid;
    skill->cool_time     = remain_cool_time;
    m_vSkillList.emplace_back(skill);
}

int Unit::CastSkill(int nSkillID, int nSkillLevel, uint target_handle, Position pos, uint8 layer, bool bIsCastedByItem)
{
    auto     player = dynamic_cast<Player *>(this);
    Summon   *summon{nullptr};
    Position tpos(pos);

    auto pSkill = GetSkill(nSkillID);
    if (pSkill == nullptr || pSkill->m_SkillBase == nullptr/* current casting skill || using storage */)
        return TS_RESULT_NOT_ACTABLE;

    //auto pSkillTarget = sMemoryPool->getPtrFromId(target_handle);
    auto pSkillTarget = sMemoryPool->GetObjectInWorld<WorldObject>(target_handle);
    /// Checking here if return feather due to nullptrs
    // Return feather
    if (pSkillTarget == nullptr && nSkillID == 6020 && GetSubType() == ST_Player /* && IsInSiegeOrRaidDungeon*/)
        return TS_RESULT_NOT_ACTABLE;
    if (pSkillTarget == nullptr)
        return TS_RESULT_NOT_EXIST;

    switch (pSkill->m_SkillBase->target) {
        case TargetType::Master:
            if (!this->GetSubType() == ST_Summon)
                return TS_RESULT_NOT_ACTABLE;
            summon = dynamic_cast<Summon *>(this);
            if (summon->GetMaster()->GetHandle() != pSkillTarget->GetHandle())
                return TS_RESULT_NOT_ACTABLE;
            break;
        case TargetType::SelfWithMaster:
            if (!this->GetSubType() == ST_Summon)
                return TS_RESULT_NOT_ACTABLE;
            summon = dynamic_cast<Summon *>(this);
            if (pSkillTarget->GetHandle() != GetHandle() && summon->GetMaster()->GetHandle() != pSkillTarget->GetHandle())
                return TS_RESULT_NOT_ACTABLE;
            break;
        case TargetType::TargetExceptCaster:
            if (pSkillTarget->GetHandle() == GetHandle())
                return TS_RESULT_NOT_ACTABLE;
            break;
        default:
            break;
    }

    if (pSkillTarget->GetSubType() != ST_Object && !pSkillTarget->IsInWorld())
        return TS_RESULT_NOT_ACTABLE;

    uint ct = sWorld->GetArTime();

    Position t               = (pSkillTarget->GetCurrentPosition(ct));
    float    target_distance = (GetCurrentPosition(ct).GetExactDist2d(&t) - 6.0f);
    float    enemy_distance  = target_distance - (6.0f); // @Todo: Unit Size
    float    range_mod       = 1.2f;
    if (pSkillTarget->bIsMoving) {
        if (pSkillTarget->IsInWorld())
            range_mod = 1.5f;
    }
    bool isInRange{false};
    //if(sObjectMgr->GetSkillBase(nSkillID).cast_range == -1)
    //isInRange = enemy_distance < (12*m_Attribute.nAttackRange) / 100f
    int  res                 = pSkill->Cast(nSkillLevel, target_handle, tpos, layer, bIsCastedByItem);
}

void Unit::onAttackAndSkillProcess()
{
    if (m_castingSkill != nullptr) {
        m_castingSkill->ProcSkill();
    } else {
        if(GetTargetHandle() != 0)
            processAttack();
    }
}

bool Unit::StartAttack(uint target, bool bNeedFastReaction)
{
    MX_LOG_INFO("unit", "Start attack: %d", sWorld->GetArTime());
    bool result{false};
    if(GetHealth() == 0) {
        result = false;
    } else {
        SetUInt32Value(BATTLE_FIELD_TARGET_HANDLE, target);
        SetFlag(UNIT_FIELD_STATUS, StatusFlags::AttackStarted);
        if(bNeedFastReaction)
            onAttackAndSkillProcess();
        result = true;
    }
    return result;
}

void Unit::processAttack()
{
    uint t = sWorld->GetArTime();
    if (false /*IsAttackable()*/)
        return;


    Player *player{nullptr};

    if (GetNextAttackableTime() <= t) {
        //auto enemy = dynamic_cast<Unit *>(sMemoryPool->getPtrFromId(GetTargetHandle()));
        auto enemy = sMemoryPool->GetObjectInWorld<Unit>(GetTargetHandle());
        if (GetHealth() == 0) {
            CancelAttack();
            return;
        }
        if (IsMoving(t) || GetTargetHandle() == 0)
            return;

        if (enemy == nullptr || enemy->GetHealth() == 0) {
            if (GetSubType() == ST_Player) {
                player = dynamic_cast<Player *>(this);
            } else if (GetSubType() == ST_Summon) {
                auto summon = dynamic_cast<Summon *>(this);
                if (this != nullptr)
                    player = summon->GetMaster();
            }

            if (player != nullptr) {
                //Messages::SendHPMPMessage(player, enemy, 0, 0, true);
                Messages::SendCantAttackMessage(player, player->GetHandle(), player->GetTargetHandle(), TS_RESULT_NOT_EXIST);
            }
            EndAttack();
            return;
        }

        if (HasFlag(UNIT_FIELD_STATUS, StatusFlags::FirsAttack)) {
            enemy->OnUpdate();
            RemoveFlag(UNIT_FIELD_STATUS, StatusFlags::FirsAttack);
        }

        if (enemy->GetHealth() == 0) {
            if (GetSubType() == ST_Player) {
                player = dynamic_cast<Player *>(this);
            } else if (GetSubType() == ST_Summon) {
                auto summon = dynamic_cast<Summon *>(this);
                if (this != nullptr)
                    player = summon->GetMaster();
            }

            if (player != nullptr)
                Messages::SendCantAttackMessage(player, player->GetHandle(), player->GetTargetHandle(), TS_RESULT_NOT_EXIST);
            EndAttack();
            return;
        }

        auto enemyPosition = enemy->GetCurrentPosition(t);
        auto myPosition    = GetCurrentPosition(t);

        auto real_distance = myPosition.GetExactDist2d(&enemyPosition) - ((enemy->GetUnitSize() * 0.5f) + (GetUnitSize() * 0.5f));
        auto attack_range  = GetRealAttackRange();
        SetDirection(enemyPosition);
        if (enemy->bIsMoving && enemy->IsInWorld())
            attack_range *= 1.5f;
        else
            attack_range *= 1.200000047683716f;

        AttackInfo Damages[4]{ };

        bool _bDoubleAttack{false};

        uint attack_interval = GetAttackInterval();
        auto attInt          = GetAttackInterval();
        if (attack_range < real_distance) {
            onCantAttack(enemy->GetHandle(), t);
            return;
        }
        int next_mode  = 0;
        // If Bow/Crossbow
        if (false) {

        } else {
            next_mode = 0;
        }
        if (next_mode == 0) {
            m_nMovableTime = attInt + t;
            Attack(enemy, t, attack_interval, Damages, _bDoubleAttack);
        }
        SetFlag(UNIT_FIELD_STATUS, StatusFlags::AttackStarted);

        if(next_mode != 1) {
            int delay = (int)(10 * (GetNextAttackableTime() - t));
            broadcastAttackMessage(enemy, Damages, (int)(10 * attInt), delay, _bDoubleAttack, next_mode == 1, false, false);
        }
        return;
    }
}

void Unit::Attack(Unit *pTarget, uint t, uint attack_interval, AttackInfo *arDamage, bool &bIsDoubleAttack)
{
    uint ct = t;
    if (ct == 0)
        ct = sWorld->GetArTime();

    DamageInfo di{};

    SetUInt32Value(BATTLE_FIELD_NEXT_ATTACKABLE_TIME, attack_interval + ct);
    bIsDoubleAttack = false;

    int nHate        = 0;
    int nAttackCount = 1;
    if (HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
        nAttackCount = 2;
    if (((uint)rand32() % 100) < m_Attribute.nDoubleAttackRatio) {
        bIsDoubleAttack = true;
        nAttackCount *= 2;
    }

    if (nAttackCount <= 0)
        return;

    int i = 0;
    do {
        attack_interval = 0;
        if (HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon)) {
            if (((uint) i & 1) != 0)
                attack_interval = 1;
        }

        auto prev_target_hp = pTarget->GetHealth();
        auto prev_target_mp = pTarget->GetMana();
        auto prev_hp        = GetHealth();
        auto prev_mp        = GetMana();
        int  crit           = 0;
        // TODO Crit

        if (attack_interval != 0)
            di;
        else
            di = pTarget->DealPhysicalNormalDamage(this, m_Attribute.nAttackPointRight, ElementalType::TypeNone, 0, crit, 0);
        arDamage[i].SetDamageInfo(di);

        if (arDamage[i].bCritical) {
            // Do Crit calc
        }

        if (!arDamage[i].bMiss) {
            /// Do more calc TODO
        }

        arDamage[i].nDamage            = prev_target_hp - pTarget->GetHealth();
        arDamage[i].mp_damage          = prev_target_mp - pTarget->GetMana();
        arDamage[i].attacker_damage    = (short) (prev_hp - GetHealth());
        arDamage[i].attacker_mp_damage = (short) (prev_mp - GetMana());
        arDamage[i].target_hp          = pTarget->GetHealth();
        arDamage[i].target_mp          = pTarget->GetMana();
        arDamage[i].attacker_hp        = GetHealth();
        arDamage[i].attacker_mp        = GetMana();

        nHate += arDamage[i].nDamage;

        if (!arDamage[i].bMiss) {
            // Set Havoc TODO
        }
        ++i;
    } while (i < nAttackCount);
    if(pTarget->GetSubType() == ST_Mob) {
        auto mob = dynamic_cast<Monster*>(pTarget);
        auto hm = GetHateMod(3, true);
        nHate = (int)((float)(nHate + hm.second) * hm.first);

        mob->AddHate(GetHandle(), nHate, true, true);
    }
}

DamageInfo Unit::DealPhysicalNormalDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag)
{
    DamageInfo result{};
    int nTargetGroup{0};
    bool bRange;
    int damage{};
    StateMod damageReduceByState{};


    if(GetSubType() == ST_Mob)
        nTargetGroup = dynamic_cast<Monster*>(this)->GetBase()->monster_group;
    else
        nTargetGroup = 9;

    if(false) {
        bRange = true;
    } else {
        bRange = false;
    }

    // Do damage reduce

    if(nDamage < 0)
        nDamage = 0;

    Damage d{};
    if(bRange) {
        d = DealPhysicalDamage(pFrom, nDamage, elemental_type, accuracy_bonus, critical_bonus, nFlag, nullptr, nullptr);
    } else {
        d = DealPhysicalDamage(pFrom, nDamage, elemental_type, accuracy_bonus, critical_bonus, nFlag, nullptr, nullptr);
    }
    result.SetDamage(d);

    result.target_hp = GetHealth();
    return result;
}

Damage Unit::DealDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, DamageType damageType, int accuracy_bonus, int critical_bonus, int nFlag, StateMod* damage_penalty, StateMod* damage_advantage)
{
    int   nCritical{0};
    float fCritical{0};

    if (damage_penalty != nullptr) {
        nCritical = damage_penalty->nDamage;
        fCritical = damage_penalty->fCritical;
    } else {
        nCritical = 0;
        fCritical = 0.0f;
    }

    if (damage_advantage != nullptr) {
        nCritical += damage_advantage->nCritical;
        fCritical += damage_advantage->fCritical - 1.0f;
    }

    auto result = pFrom->CalcDamage(this, damageType, nDamage, elemental_type, accuracy_bonus, fCritical + 1.0f, critical_bonus + nCritical, nFlag);

    if (!result.bMiss) {
        if (damage_penalty != nullptr) {
            result.nDamage += damage_penalty->nDamage;
            result.nDamage = (int) ((float) result.nDamage * damage_penalty->fDamage);
        }
        if (damage_advantage != nullptr) {
            result.nDamage += damage_advantage->nDamage;
            result.nDamage = (int) ((float) result.nDamage * damage_advantage->fDamage);
        }
    }
    if (result.nDamage < 0)
        result.nDamage = 0;

    //             if (damageType == DamageType.NormalPhysical)
//                 fDamageFlag = this.m_fPhysicalDamageManaShieldAbsorbRatio; // goto LABEL_26;
//             else if ( damageType == DamageType.NormalMagical)
//                 fDamageFlag = this.m_fMagicalDamageManaShieldAbsorbRatio; //goto LABEL_29;
// //             if ( damageType <= DamageType.NormalMagical)
// //                 goto LABEL_36;
//             else if ( damageType <= DamageType.AdditionalLeftHand)
//             {
//         LABEL_26:
//                 fDamageFlag = this.m_fPhysicalDamageManaShieldAbsorbRatio;
//                 goto LABEL_27;
//             }
//             if ( damageType > DamageType.StateMagical)
//             {
//                 if ( damageType != DamageType.StatePhysical)
//                     goto LABEL_36;
//                 fDamageFlag = this.m_fPhysicalDamageManaShieldAbsorbRatio;
//                 goto LABEL_27;
//             }
//         LABEL_29:
//             fDamageFlag = this.m_fMagicalDamageManaShieldAbsorbRatio;
//         LABEL_27:
//             fDamageFlag = v20;

    float fDamageFlag = 0; /*m_fPhysicalDamageManaShieldAbsorbRatio*/
    int   nDamageFlag{0};
    if (fDamageFlag < 0.0f)
        fDamageFlag = 0.0f;
    if (fDamageFlag > 0.0f) {
        if (fDamageFlag > 1.0f)
            fDamageFlag = 1.0f;
        nDamageFlag     = (int) (result.nDamage * fDamageFlag);
        if (GetMana() < nDamageFlag)
            nDamageFlag = GetMana();
        result.nDamage -= nDamageFlag;
        AddMana(-nDamageFlag);
        Messages::BroadcastHPMPMessage(this, 0, nDamageFlag, false);
    }

    int real_damage = onDamage(pFrom, elemental_type, damageType, result.nDamage, result.bCritical);
    damage(pFrom, real_damage, true);

    if(!result.bMiss) {
        auto nPrevHP = pFrom->GetHealth();
        auto nPrevMP = pFrom->GetMana();

        Messages::BroadcastHPMPMessage(pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, false);

        if(GetSubType() == ST_Player) {
            Messages::SendHPMPMessage(dynamic_cast<Player *>(this), pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, true);
        } else if(GetSubType() == ST_Summon) {
            auto s = dynamic_cast<Summon*>(this);
            if(s != nullptr && s->GetMaster() != nullptr)
                Messages::SendHPMPMessage(s->GetMaster(), pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, true);
        }

        if(pFrom->GetSubType() == ST_Player) {
            Messages::SendHPMPMessage(dynamic_cast<Player *>(pFrom), pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, true);
        } else if(pFrom->GetSubType() == ST_Summon) {
            auto s = dynamic_cast<Summon*>(pFrom);
            if(s != nullptr && s->GetMaster() != nullptr)
                Messages::SendHPMPMessage(s->GetMaster(), pFrom, pFrom->GetHealth() - nPrevHP, pFrom->GetMana() - nPrevMP, true);
        }

    }
    result.nDamage = real_damage;
    return result;
}

Damage Unit::DealPhysicalDamage(Unit *pFrom, float nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod* damage_penalty, StateMod* damage_advantage)
{
    DealDamage(pFrom, nDamage, elemental_type, DamageType::NormalPhysical, accuracy_bonus, critical_bonus, nFlag, damage_penalty, damage_advantage);
}

Damage Unit::CalcDamage(Unit *pTarget, DamageType damage_type, float nDamage, ElementalType elemental_type, int accuracy_bonus, float critical_amp, int critical_bonus, int nFlag)
{
    Damage result{ };
    if (damage_type == DamageType::NormalMagical || damage_type == DamageType::StateMagical) {
        // TODO how about no?
    }

    auto nDamagec = int(m_Expert[GetCreatureGroup()].fDamage * nDamage + nDamage);
    auto nDamagea = int(pTarget->m_Expert[GetCreatureGroup()].fAvoid * nDamagec + nDamagec);

    float fDefAdjustb{0}, fDefAdjustc{0}, fDefAdjust{0};
    bool bIsPhysicalDamage   = damage_type == DamageType::NormalPhysical || damage_type == DamageType::NormalPhysicalLeftHand || damage_type == DamageType::StatePhysical || damage_type ==DamageType::NormalPhysicalSkill;
    bool bIsMagicalDamage    = damage_type == DamageType::NormalMagical || damage_type == DamageType::StateMagical;
    bool bIsAdditionalDamage = damage_type == DamageType::Additional || damage_type == DamageType::AdditionalLeftHand || damage_type == DamageType::AdditionalLeftHand;
    bool bIsLeftHandDamage   = damage_type == DamageType::NormalPhysicalLeftHand || damage_type == DamageType::AdditionalLeftHand;
    bool bDefAdjust          = damage_type == DamageType::StatePhysical || damage_type == DamageType::StateMagical;

    int nAccuracy{0};
    int nPercentage{0};

    if ((nFlag & 2) == 0 && bIsPhysicalDamage && !bDefAdjust) {
        if (bIsLeftHandDamage)
            nAccuracy = (int)m_Attribute.nAccuracyLeft;
        else
            nAccuracy = (int)m_Attribute.nAccuracyRight;

        nPercentage     = 2 * ((44 - pTarget->GetLevel()) + GetLevel());
        if (nPercentage < 10)
            nPercentage = 10;

        nPercentage = (int) (nAccuracy / pTarget->m_Attribute.nAvoid * (float) nPercentage + 7.0f + accuracy_bonus);
        //nAccuracy   = (int)pTarget->m_Attribute.nAvoid;
        if((uint)rand32() % 100 > nPercentage) {
            result.bMiss = true;
            result.nDamage = 0;
            return result;
        }
    }

    if((nFlag & 2) == 0 && bIsMagicalDamage && !bDefAdjust) {
        nAccuracy = 2 * ((44 - pTarget->GetLevel()) + GetLevel());
        nPercentage =(int)(m_Attribute.nMagicAccuracy / pTarget->m_Attribute.nMagicAvoid * nAccuracy + 7.0f + accuracy_bonus);
        if(nAccuracy < 10)
            nAccuracy = 10;
        if((uint)rand32() % 100 > nPercentage) {
            result.bMiss = true;
            result.nDamage = 0;
            return result;
        }
    }

    int nRandomDamage = 0;
    float nDefence = 0;
    if(bIsAdditionalDamage && bDefAdjust) {
        float fDefAdjusta = 1.0f;
        if(bIsMagicalDamage && (m_Attribute.nMagicPoint < pTarget->m_Attribute.nMagicDefence)) {
            if(pTarget->m_Attribute.nMagicDefence <= 0)
                fDefAdjusta = 1.0f;
            else
                fDefAdjusta = pTarget->m_Attribute.nMagicDefence;
            fDefAdjusta = 1.0f - ((pTarget->m_Attribute.nMagicDefence - m_Attribute.nMagicPoint) / (2 * fDefAdjusta));
        } else if(bIsPhysicalDamage && (m_Attribute.nAttackPointRight < pTarget->m_Attribute.nDefence)) {
            if (pTarget->m_Attribute.nDefence <= 0)
                fDefAdjusta = 1;
            else
                fDefAdjusta = pTarget->m_Attribute.nDefence;
            fDefAdjusta     = 1.0f - ((pTarget->m_Attribute.nDefence - m_Attribute.nAttackPointRight) / (2 * fDefAdjusta));

        }
        fDefAdjust = (int)((float)nDamagea * fDefAdjusta);
    } else {
        fDefAdjust = nDamagea;
        if(GetSubType() != ST_SkillProp) {
            if((nFlag & 4) == 0) {
                if (bIsPhysicalDamage) {
                    nDefence = pTarget->m_Attribute.nDefence;
                    // TODO Shield
                } else if (bIsMagicalDamage) {
                    nDefence = pTarget->m_Attribute.nMagicDefence;
                }
            }
            float nDamageb = 1.0f - 0.4f * nDefence / nDamagea;
            if(nDamageb < 0.3f)
                nDamageb = 0.3f;

            fDefAdjustc = 1.0f - nDefence * 0.5f / nDamagea;
            if(fDefAdjustc < 0.05f)
                fDefAdjustc = 0.05f;

            int nDefencea = GetLevel();
            fDefAdjust = (int)((float)nDefencea * 1.7f * nDamageb + nDamagea * fDefAdjustc);
            if(fDefAdjust < 1)
                fDefAdjust = 1;
        }
        // TODO IgnoreRandomDamage
        if(true) {
            nRandomDamage = irand((int)(-(fDefAdjust * 0.05f)), (int)(fDefAdjust * 0.05f));
        }
    }

    fDefAdjustb = (fDefAdjust + nRandomDamage);
    if(!bIsAdditionalDamage && (nFlag & 0x10) == 0) {
        int cd = 0; // GetCriticalDamage
        if (cd != 0) {
            fDefAdjustb += cd;
            result.bCritical = true;
        }
    }
    if((damage_type == DamageType::Additional || damage_type == DamageType::AdditionalLeftHand) && HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon)) {
        if(bIsLeftHandDamage)
            fDefAdjustb = (int)(fDefAdjustb * (float)1/*m_nDoubleWeaponMasteryLevel*/ * 0.02f + 0.44f);
        else
            fDefAdjustb *= (int)(fDefAdjustb * (float)1/*m_nDoubleWeaponMasteryLevel*/ * 0.001f + 0.9f);
    }

    float nDamaged = 1.0f - (pTarget->m_Resist.nResist[(int)elemental_type] / 300);
    result.nDamage = (int)(nDamaged * fDefAdjustb);
    result.nResistedDamage = fDefAdjustb - result.nDamage;

    if(bIsPhysicalDamage && pTarget->GetStateByEffectType(SEF_FORCE_CHIP) != nullptr)
        result.nDamage *= 2;
    else if(bIsMagicalDamage && pTarget->GetStateByEffectType(SEF_SOUL_CHIP) != nullptr)
        result.nDamage *= 2;
    else if(pTarget->GetStateByEffectType(SEF_LUNAR_CHIP) != nullptr)
        result.nDamage *= 2;

    if((pTarget->GetSubType() == ST_Player || pTarget->GetSubType() == ST_Summon) && GetHandle() != pTarget->GetHandle()) {
        if(GetSubType() == ST_Player)
            result.nDamage = (int)((float)result.nDamage * 1 /*PVPRateForPlayer*/);
        else if(GetSubType() == ST_Summon)
            result.nDamage = (int)((float)result.nDamage * 1 /*PVPRateForSummon*/);
    }
    if(bIsMagicalDamage && !bIsAdditionalDamage && fDefAdjustb < 1)
        result.nDamage = 1;
    return result;
}

uint Unit::GetCreatureGroup()
{
    switch(_subType) {
        case ST_Player:
            return 9;
        case ST_Mob: {
            auto monster = dynamic_cast<Monster *>(this);
            return monster != nullptr ? (uint) monster->GetBase()->grp : 0;
        }
        case ST_Summon:
            return 9;
        default:
            return 0;
    }
    ACE_NOTREACHED(return 0);
}

int Unit::damage(Unit *pFrom, int nDamage, bool decreaseEXPOnDead)
{
    int result{0};
    if(GetHealth() != 0) {
        //if(HasFlag(UNIT_FIELD_STATUS, StatusFlags::Hiding))
            //1RemoveState(StateCode::Hide, 65535);

        if(GetHealth() <= nDamage)
            SetHealth(0);
        else
            AddHealth(-nDamage);

        if(GetHealth() == 0) {
            SetUInt32Value(UNIT_FIELD_DEAD_TIME, sWorld->GetArTime());
            onDead(pFrom, decreaseEXPOnDead);
        }
        result = nDamage;
    }
    return result;
}

void Unit::broadcastAttackMessage(Unit *pTarget, AttackInfo *arDamage, int tm, int delay, bool bIsDoubleAttack, bool bIsAiming, bool bEndAttack, bool bCancelAttack)
{
    XPacket pct(TS_SC_ATTACK_EVENT);
    pct << GetHandle();
    if(pTarget != nullptr)
        pct << pTarget->GetHandle();
    else
        pct << (uint)0;
    pct << (uint16)tm; // attack_speed
    pct << (uint16)delay;

    uint8 attack_flag = 0;
    if(!HasFlag(UNIT_FIELD_STATUS, StatusFlags::FormChanged)) {
        if(bIsDoubleAttack)
            attack_flag = AttackFlag::AF_DoubleAttack;
        if(HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
            attack_flag |= AttackFlag ::AF_DoubleWeapon;
        // If bow / CrossBow
    }

    uint8 attack_action = AttackAction::AA_Attack;
    if(bIsAiming)
        attack_action = AttackAction::AA_Aiming;
    else if(bEndAttack)
        attack_action = AttackAction::AA_End;
    else if(bCancelAttack)
        attack_action = AttackAction::AA_Cancel;

    pct << attack_action;
    pct << attack_flag;

    uint8 attack_count = 1;
    if(bEndAttack || bCancelAttack)
        attack_count = 0;

    pct << attack_count;
    for(int i = 0; i < attack_count; ++i) {
        pct << (uint16)arDamage[i].nDamage;
        pct << (uint16)arDamage[i].mp_damage;
        uint8 flag = 0;
        if(arDamage[i].bPerfectBlock)
            flag = AFlag::AF_PerfectBlock;
        if(arDamage[i].bBlock)
            flag |= AFlag::AF_Block;
        if(arDamage[i].bMiss)
            flag |= AFlag::AF_Miss;
        if(arDamage[i].bCritical)
            flag |= AFlag::AF_Critical;
        pct << flag;
        for(auto& ed : arDamage[i].elemental_damage) {
            pct << (uint16)ed;
        }
        pct << (int)arDamage[i].target_hp;
        pct << (uint16)arDamage[i].target_mp;
        pct << (uint16)arDamage[i].attacker_damage;
        pct << (uint16)arDamage[i].attacker_mp_damage;
        pct << (int)arDamage[i].attacker_hp;
        pct << (uint16)arDamage[i].attacker_mp;
    }
    sWorld->Broadcast((uint)GetPositionX() / g_nRegionSize, (uint)GetPositionY() / g_nRegionSize, GetLayer(), pct);
}

void Unit::EndAttack()
{
    AttackInfo info[4]{};
    uint ct = sWorld->GetArTime();

    if(HasFlag(UNIT_FIELD_STATUS, StatusFlags::AttackStarted)) {
        //auto target = dynamic_cast<Unit*>(sMemoryPool->getPtrFromId(GetTargetHandle()));
        auto target = sMemoryPool->GetObjectInWorld<Unit>(GetTargetHandle());
        if(GetSubType() == ST_Player || GetSubType() == ST_Summon) {
            if(target != nullptr)
                broadcastAttackMessage(target, info, 0, 0, false, false, true, false);
        }
    }
    SetUInt32Value(BATTLE_FIELD_TARGET_HANDLE, 0);
    SetFlag(UNIT_FIELD_STATUS, StatusFlags::FirsAttack);
}

void Unit::onDead(Unit *pFrom, bool decreaseEXPOnDead)
{
    Position pos{};

    if(m_castingSkill != nullptr) {
        CancelSkill();
    }
    if(bIsMoving && IsInWorld()) {
        pos = GetCurrentPosition(GetUInt32Value(UNIT_FIELD_DEAD_TIME));
        sWorld->SetMove(this, pos, pos, 0, true, sWorld->GetArTime(), true);
        if(GetSubType() == ST_Player) {
            // Ride handle
        }
    }
    if(GetTargetHandle() != 0)
        EndAttack();
}

void Unit::AddEXP(uint64 exp, uint jp, bool bApplyStanima)
{
    SetUInt64Value(UNIT_FIELD_EXP, GetEXP() + exp);
    SetUInt32Value(UNIT_FIELD_JOBPOINT, GetJP() + jp);
    // SetTotalJP
    if(HasFlag(UNIT_FIELD_STATUS, StatusFlags::LoginComplete))
        onExpChange();
}

void Unit::CancelSkill()
{
    if(m_castingSkill != nullptr && m_castingSkill->Cancel()) {
        m_castingSkill = nullptr;
    }
}

void Unit::CancelAttack()
{
    AttackInfo info[4]{};
    if(HasFlag(UNIT_FIELD_STATUS, StatusFlags::AttackStarted)) {
        this->broadcastAttackMessage(sMemoryPool->GetObjectInWorld<Unit>(GetTargetHandle()), info, 0, 0, false, false, false, true);
    }
    SetUInt32Value(BATTLE_FIELD_TARGET_HANDLE, 0);
    SetFlag(UNIT_FIELD_STATUS, StatusFlags::FirsAttack);
}

bool Unit::TranslateWearPosition(ItemWearType &pos, Item *item, std::vector<int> &ItemList)
{
    bool result;
    if(item->GetWearType() != ItemWearType::WearCantWear && item->IsWearable()) {
        int elevel = m_nUnitExpertLevel;
        int level = GetLevel();
        if(m_nUnitExpertLevel <= level)
            elevel = level;
        result = (item->GetLevelLimit() <= elevel) && ((item->m_pItemBase->use_min_level == 0 || level >= item->m_pItemBase->use_min_level)
                                                      && (item->m_pItemBase->use_max_level == 0 || level <= item->m_pItemBase->use_max_level));
    } else {
        result = false;
    }
    return result;
}

uint16_t Unit::putonItem(ItemWearType pos, Item *item)
{
    m_anWear[pos] = item;
    item->m_Instance.nWearInfo = pos;
    item->m_bIsNeedUpdateToDB = true;
    // Binded target
    if(GetSubType() == ST_Player) {
        auto p = dynamic_cast<Player*>(this);
        p->SendItemWearInfoMessage(item, this);
    }
    return 0;
}

ushort Unit::Puton(ItemWearType pos, Item *item)
{
    if(item->m_Instance.nWearInfo != ItemWearType::WearCantWear)
        return 0;

    std::vector<int> vOverlapItemList{ };
    if(!TranslateWearPosition(pos, item, vOverlapItemList))
        return 0;

    for(int& s : vOverlapItemList) {
        putoffItem((ItemWearType)s);
        if(m_anWear[s] != nullptr)
            return 0;
    }
    return putonItem(pos, item);
}

uint16_t Unit::putoffItem(ItemWearType pos)
{
    auto item = m_anWear[pos];

    item->m_Instance.nWearInfo = ItemWearType::WearNone;
    item->m_bIsNeedUpdateToDB = true;
    // Binded Target
    m_anWear[pos] = nullptr;
    if(GetSubType() == ST_Player) {
        auto p = dynamic_cast<Player*>(this);
        p->SendItemWearInfoMessage(item, this);
    }
    // Todo: Summon
    return 0;
}

ushort Unit::Putoff(ItemWearType pos)
{
    int i;

    if(pos == ItemWearType::WearTwoHand)
        pos = ItemWearType::WearWeapon;
    if(pos == ItemWearType::WearTwoFingerRing)
        pos = ItemWearType::WearRing;
    if(pos >= 24 || pos < 0)
        return 5;
    ItemWearType abspos = GetAbsoluteWearPos(pos);
    if(abspos == ItemWearType::WearCantWear)
        return 5;
    if(pos != ItemWearType::WearBagSlot)
        return putoffItem(abspos);

    // TODO: Bag
}

ItemWearType Unit::GetAbsoluteWearPos(ItemWearType pos)
{
    ItemWearType result = pos;
    if(m_anWear[pos] == nullptr)
        result = ItemWearType::WearCantWear;
    return result;
}

void Unit::applyPassiveSkillEffect(Skill *skill)
{
    float atk = 0;
    switch(skill->m_SkillBase->effect_type) {
        case EffectType::WeaponMastery: {
            auto weapon = GetWornItem(ItemWearType::WearWeapon);
            if (weapon == nullptr)
                return;

            if (skill->m_SkillBase->id == SkillId::AdvWeaponExpert)
                if (weapon->GetItemRank() < 2)
                    return;
            atk = (skill->m_SkillBase->var[0] + (skill->m_SkillBase->var[1] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel)));
            m_Attribute.nAttackPointRight += atk;
            if (HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
                m_Attribute.nAttackPointLeft += atk;

            m_Attribute.nAttackSpeed += (skill->m_SkillBase->var[2] + (skill->m_SkillBase->var[3] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel)));
            m_Attribute.nMagicPoint += (skill->m_SkillBase->var[6] + (skill->m_SkillBase->var[7] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel)));
        }
            break;
        case EffectType::IncreaseBaseAttribute: {
            atk = (skill->m_SkillBase->var[0] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel));
            m_Attribute.nAttackPointRight += atk;
            if (HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
                m_Attribute.nAttackPointLeft += atk;

            m_Attribute.nDefence      += (skill->m_SkillBase->var[1] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel));
            m_Attribute.nMagicPoint   += (skill->m_SkillBase->var[2] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel));
            m_Attribute.nMagicDefence += (skill->m_SkillBase->var[3] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel));

            atk = (skill->m_SkillBase->var[6] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel));
            m_Attribute.nAccuracyRight += atk;
            if (HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
                m_Attribute.nAccuracyLeft += atk;
            m_Attribute.nMagicAccuracy += (skill->m_SkillBase->var[7] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel));
        }
            break;
        case EffectType::IncreaseHPMP: {
            SetMaxHealth((uint) (GetMaxHealth() + skill->m_SkillBase->var[0] + (skill->m_SkillBase->var[1] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel))));
            SetMaxMana((uint) (GetMaxMana() + skill->m_SkillBase->var[2] + (skill->m_SkillBase->var[3] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel))));
        }
            break;
        case EffectType::EF_WEAPON_TRAINING: {
            m_Attribute.nAttackSpeedRight += (skill->m_SkillBase->var[3] * 100 * skill->m_nSkillLevel);
        }
        break;
        default:
            //MX_LOG_DEBUG("entities.unit", "Unknown SKill Effect Type Unit::applyPassiveSkillEffect: %u", skill->m_SkillBase->id);
            break;
    }

    // SPECIAL CASES
    switch(skill->m_nSkillID) {
        case 1202: // Defense Practice
            m_Attribute.nDefence += (skill->m_SkillBase->var[0] * (skill->m_nSkillLevelAdd + skill->m_nSkillLevel));
            break;
        default:
            break;
    }
}

void Unit::applyPassiveSkillEffect()
{
    for(auto& s : m_vSkillList) {
        // yes, is_passive == 0 to get passive skills.. Gala :shrug:
        if(s != nullptr && s->m_SkillBase != nullptr && s->m_SkillBase->is_passive == 0) {
            bool r = s->m_SkillBase->vf_shield_only == 0 ? s->m_SkillBase->IsUseableWeapon(GetWeaponClass()) : IsWearShield();
            if(s->m_SkillBase->vf_is_not_need_weapon != 0 || r)
                applyPassiveSkillEffect(s);
        }
    }
}

ItemClass Unit::GetWeaponClass()
{
    ItemClass result = ItemClass::ClassEtc;
    auto itemRight = GetWornItem(ItemWearType::WearRightHand);

    if (itemRight != nullptr)
    {
        if(HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
        {
            Item* itemLeft = GetWornItem(ItemWearType::WearLeftHand);
            result = (ItemClass)itemRight->m_pItemBase->iclass;
            if (itemRight->m_pItemBase->iclass == ItemClass::ClassOneHandSword && itemLeft->m_pItemBase->iclass == ItemClass::ClassOneHandSword)
                return ItemClass::ClassDoubleSword;
            if (itemRight->m_pItemBase->iclass == ItemClass::ClassDagger && itemLeft->m_pItemBase->iclass == ItemClass::ClassDagger)
                return ItemClass::ClassDoubleDagger;
            if (itemRight->m_pItemBase->iclass== ItemClass::ClassOneHandAxe && itemLeft->m_pItemBase->iclass == ItemClass::ClassOneHandAxe)
                return ItemClass::ClassDoubleAxe;
        }
        result = (ItemClass)itemRight->m_pItemBase->iclass;
    }
    return result;
}

bool Unit::IsWearShield()
{
    bool result{false};
    auto item = GetWornItem(ItemWearType::WearShield);
    if(item != nullptr)
        result = item->m_pItemBase->iclass == ItemClass::ClassShield;
    else
        result = false;
    return result;
}

float Unit::GetItemChance() const
{
    return m_Attribute.nItemChance;
}

void Unit::applyDoubeWeaponEffect()
{
    float fAddPerLevel{0};
    Skill* skill{nullptr};

    if(HasFlag(UNIT_FIELD_STATUS, StatusFlags::UsingDoubleWeapon))
    {
        return;
    }
    m_Attribute.nAttackSpeed = m_Attribute.nAttackSpeedRight;
    m_AttributeByState.nAttackSpeed= m_AttributeByState.nAttackSpeedRight;
}

uint16 Unit::AddState(StateType type, StateCode code, uint caster, int level, uint start_time, uint end_time, bool bIsAura, int nStateValue, std::string szStateValue)
{
    SetFlag(UNIT_FIELD_STATUS, StatusFlags::NeedToUpdateState);
    auto stateInfo = sObjectMgr->GetStateInfo(code);
    if (stateInfo == nullptr) {
        return TS_RESULT_NOT_EXIST;
    }
    if (GetHealth() == 0 && (stateInfo->state_time_type & 0x81) != 0)
        return TS_RESULT_NOT_ACTABLE;

    if ((stateInfo->state_time_type & 8) != 0 && GetSubType() == ST_Mob && false /* Connector/AutoTrap */) {
        return TS_RESULT_LIMIT_TARGET;
    } /*else if (code != StateCode::SC_SLEEP && code != StateCode::SC_NIGHTMARE && code != StateCode::SC_SEAL
               && code != StateCode::SC_SHINE_WALL && code != StateCode::SC_STUN && stateInfo->effect_type != 104) {
        if (stateInfo->effect_type != 82 || stateInfo->value[0] == 0.0f && stateInfo->value[1] == 0.0f
                                            && stateInfo->value[2] == 0.0f && stateInfo->value[3] == 0.0f)

    }*/
    if(code == StateCode::SC_FEAR)
        ToggleFlag(UNIT_FIELD_STATUS, StatusFlags::MovingByFear);

    //auto pCaster = dynamic_cast<Unit*>(sMemoryPool->getPtrFromId(caster));
    auto pCaster = sMemoryPool->GetObjectInWorld<Unit>(caster);
    int base_damage = 0;
    if(pCaster != nullptr && stateInfo->base_effect_id > 0) {
        // DAMAGES WOHOOOOOOOO
    }

    bool bNotErasable = ((stateInfo->state_time_type >> 4) & 1) != 0;
    std::vector<uint16> vDeleteStateUID{};
    bool bAlreadyExist{false};

    for(auto& s : m_vStateList) {
        if(code == s.m_nCode) {
            bAlreadyExist = true;
        } else {
            bool bf = false;
            for(int i = 0; i < 3; ++i) {
                if(s.IsDuplicatedGroup(stateInfo->duplicate_group[i])) {
                    bf = true;
                    break;
                }
            }
            if(!bf)
                continue;
        }
        if(bNotErasable != (((s.GetTimeType() >> 4 ) & 1 ) != 0)) {
            if(bNotErasable)
                return TS_RESULT_ALREADY_EXIST;
            vDeleteStateUID.emplace_back(s.m_nUID);
        } else {
            if(s.GetLevel() > level)
                return TS_RESULT_ALREADY_EXIST;

            if(s.GetLevel() == level) {
                uint et = s.m_nEndTime[1];
                if(s.m_nEndTime[0] > et)
                    et = s.m_nEndTime[0];
                if(et > end_time)
                    return TS_RESULT_ALREADY_EXIST;
            }
            if(code != s.m_nCode)
                vDeleteStateUID.emplace_back(s.m_nUID);
        }
    }

    for(auto& id : vDeleteStateUID) {
        for(int i = (int)m_vStateList.size() - 1; i >= 0; --i) {
            State s = m_vStateList[i];
            if(id == s.m_nUID) {
                m_vStateList.erase(m_vStateList.begin() + i);
                CalculateStat();
                break;
            }
        }
    }
    if(bAlreadyExist) {
        for(auto& s : m_vStateList) {
            if(code == s.m_nCode) {
                s.AddState(type, caster, (uint16)level, start_time, end_time, base_damage, bIsAura);
                CalculateStat();
                onUpdateState(s, false);
                onAfterAddState(s);
                break;
            }
        }
    } else {
        m_nCurrentStateUID++;
        State ns{type, code, (int)m_nCurrentStateUID, caster, (uint16)level, start_time, end_time, base_damage, false, nStateValue, std::move(szStateValue)};
        m_vStateList.emplace_back(ns);
        CalculateStat();

        onUpdateState(ns, false);
        if(GetSubType() == ST_Mob && !HasFlag(UNIT_FIELD_STATUS, StatusFlags::Movable)) {
            if(m_Attribute.nAttackRange < 84)
                m_Attribute.nAttackRange = 83;
        }
        onAfterAddState(ns);
    }
    return TS_RESULT_SUCCESS;
}

void Unit::onAfterAddState(State)
{
    procMoveSpeedChange();
}

void Unit::procMoveSpeedChange()
{
    std::vector<Position> vMovePos{};

    if(bIsMoving && IsInWorld()) {
        uint ct = sWorld->GetArTime();
        auto pos = GetCurrentPosition(ct);
        if(HasFlag(UNIT_FIELD_STATUS, StatusFlags::Movable)) {
            if(speed != m_Attribute.nMoveSpeed / 7) {
                 for(auto mi : ends)
                     vMovePos.emplace_back(mi.end);
                sWorld->SetMultipleMove(this, pos, vMovePos, (uint8)(m_Attribute.nMoveSpeed / 7), true, ct, true);
            }
        } else {
            sWorld->SetMove(this, pos, pos, 0, true, ct, true);
        }
    }
}

void Unit::onUpdateState(State state, bool bIsExpire)
{
    Messages::BroadcastStateMessage(this, state, bIsExpire);
}

uint16 Unit::onItemUseEffect(Unit *pCaster, Item* pItem, int type, float var1, float var2, const std::string &szParameter)
{
    uint16 result{TS_RESULT_ACCESS_DENIED};
    uint target_handle{0};
    Position pos{};
    switch(type) {
        case 5:
            target_handle = GetHandle();
            if(var1 == 6020.0f || var1 == 6021.0f )
                target_handle = pItem->GetHandle();
            pos.Relocate(0.0f, 0.0f, 0.0f, 0.0f);
            return (uint16)pCaster->CastSkill((int)var1, (int)var2, target_handle, pos, GetLayer(), true);
        default:
            result = TS_RESULT_UNKNOWN;
        break;
    }
    return result;
}

State *Unit::GetStateByEffectType(int effectType) const
{
    for (int i = 0; i < m_vStateList.size(); i++) {
        if (m_vStateList[i].GetEffectType() == effectType)
            return (State*)&m_vStateList[i];
    }
    return nullptr;
}

std::pair<float, int> Unit::GetHateMod(int nHateModType, bool bIsHarmful)
{
    float fAmpValue = 1.0f;
    int nIncValue = 0;

    for(auto& hm : m_vHateMod) {
        if(bIsHarmful) {
            if(!hm.bIsApplyToHarmful)
                continue;
        } else {
            if(!hm.bIsApplyToHelpful)
                continue;
        }

        if((nHateModType == 1 && hm.bIsApplyToPhysicalSkill) || (nHateModType == 2 && hm.bIsApplyToMagicalSkill) || (nHateModType == 33 && hm.bIsApplyToPhysicalAttack)) {
            fAmpValue += hm.fAmpValue;
            nIncValue += hm.nIncValue;
        }
    }
    return {fAmpValue, nIncValue};
}

bool Unit::ClearExpiredState(uint t)
{
    bool bDeleted{false};
    for(int i = 0; i < m_vStateList.size(); i++) {
        uint et = m_vStateList[i].m_nEndTime[1];
        if(m_vStateList[i].m_nEndTime[0] > et)
            et = m_vStateList[i].m_nEndTime[0];
        if(et < t) {
            //RemoveState(it);
            Messages::BroadcastStateMessage(this, m_vStateList[i], true);
            m_vStateList.erase(m_vStateList.begin() + i);
            bDeleted = true;
        }
    }
    return bDeleted;
}

int Unit::GetAttackPointRight(ElementalType type, bool bPhysical, bool bBad) const
{
    float v4{1};
    float v5 = m_Attribute.nAttackPointRight;

    // TODO: ElementalStateMod

    return (int)(v5 * v4);
}

DamageInfo Unit::DealMagicalSkillDamage(Unit *pFrom, int nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag)
{
    DamageInfo result{};
    auto d = DealMagicalDamage(pFrom, (float)nDamage, elemental_type, 0, critical_bonus, nFlag, nullptr, nullptr);
    result.SetDamage(d);
    result.target_hp = GetHealth();
    return result;
}

DamageInfo Unit::DealPhysicalSkillDamage(Unit *pFrom, int nDamage, ElementalType elemental_type, int accuracy_bonus, int critical_bonus, int nFlag)
{
    DamageInfo result{};
    auto d = DealPhysicalDamage(pFrom, (float)nDamage, elemental_type, 0, critical_bonus, nFlag, nullptr, nullptr);
    result.SetDamage(d);
    result.target_hp = GetHealth();
    return result;
}

Damage Unit::DealMagicalDamage(Unit *pFrom, float nDamage, ElementalType type, int accuracy_bonus, int critical_bonus, int nFlag, StateMod *damage_penalty, StateMod *damage_advantage)
{
    return DealDamage(pFrom, nDamage, type, DamageType::NormalMagical, accuracy_bonus, critical_bonus, nFlag, damage_penalty, damage_advantage);
}

uint Unit::GetRemainCoolTime(int skill_id) const
{
    uint ct = sWorld->GetArTime();
    auto sk = GetSkill(skill_id);
    if(sk == nullptr || sk->m_nNextCoolTime < ct)
        return 0;
    return sk->m_nNextCoolTime - ct;
}

uint Unit::GetTotalCoolTime(int skill_id) const
{
    auto sk = GetSkill(skill_id);
    if (sk == nullptr)
        return 0;
    else
        return sk->GetSkillCoolTime();
}

void Unit::SetJP(int jp)
{
    if(jp < 0)
        jp = 0;
    SetInt32Value(UNIT_FIELD_JOBPOINT, jp);
    if(HasFlag(UNIT_FIELD_STATUS, StatusFlags::LoginComplete))
        onExpChange();
}

void Unit::SetEXP(uint exp)
{
    SetUInt64Value(UNIT_FIELD_EXP, exp);
    if(HasFlag(UNIT_FIELD_STATUS, StatusFlags::LoginComplete))
        onExpChange();
}
