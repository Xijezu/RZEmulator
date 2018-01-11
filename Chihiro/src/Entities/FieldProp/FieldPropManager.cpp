/*
  *  Copyright (C) 2018 Xijezu <http://xijezu.com/>
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

#include "FieldPropManager.h"
#include "ObjectMgr.h"
#include "World.h"

void FieldPropManager::SpawnFieldPropFromScript(FieldPropRespawnInfo prop, int lifeTime)
{
    auto propTemplate = sObjectMgr->GetFieldPropBase(prop.nPropID);
    if(propTemplate == nullptr)
        return;

    m_vRespawnInfo.emplace_back(prop);

    FieldPropRegenInfo ri = FieldPropRegenInfo{0, (uint)lifeTime};
    ri.pRespawnInfo = prop;
    m_vRespawnList.emplace_back(ri);
}

void FieldPropManager::RegisterFieldProp(FieldPropRespawnInfo prop)
{
    FieldPropRespawnInfo info{};
    int nPropID = prop.nPropID;
    FieldPropTemplate* propTemplate = sObjectMgr->GetFieldPropBase(nPropID);
    if(propTemplate == nullptr)
        return;

    info.nPropID = nPropID;
    info.layer = 0; // Layer management
    m_vRespawnInfo.emplace_back(info);
    FieldPropRegenInfo ri = FieldPropRegenInfo{propTemplate->nRegenTime + sWorld->GetArTime(), propTemplate->nLifeTime};
    ri.pRespawnInfo = info;
    m_vRespawnList.emplace_back(ri);
}

void FieldPropManager::onFieldPropDelete(FieldProp *prop)
{

}

void FieldPropManager::Update(uint/* diff*/)
{
    uint ct = sWorld->GetArTime();
    std::vector<FieldPropRegenInfo> vRegenInfo{};
    std::vector<FieldProp*> vDeleteList{};

    for(int i = 0; i < m_vRespawnInfo.size(); ++i) {
        FieldPropRegenInfo regen = m_vRespawnList[i];
        if(regen.nLifeTime < ct) {
            vRegenInfo.emplace_back(regen);
            m_vRespawnList.erase(m_vRespawnList.begin() + i);
        }
    }

    if(!vRegenInfo.empty())
    {
        for(auto& rg : vRegenInfo)
        {
            FieldProp* pProp = FieldProp::Create(this, rg.pRespawnInfo, rg.nLifeTime);
            if(pProp->nLifeTime != 0)
            {
                m_vExpireObject.emplace_back(pProp);
            }
        }
    }

    for(auto& fp : m_vExpireObject)
    {
        if(fp->m_nRegenTime + fp->nLifeTime < ct)
            vDeleteList.emplace_back(fp);
    }

    if(!vDeleteList.empty())
    {
        for(auto& fp : vDeleteList) {
            if(fp->IsInWorld() && ! fp->IsDeleteRequested())
            {
                sWorld->RemoveObjectFromWorld(fp);
                fp->DeleteThis();
            }
        }
    }
}
