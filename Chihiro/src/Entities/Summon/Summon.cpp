/*
  *  Copyright (C) 2017 Xijezu <http://xijezu.com/>
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

Summon::Summon(uint pHandle, uint pIdx) : Unit(true)
{
#ifdef _MSC_VER
#   pragma warning(default:4355)
#endif
    _mainType = MT_StaticObject;
    _subType  = ST_Summon;
    _objType  = OBJ_MOVABLE;
    _valuesCount = UNIT_END;

    _InitValues();
    SetInt32Value(UNIT_FIELD_HANDLE, pHandle);
    SetSummonInfo(pIdx);
}

Summon* Summon::AllocSummon(Player * pMaster, uint pCode)
{
    Summon* summon = sMemoryPool->AllocSummon(pCode);
    summon->m_pMaster = pMaster;
}

void Summon::SetSummonInfo(int idx)
{
    m_tSummonBase = sObjectMgr->GetSummonBase(idx);
    SetCurrentJob(idx);
}

int Summon::GetSummonCode()
{
    return m_tSummonBase.id;
}

uint32_t Summon::GetCardHandle()
{
    ACE_ASSERT(m_pItem);

    return m_pItem->m_nHandle;
}