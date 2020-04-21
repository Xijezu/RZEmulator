#include "Roamer.h"
#include "MemPool.h"
#include "World.h"
/*
Roamer::Roamer(const int32_t nID, const Roamer::ROAMING_TYPE eRoamingType, const int32_t nMoveSpeed, const HATE_TYPE eHateType, const uint32_t nRespawnInterval, const int32_t nAttributeFlag, const bool bIsRaidDungeonRoamer)
    : WorldObject(true), m_nID(nID), m_nMoveSpeed(nMoveSpeed), m_eRoamingType(eRoamingType), m_eHateType(eHateType), m_nRespawnInterval(nRespawnInterval), m_bIsRaidDungeonRoamer(bIsRaidDungeonRoamer), m_eCurrentRoamingDirection(ROAMING_DIRECTION_FORWARD), m_nCurrentRoamingPointIndex(0), m_eRoamingStatus(ROAMING_STATUS_IDLE), m_nNextRespawnProcTime(0), m_nLastRegenCount(0)
{
    //m_AttributeFlag = nAttri( &nAttributeFlag );
    _mainType = MT_StaticObject;
    _subType = ST_Object;
    _objType = OBJ_MOVABLE;

    _valuesCount = UNIT_FIELD_HANDLE + 1;
    _InitValues();
}

void Roamer::AddCreatureRespawnInfo(const ROAMING_CREATURE_RESPAWN_INFO &info)
{
    m_vRoamingCreatureRespawnInfo.emplace_back(info);
}

void Roamer::AddRoamingPoint(const Position &pos)
{
    if (IsInitialized() || IsInWorld())
    {
        return;
    }

    m_vRoamingPoint.emplace_back(pos);
    switch (m_vRoamingPoint.size())
    {
    case 1:
        SetCurrentXY(pos.GetPositionX(), pos.GetPositionY());
        break;
    case 2:
        SetDirection(pos);
        break;
    default:
        break;
    }
}

void Roamer::DeleteRespawnedCreature()
{
    m_nNextRespawnProcTime = 0;
    for (auto &it : m_vRoamingCreatureRespawnInfo)
    {
        auto pUnit = it.m_pUnit;
        if (it.m_pUnit == nullptr)
            continue;

        it.m_nNextRespawnTime = 0;
        it.m_pUnit = nullptr;

        if (pUnit->IsInWorld())
        {
            switch (it.m_eCreatureType)
            {
            case ROAMING_CREATURE_MONSTER:
            {
                auto pMonster = pUnit->As<Monster>();
                if (pMonster->IsInWorld())
                    sWorld.RemoveObjectFromWorld(pMonster);
                pMonster->m_pDeleteHandler = nullptr;
                pMonster->DeleteThis();
                break;
            }
            case ROAMING_CREATURE_NPC:
            {
                
                break;
            }
            default:
                break;
            }
        }
    }
}

const bool Roamer::Init()
{
    if (IsInitialized())
        return false;

    m_nCurrentRoamingPointIndex = 0;
    m_eCurrentRoamingDirection = ROAMING_DIRECTION_FORWARD;
    SetCurrentXY(m_vRoamingPoint.front().GetPositionX(), m_vRoamingPoint.front().GetPositionY());

    m_eRoamingStatus = ROAMING_STATUS_ROAMING;
    if (m_nRespawnInterval != 0)
    {
        m_nNextRespawnProcTime = sWorld.GetArTime();
        m_nLastRegenCount = 0;
    }
    else
    {
        for (auto &it : m_vRoamingCreatureRespawnInfo)
            it.m_nNextRespawnTime = sWorld.GetArTime();
    }

    sWorld.AddObjectToWorld(this);
    return true;
}

const bool Roamer::DeInit(const bool bForceToDeleteEverlastingRoamer)
{
    for (auto &itHate : m_vPendingHateInfo)
    {
        delete itHate;
    }
    m_vPendingHateInfo.clear();

    DeleteRespawnedCreature();

    if (IsInWorld())
        sWorld.RemoveObjectFromWorld(this);

    if (m_bIsRaidDungeonRoamer && !bForceToDeleteEverlastingRoamer)
        return true;

    // @todo: Delete this
}
*/