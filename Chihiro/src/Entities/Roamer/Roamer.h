#pragma once
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

#include "Common.h"
#include "Monster.h"

struct ROAMING_CREATURE_RESPAWN_INFO
{
    ROAMING_CREATURE_RESPAWN_INFO(const int32_t eCreatureType, const int32_t nCreatureID, const uint32_t nRespawnInterval, const int32_t nAngle, const uint32_t nDistance)
        : m_eCreatureType(eCreatureType), m_nCreatureID(nCreatureID), m_nRespawnInterval(nRespawnInterval), m_nAngle(nAngle), m_nDistance(nDistance)
    {
    }

    int32_t m_eCreatureType;
    int32_t m_nCreatureID;
    uint32_t m_nRespawnInterval;
    int32_t m_nAngle;
    uint32_t m_nDistance;
};

struct Roamer : public WorldObject, MonsterDeleteHandler
{
  public:
    enum ROAMING_TYPE
    {
        ROAMING_TYPE_ROUND = 0,
        ROAMING_TYPE_GO_BACK = 1,
    };

    enum HATE_TYPE
    {
        HATE_TYPE_INDIVIDUAL = 1 << 0,
        HATE_TYPE_SHARE_FIRST = 1 << 1,
        HATE_TYPE_FULL_SHARE = 1 << 2,
    };

    enum ROAMING_CREATURE_TYPE
    {
        ROAMING_CREATURE_MONSTER = 0,
        ROAMING_CREATURE_NPC = 1,
    };

  protected:
    enum ROAMING_DIRECTION
    {
        ROAMING_DIRECTION_FORWARD = 0,
        ROAMING_DIRECTION_BACKWARD = 1
    };

    enum ROAMING_STATUS
    {
        ROAMING_STATUS_IDLE = 0,
        ROAMING_STATUS_ROAMING = 1,
        ROAMING_STATUS_ROTATING = 2,
        ROAMING_STATUS_PAUSED = 3,
    };

    struct ROAMING_CREATURE_INFO : public ROAMING_CREATURE_RESPAWN_INFO
    {
        ROAMING_CREATURE_INFO(const ROAMING_CREATURE_TYPE eCreatureType, const int32_t nCreatureID, const uint32_t nRespawnInterval, const int32_t nAngle, const uint32_t nDistance)
            : ROAMING_CREATURE_RESPAWN_INFO(eCreatureType, nCreatureID, nRespawnInterval, nAngle, nDistance), m_pUnit(nullptr), m_nNextRespawnTime(0)
        {
        }

        ROAMING_CREATURE_INFO(const ROAMING_CREATURE_RESPAWN_INFO &info)
            : ROAMING_CREATURE_RESPAWN_INFO(info), m_pUnit(nullptr), m_nNextRespawnTime(0)
        {
        }

        Unit *m_pUnit;
        uint32_t m_nNextRespawnTime;
    };

    struct PENDING_HATE_SHARE_INFO
    {
        PENDING_HATE_SHARE_INFO(const uint32_t hRequester, const uint32_t hHateTarget, const int32_t nHate)
            : m_hRequester(hRequester), m_hHateTarget(hHateTarget), m_nHate(nHate)
        {
        }

        uint32_t m_hRequester;
        uint32_t m_hHateTarget;
        int32_t m_nHate;
    };

  public:
    Roamer(const int32_t nID, const ROAMING_TYPE eRoamingType, const int32_t nMoveSpeed, const HATE_TYPE eHateType, const uint32_t nRespawnInterval, const int32_t nAttributeFlag, const bool bIsRaidDungeonRoamer);
    ~Roamer();

    virtual bool IsRoamer() const { return true; }

    const int32_t GetRoamingID() { return m_nID; }

    void AddCreatureRespawnInfo(const ROAMING_CREATURE_RESPAWN_INFO &info);
    const size_t GetCreatureRespawnInfoCount() { return m_vRoamingCreatureRespawnInfo.size(); }

    void AddRoamingPoint(const Position &pos);
    const size_t GetRoamingPointCount() { return m_vRoamingPoint.size(); }

    void DeleteRespawnedCreature();

    const bool Init();
    const bool DeInit(const bool bForceToDeleteEverlastingRoamer);
    const bool IsInitialized() { return m_bisInitialized; }

    virtual void onMonsterDelete(struct StructMonster *pMonster);
    virtual void onNPCDead(struct StructNPC *pNPC);

    virtual bool IsMovable();
    const bool IsRaidDungeonRoamer() const { return m_bIsRaidDungeonRoamer; }
    const bool IsPaused() { return m_eRoamingStatus == ROAMING_STATUS_PAUSED; }
    void PauseRoaming();

    const Position GetCurrentRoamingTargetPosition();
    const Position GetNextRoamingTargetPosition();

    void PendHateShare(const uint32_t hRequester, const uint32_t hHateTarget, const int32_t nHate, const int32_t eApplyHateType = HATE_TYPE_FULL_SHARE);

  protected:
    const bool isMovable();
    void processWalk(uint32_t t);
    void processRoaming(uint32_t t, const Position &currentPos, const float &fFace);
    void processHateSharing();

    const size_t getNextRoamingTargetIndex() const;
    void proceedRoamingTargetIndex();

    static const Position getCurrentRespawnObjectPosition(const Position &currentPos, const float &fFace, const int32_t &nAngle, const uint32_t &nDistance);

  private:
    const int32_t m_nID;
    const int32_t m_nMoveSpeed;

    ROAMING_TYPE m_eRoamingType;
    HATE_TYPE m_eHateType;
    uint32_t m_nRespawnInterval;

    ROAMING_DIRECTION m_eCurrentRoamingDirection;
    size_t m_nCurrentRoamingPointIndex;
    ROAMING_STATUS m_eRoamingStatus;
    const bool m_bIsRaidDungeonRoamer;

    std::vector<Position> m_vRoamingPoint{};
    std::vector<ROAMING_CREATURE_INFO> m_vRoamingCreatureRespawnInfo{};
    uint32_t m_nNextRespawnProcTime;
    int32_t m_nLastRegenCount;
    bool m_bisInitialized;

    std::vector<PENDING_HATE_SHARE_INFO *> m_vPendingHateInfo{};
};
*/