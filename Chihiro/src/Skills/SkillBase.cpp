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
#include "SkillBase.h"

bool SkillBase::IsUseableWeapon(ItemClass cl)
{
    auto c = (int)cl;

    if (c <= 210)
    {
        switch ( c )
        {
            case 101:
                return vf_one_hand_sword != 0;
            case 102:
                return vf_two_hand_sword != 0;
            case 98:
                return vf_double_dagger != 0;
            case 96:
                return vf_double_sword != 0;
            case 103:
                return vf_dagger != 0;
            case 104:
                return vf_spear != 0;
            case 105:
                return vf_axe != 0;
            case 113:
                return vf_one_hand_axe != 0;
            case 95:
                return vf_double_axe != 0;
            case 106:
                return vf_one_hand_mace != 0;
            case 107:
                return vf_two_hand_mace != 0;
            case 109:
                return vf_lightbow != 0;
            case 108:
                return vf_heavybow != 0;
            case 110:
                return vf_crossbow != 0;
            case 111:
                return vf_one_hand_staff != 0;
            case 112:
                return vf_two_hand_staff != 0;
            case 210:
                return vf_shield_only != 0;
            default:
                return false;
        }
    }

    return false;
}