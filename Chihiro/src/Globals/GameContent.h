/*
 *  Copyright (C) 2017-2018 NGemity <https://ngemity.org/>
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

#ifndef NGEMITY_GAMECONTENT_H
#define NGEMITY_GAMECONTENT_H

#include "Common.h"
#include "Unit.h"

class GameContent
{
    public:
        static int EnumSkillTargetsAndCalcDamage(
                Position _OriginalPos, uint8_t layer, Position _TargetPos, bool bTargetOrigin,
                float fEffectLength, int nRegionType, float fRegionProperty, int nOriginalDamage,
                bool bIncludeOriginalPos, Unit *pCaster, int nDistributeType, int nTargetMax,
                std::vector<Unit *> &vTargetList, bool bEnemyOnly);
    private:
        GameContent() = default;
        ~GameContent() = default;
};

#endif // NGEMITY_GAMECONTENT_H
