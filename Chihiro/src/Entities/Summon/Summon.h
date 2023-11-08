#pragma once
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
#include "Common.h"
#include "SummonBase.h"
#include "Unit.h"

class Player;
struct TS_SC_ENTER;

class Summon : public Unit {
public:
    friend class Player;
    static Summon *AllocSummon(Player *, uint32_t);
    explicit Summon(uint32_t, uint32_t);
    // Deleting the copy & assignment operators
    // Better safe than sorry
    Summon(const Summon &) = delete;
    Summon &operator=(const Summon &) = delete;
    ~Summon();

    static void DB_InsertSummon(Player *, Summon *);
    static void DB_UpdateSummon(Player *, Summon *);
    static void EnterPacket(TS_SC_ENTER &, Summon *, Player *pPlayer);

    CreatureStat *GetBaseStat() const override;
    inline uint32_t GetSummonSID() const { return GetUInt32Value(UNIT_FIELD_UID); }

    bool IsBattleMode() const override { return m_bIsBattleMode; }
    void SetBattleModeOn() { m_bIsBattleMode = true; }
    void SetBattleModeOff() { m_bIsBattleMode = false; }

    uint32_t GetCreatureGroup() const override { return 9; }

    int32_t GetTransformLevel() const { return m_tSummonBase->type; }
    int32_t GetEvolveTarget() const { return m_tSummonBase->evolve_target; }


    void OnAfterReadSummon();
    uint32_t GetCardHandle();

    bool IsSummon() const override { return true; }

    bool IsAlly(const Unit *pTarget) override;

    int32_t GetSummonCode();
    float_t GetFCM() const override;
    SUMMON_RIDE_TYPE GetRidingInfo();

    Player *GetMaster() const { return m_pMaster; }

    void Update(uint32_t /*diff*/) override;

    bool TranslateWearPosition(ItemWearType &pos, Item *item, std::vector<int32_t> *ItemList) override;

    float_t GetSize() const override;
    float_t GetScale() const override;
    int32_t GetSP() const { return m_nSP; }
    int32_t GetMaxSP() const { return m_nMaxSP; }
    int32_t AddSP(int SP) { return SetSP(GetSP() + SP); }
    int32_t SetSP(int SP)
    {
        int32_t oldSP = m_nSP;
        m_nSP = SP;
        if (m_nSP > m_nMaxSP)
            m_nSP = m_nMaxSP;
        if (m_nSP < 0)
            m_nSP = 0;
        if (oldSP != m_nSP)
            onSPChange();
        return m_nSP;
    }

    void SetSummonInfo(int32_t);
    bool DoEvolution(int32_t nTargetCode = 0);

    int32_t m_nSummonInfo{};
    int32_t m_nCardUID{};
    int32_t m_nTransform{};
    uint8_t m_cSlotIdx{};
    Item *m_pItem{nullptr};

protected:
    void onCantAttack(uint32_t handle, uint32_t t) override;
    uint16_t putonItem(ItemWearType pos, Item *pItem) override;
    uint16_t putoffItem(ItemWearType pos) override;
    void onRegisterSkill(int64_t skillUID, int32_t skill_id, int32_t prev_level, int32_t skill_level) override;
    void processWalk(uint32_t t);
    void onExpChange() override;
    ///- Stat calculations overwriting from Unit class
    void onBeforeCalculateStat() override;
    void applyJobLevelBonus() override;
    void onUpdateState(State *state, bool bIsExpire) override;
    void onAfterRemoveState(State *state, bool bByDead = false) override;
    using Unit::applyPassiveSkillEffect; // -Woverloaded-virtual, honestly I have no idea if this is correct, but it works
    void applyPassiveSkillEffect() override;
    using Unit::applyState; // -Woverloaded-virtual, honestly I have no idea if this is correct, but it works
    void applyState(State &state) override;
    using Unit::applyPassiveSkillAmplifyEffect; // -Woverloaded-virtual, honestly I have no idea if this is correct, but it works
    void applyPassiveSkillAmplifyEffect() override;
    void onModifyStatAndAttribute() override;
    void onItemWearEffect(Item *pItem, bool bIsBaseVar, int32_t type, float_t var1, float_t var2, float_t fRatio) override;
    void onCompleteCalculateStat() override;
    void applyStatByState() override;
    void onApplyStat() override;
    void onSPChange() override;

private:
    SummonResourceTemplate *m_tSummonBase{nullptr};
    Player *m_pMaster{nullptr};

    int32_t m_nAccountID{};
    float_t m_fBaseAttackPointRatio{};
    float_t m_fBaseMagicPointRatio{};
    float_t m_fBaseDefenceRatio{};
    float_t m_fBaseMagicDefenceRatio{};
    bool m_bIsBattleMode{false};
    int32_t m_nSP{};
    int32_t m_nMaxSP{};
};