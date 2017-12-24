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

#include "Skill.h"
#include "Unit.h"
#include "Database/DatabaseEnv.h"

Skill::Skill(Unit *pOwner, int _uid, int _id)
{
    sid = _uid;
    skill_id = _id;
    owner_id = pOwner->GetHandle();
    cool_time = 0;
    summon_id = 0;
    skill_level = 0;
}

void Skill::DB_InsertSkill(Unit *pUnit, uint skillUID, uint owner_uid, uint summon_uid, uint skill_id, uint skill_level)
{
    PreparedStatement* stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_ADD_SKILL);
    stmt->setInt32(0, skillUID);
    stmt->setInt32(1, owner_uid);
    stmt->setInt32(2, summon_uid);
    stmt->setInt32(3, skill_id);
    stmt->setInt32(4, skill_level);
    stmt->setInt32(5, 0); // cool_time
    CharacterDatabase.Execute(stmt);
}

void Skill::DB_UpdateSkill(Unit *pUnit, uint skill_uid, uint skill_level)
{
    PreparedStatement *stmt = CharacterDatabase.GetPreparedStatement(CHARACTER_UPD_SKILL);
    stmt->setInt32(0, skill_level);
    stmt->setInt32(1, 0); // cool_time
    auto uid = pUnit->GetUInt32Value(UNIT_FIELD_UID);
    stmt->setInt32(2, uid);
    stmt->setInt32(3, skill_uid);
    CharacterDatabase.Execute(stmt);
}
