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

#include "Monster.h"
#include "Messages.h"

Monster::Monster(uint handle, MonsterBase mb) : Unit(true)
{
    _mainType = MT_NPC;
    _subType = ST_Mob;
    _objType = OBJ_MOVABLE;
    _valuesCount = UNIT_END;

    _InitValues();
    SetUInt32Value(UNIT_FIELD_HANDLE, handle);
    m_Base = mb;
    SetInt32Value(UNIT_FIELD_RACE, m_Base.race);
    SetMaxHealth(mb.hp);
    SetMaxMana(mb.mp);
    SetHealth(GetMaxHealth());
    SetMana(GetMaxMana());
    SetLevel(mb.level);
}

void Monster::EnterPacket(XPacket &pEnterPct, Monster *monster)
{
    Unit::EnterPacket(pEnterPct, monster);
    //pEnterPct << (uint32_t)0;
    Messages::GetEncodedInt(pEnterPct, bits_scramble(monster->m_Base.id));
    // pEnterPct << (uint32_t)0;
    pEnterPct << (uint8_t)0;
}