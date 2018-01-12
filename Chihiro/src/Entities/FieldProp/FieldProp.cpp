#include "FieldProp.h"
#include "World.h"
#include "MemPool.h"
#include "ObjectMgr.h"
#include "Messages.h"
#include "Scripting/XLua.h"

void FieldProp::EnterPacket(XPacket &pEnterPct, FieldProp *pFieldProp, Player */*pPlayer*/)
{
    pEnterPct << (uint)pFieldProp->m_PropInfo.nPropID;
    pEnterPct << pFieldProp->m_PropInfo.fZOffset;
    pEnterPct << pFieldProp->m_PropInfo.fRotateX;
    pEnterPct << pFieldProp->m_PropInfo.fRotateY;
    pEnterPct << pFieldProp->m_PropInfo.fRotateZ;
    pEnterPct << pFieldProp->m_PropInfo.fScaleX;
    pEnterPct << pFieldProp->m_PropInfo.fScaleY;
    pEnterPct << pFieldProp->m_PropInfo.fScaleZ;
    pEnterPct << (uint8)(pFieldProp->m_PropInfo.bLockHeight ? 1 : 0);
    pEnterPct << pFieldProp->m_PropInfo.fLockHeight;
}


FieldProp *FieldProp::Create(FieldPropDeleteHandler *propDeleteHandler, FieldPropRespawnInfo pPropInfo, uint lifeTime)
{
    auto fp = new FieldProp{propDeleteHandler, pPropInfo};
    fp->nLifeTime = lifeTime;
    fp->SetCurrentXY(pPropInfo.x, pPropInfo.y);
    fp->m_PropInfo.layer = pPropInfo.layer;
    sWorld->AddObjectToWorld(fp);
    MX_LOG_TRACE("objects", "Adding FieldProp [X: %f, Y: %f, Layer: %d]: %d", fp->GetPositionX(),fp->GetPositionY(), fp->GetLayer(), fp->m_pFieldPropBase->nPropID);
    return fp;
}

bool FieldProp::IsUsable(Player *pPlayer) const
{
    if(pPlayer == nullptr)
        return false;
    if(m_bIsCasting)
        return false;

    // Level checks
    if(pPlayer->GetLevel() < m_pFieldPropBase->nMinLevel)
        return false;
    if(pPlayer->GetLevel() > m_pFieldPropBase->nMaxLevel)
        return false;

    bool bResult = false;
    // Class checks
    if ((m_pFieldPropBase->nLimit & 0x800) != 0 && !pPlayer->IsHunter())
        bResult = true;
    if ((m_pFieldPropBase->nLimit & 0x400) != 0 && !pPlayer->IsFighter())
        bResult = true;
    if ((m_pFieldPropBase->nLimit & 0x1000) != 0 && !pPlayer->IsMagician())
        bResult = true;
    if ((m_pFieldPropBase->nLimit & 0x2000) != 0 && !pPlayer->IsSummoner())
        bResult = true;
    if(!bResult)
        return false;

    // Race checks
    bResult = false;
    if(pPlayer->GetRace() != 3 || (m_pFieldPropBase->nLimit & 0x10) != 0)
        bResult = true;
    if(pPlayer->GetRace() != 4 || (m_pFieldPropBase->nLimit & 4) != 0)
        bResult = true;
    if(pPlayer->GetRace() != 5 || (m_pFieldPropBase->nLimit & 8) != 0)
        bResult = true;
    if(!bResult)
        return false;

    // Job check
    if(m_pFieldPropBase->nLimitJobID != 0 && m_pFieldPropBase->nLimitJobID != pPlayer->GetCurrentJob())
        return false;

    for(int x = 0; x < 2; ++x)
    {
        switch(m_pFieldPropBase->nActivateID[x])
        {
            case 1:
            {
                auto item = pPlayer->FindItemByCode(m_pFieldPropBase->nActivateValue[x][0]);
                if (item == nullptr || item->m_Instance.nCount < m_pFieldPropBase->nActivateValue[x][1])
                    return false;
            }
                break;
            case 2:
                if(pPlayer->GetQuestProgress(m_pFieldPropBase->nActivateValue[x][0]) != m_pFieldPropBase->nActivateValue[x][1])
                    return false;
                break;
            case 3:
                if(pPlayer->GetCurrentSkillLevel(m_pFieldPropBase->nActivateValue[x][0]) != m_pFieldPropBase->nActivateValue[x][1])
                    return false;
                break;
            case 4:
                if ((m_pFieldPropBase->nActivateValue[x][0] != 0 && !pPlayer->IsWornByCode(m_pFieldPropBase->nActivateValue[x][0]))
                     || (m_pFieldPropBase->nActivateValue[x][1] != 0 && !pPlayer->IsWornByCode(m_pFieldPropBase->nActivateValue[x][1])))
                    return false;
                break;
            default:
                MX_LOG_INFO("entities.fieldprop", "FieldProp::IsUsable: Unknown ActivateID: %d [%d][%s]", m_pFieldPropBase->nActivateID[x], m_pFieldPropBase->nPropID, pPlayer->GetName());
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

bool FieldProp::UseProp(Player * pPlayer)
{
    //int oldUseCount = m_nUseCount;
    if(m_pFieldPropBase->nUseCount == 0 || m_nUseCount-- >= 0)
    {
        for (auto &i : m_pFieldPropBase->drop_info)
        {
            if(i.code != 0)
            {
                if (irand(1, 100000000) <= i.ratio)
                {
                    int  nItemCount = irand(i.min_count, i.max_count);
                    int  nLevel     = irand(i.min_level, i.max_level + 1);
                    auto ti         = Item::AllocItem(0, i.code, (uint64)nItemCount, GenerateCode::ByFieldProp, nLevel, -1, -1, 0, 0, 0, 0, 0);

                    auto cnt = ti->m_Instance.nCount;
                    pPlayer->PushItem(ti, cnt, false);

                    if(ti != nullptr)
                    {
                        Messages::SendResult(pPlayer, 0xCC, 0, ti->GetHandle());
                    }
                }
            }
        }
        if(!m_pFieldPropBase->strScript.empty() && m_pFieldPropBase->strScript.length() > 2)
        {
            sScriptingMgr->RunString(pPlayer, m_pFieldPropBase->strScript);
        }
        if(m_nUseCount == 0)
        {
            sWorld->RemoveObjectFromWorld(this);
            if(m_pDeleteHandler != nullptr)
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
    ACE_NOTREACHED(return false);
}

uint FieldProp::GetCastingDelay() const
{
    return m_pFieldPropBase != nullptr ? m_pFieldPropBase->nCastingTime : 0;
}

FieldProp::FieldProp(FieldPropDeleteHandler *propDeleteHandler, FieldPropRespawnInfo pPropInfo) : WorldObject(true)
{
    _mainType = MT_StaticObject;
    _subType  = ST_FieldProp;
    _objType  = OBJ_STATIC;
    _valuesCount = 1;
    _InitValues();

    m_bIsCasting = false;
    sMemoryPool->AllocMiscHandle(this);
    m_pFieldPropBase = sObjectMgr->GetFieldPropBase(pPropInfo.nPropID);
    m_PropInfo = pPropInfo;
    m_pDeleteHandler = propDeleteHandler;
    nLifeTime = m_pFieldPropBase->nLifeTime;
    m_nRegenTime = sWorld->GetArTime();
    m_nUseCount = m_pFieldPropBase->nUseCount;
    if(m_pFieldPropBase->nUseCount == 0)
        m_nUseCount = 1;
}