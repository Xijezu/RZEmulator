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

#include "FieldProp.h"
#include "Log.h"
#include "MemPool.h"
#include "Messages.h"
#include "ObjectMgr.h"
#include "Player.h"
#include "Scripting/XLua.h"
#include "World.h"

void FieldProp::EnterPacket(XPacket &pEnterPct, FieldProp *pFieldProp, Player * /*pPlayer*/)
{
    pEnterPct << (uint32_t)pFieldProp->m_PropInfo.nPropID;
    pEnterPct << pFieldProp->m_PropInfo.fZOffset;
    pEnterPct << pFieldProp->m_PropInfo.fRotateX;
    pEnterPct << pFieldProp->m_PropInfo.fRotateY;
    pEnterPct << pFieldProp->m_PropInfo.fRotateZ;
    pEnterPct << pFieldProp->m_PropInfo.fScaleX;
    pEnterPct << pFieldProp->m_PropInfo.fScaleY;
    pEnterPct << pFieldProp->m_PropInfo.fScaleZ;
    pEnterPct << (uint8_t)(pFieldProp->m_PropInfo.bLockHeight ? 1 : 0);
    pEnterPct << pFieldProp->m_PropInfo.fLockHeight;
}

FieldProp *FieldProp::Create(FieldPropDeleteHandler *propDeleteHandler, FieldPropRespawnInfo pPropInfo, uint32_t lifeTime)
{
    auto fp = new FieldProp{propDeleteHandler, pPropInfo};
    fp->nLifeTime = lifeTime;
    fp->SetCurrentXY(pPropInfo.x, pPropInfo.y);
    fp->m_PropInfo.layer = pPropInfo.layer;
    sWorld.AddObjectToWorld(fp);
    return fp;
}

bool FieldProp::IsUsable(Player *pPlayer) const
{
    if (pPlayer == nullptr)
        return false;
    if (m_bIsCasting)
        return false;

    // Level checks
    if (pPlayer->GetLevel() < m_pFieldPropBase->nMinLevel)
        return false;
    if (pPlayer->GetLevel() > m_pFieldPropBase->nMaxLevel)
        return false;

    bool bResult = true;

    // Class checks
    if ((m_pFieldPropBase->nLimit & static_cast<int32_t>(LIMIT_FLAG::LIMIT_HUNTER)) != 0 && !pPlayer->IsHunter())
        bResult = false;
    if ((m_pFieldPropBase->nLimit & static_cast<int32_t>(LIMIT_FLAG::LIMIT_FIGHTER)) != 0 && !pPlayer->IsFighter())
        bResult = false;
    if ((m_pFieldPropBase->nLimit & static_cast<int32_t>(LIMIT_FLAG::LIMIT_MAGICIAN)) != 0 && !pPlayer->IsMagician())
        bResult = false;
    if ((m_pFieldPropBase->nLimit & static_cast<int32_t>(LIMIT_FLAG::LIMIT_SUMMONER)) != 0 && !pPlayer->IsSummoner())
        bResult = false;
    if (!bResult)
        return false;

    // Race checks
    if (pPlayer->GetRace() != 3 && (m_pFieldPropBase->nLimit & static_cast<int32_t>(LIMIT_FLAG::LIMIT_GAIA)) != 0)
        bResult = false;
    if (pPlayer->GetRace() != 4 && (m_pFieldPropBase->nLimit & static_cast<int32_t>(LIMIT_FLAG::LIMIT_DEVA)) != 0)
        bResult = false;
    if (pPlayer->GetRace() != 5 && (m_pFieldPropBase->nLimit & static_cast<int32_t>(LIMIT_FLAG::LIMIT_ASURA)) != 0)
        bResult = false;
    if (!bResult)
        return false;

    // Job check
    if (m_pFieldPropBase->nLimitJobID != 0 && m_pFieldPropBase->nLimitJobID != pPlayer->GetCurrentJob())
        return false;

    for (int32_t x = 0; x < 2; ++x)
    {
        switch (m_pFieldPropBase->nActivateID[x])
        {
        case 1:
        {
            auto item = pPlayer->FindItemByCode(m_pFieldPropBase->nActivateValue[x][0]);
            if (item == nullptr || item->GetItemInstance().GetCount() < m_pFieldPropBase->nActivateValue[x][1])
                return false;
        }
        break;
        case 2:
            if (pPlayer->GetQuestProgress(m_pFieldPropBase->nActivateValue[x][0]) != m_pFieldPropBase->nActivateValue[x][1])
                return false;
            break;
        case 3:
            if (pPlayer->GetCurrentSkillLevel(m_pFieldPropBase->nActivateValue[x][0]) != m_pFieldPropBase->nActivateValue[x][1])
                return false;
            break;
        case 4:
            if ((m_pFieldPropBase->nActivateValue[x][0] != 0 && !pPlayer->IsWornByCode(m_pFieldPropBase->nActivateValue[x][0])) || (m_pFieldPropBase->nActivateValue[x][1] != 0 && !pPlayer->IsWornByCode(m_pFieldPropBase->nActivateValue[x][1])))
                return false;
            break;
        default:
            NG_LOG_INFO("entities.fieldprop", "FieldProp::IsUsable: Unknown ActivateID: %d [%d][%s]", m_pFieldPropBase->nActivateID[x], m_pFieldPropBase->nPropID, pPlayer->GetName());
            break;
        }
    }
    return true;
}

bool FieldProp::Cast()
{
    m_bIsCasting = true;
    return true;
}

bool FieldProp::UseProp(Player *pPlayer)
{
    m_bIsCasting = false;
    //int32_t oldUseCount = m_nUseCount;
    if (m_pFieldPropBase->nUseCount == 0 || m_nUseCount-- >= 0)
    {
        for (auto &i : m_pFieldPropBase->drop_info)
        {
            if (i.code != 0)
            {
                if (irand(1, 100000000) <= i.ratio)
                {
                    int32_t nItemCount = irand(i.min_count, i.max_count);
                    int32_t nLevel = irand(i.min_level, i.max_level + 1);
                    auto ti = Item::AllocItem(0, i.code, (uint64_t)nItemCount, BY_FIELD_PROP, nLevel, -1, -1, 0, 0, 0, 0, 0);

                    auto cnt = ti->GetItemInstance().GetCount();
                    Item *pNewItem = pPlayer->PushItem(ti, cnt, false);

                    if (pNewItem != nullptr)
                    {
                        Messages::SendResult(pPlayer, 0xCC, 0, ti->GetHandle());
                    }
                    if (pNewItem != nullptr && pNewItem->GetHandle() != ti->GetHandle())
                        Item::PendFreeItem(ti);
                }
            }
        }
        if (!m_pFieldPropBase->strScript.empty() && m_pFieldPropBase->strScript.length() > 2)
        {
            sScriptingMgr.RunString(pPlayer, m_pFieldPropBase->strScript);
        }
        if (m_pFieldPropBase->nUseCount != 0 && m_nUseCount == 0)
        {
            sWorld.RemoveObjectFromWorld(this);
            if (m_pDeleteHandler != nullptr)
                m_pDeleteHandler->onFieldPropDelete(this);
            DeleteThis();
        }
        return true;
    }
    else
    {
        m_nUseCount++;
        return false;
    }
}

int32_t FieldProp::GetCastingDelay() const
{
    return m_pFieldPropBase != nullptr ? static_cast<int32_t>(m_pFieldPropBase->nCastingTime) : 0;
}

FieldProp::FieldProp(FieldPropDeleteHandler *propDeleteHandler, FieldPropRespawnInfo pPropInfo) : WorldObject(true)
{
    _mainType = MT_StaticObject;
    _subType = ST_FieldProp;
    _objType = OBJ_STATIC;
    _valuesCount = 1;
    _InitValues();

    m_bIsCasting = false;
    sMemoryPool.AllocMiscHandle(this);
    m_pFieldPropBase = sObjectMgr.GetFieldPropBase(pPropInfo.nPropID);
    m_PropInfo = pPropInfo;
    m_pDeleteHandler = propDeleteHandler;
    nLifeTime = m_pFieldPropBase->nLifeTime;
    m_nRegenTime = sWorld.GetArTime();
    m_nUseCount = m_pFieldPropBase->nUseCount;
    if (m_pFieldPropBase->nUseCount == 0)
        m_nUseCount = 1;
}