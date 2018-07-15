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

#include "SkillBase.h"
#include "World.h"

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

int SkillBase::GetStateSecond(int skill_lv, int enhance_lv)
{
    return (int) state_second + (int) enhance_lv * (int) state_second_per_enhance + skill_lv * (int) state_second_per_level;
}

int SkillBase::GetHitBonus(int enhance, int level_diff) const
{
    return hit_bonus + level_diff * percentage + enhance * hit_bonus_per_enhance;
}

int SkillBase::GetStateLevel(int skill_lv, int enhance_lv)
{
    return (int) (state_level_base
                  + (state_level_per_enhance * enhance_lv)
                  + (state_level_per_skl * skill_lv));
}

uint SkillBase::GetCastDelay(int skill_lv, int enhance)
{
    return (uint)( (float)((float)delay_cast + (float)skill_lv * (float)delay_cast_per_skl ) * (float)(delay_cast_mode_per * (float)enhance + 1.0f));
}

uint SkillBase::GetCoolTime(int enhance) const
{
    return sWorld.getBoolConfig(CONFIG_NO_SKILL_COOLTIME) ? 0 : (uint)((delay_cooltime_mode * (float)enhance + 1.0f) * delay_cooltime);
}

int SkillBase::GetNeedJobPoint(int skill_lv)
{
    int result;

    if (skill_lv <= 50)
        result = this->m_need_jp[skill_lv - 1];
    else
        result = this->m_need_jp[49];
    return result;
}

bool SkillBase::IsUsable(uint8 nUseIndex) const
{
    switch (nUseIndex)
    {
        case 0:
            return uf_self != 0;
        case 1:
            return uf_party != 0;
        case 2:
            return uf_guild != 0;
        case 3:
            return uf_neutral != 0;
        case 4:
            return uf_purple != 0;
        case 5:
            return uf_enemy != 0;
        default:
            return false;
    }
}
