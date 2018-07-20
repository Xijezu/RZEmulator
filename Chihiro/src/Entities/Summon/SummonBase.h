#pragma once
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

struct SummonLevelBonus
{
    int   summon_id;
    float strength;
    float vital;
    float dexterity;
    float agility;
    float intelligence;
    float mentality;
    float luck;
};

struct SummonResourceTemplate {
    int   id;
    int   type;
    int   magic_type;
    int   rate;
    int   stat_id;
    float size;
    float scale;
    int   standard_walk_speed;
    int   standard_run_speed;
    int   walk_speed;
    int   run_speed;
    bool  is_riding_only;
    float attack_range;
    int   material;
    int   weapon_type;
    int   form;
    int   evolve_target;
    int   card_id;
};