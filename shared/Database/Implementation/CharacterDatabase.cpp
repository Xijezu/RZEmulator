/*
* Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
*
* This program is free software; you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the
* Free Software Foundation; either version 3 of the License, or (at your
* option) any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#include "CharacterDatabase.h"

void CharacterDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_CHARACTERDATABASE_STATEMENTS);

    PrepareStatement(CHARACTER_GET_CHARACTERLIST, "SELECT sid, name, race, sex, lv, jlv, exp, hp, mp, job, permission, skin_color, model_00, model_01, model_02, model_03, model_04, CONVERT(create_time, char) AS create_time, CONVERT(delete_time, char) AS delete_time FROM `Character` WHERE account_id = ? AND `name` NOT LIKE '@%' ORDER BY sid", CONNECTION_SYNCH);
    PrepareStatement(CHARACTER_GET_WEARINFO, "SELECT wear_info, code, enhance, level FROM Item WHERE account_id = 0 AND owner_id = ? AND summon_id = 0 AND auction_id = 0 AND keeping_id = 0 AND wear_info > -1 AND wear_info < 22 ORDER BY update_time", CONNECTION_SYNCH);
    PrepareStatement(CHARACTER_GET_CHARACTER, "SELECT sid, account, permission, party_id, guild_id, x, y, z, layer, race, sex, lv, exp, hp, mp, stamina, havoc, job_depth, jp, job_0, job_1, job_2, jlv_0, jlv_1, jlv_2, immoral_point, cha, pkc, dkc, summon_0, summon_1, summon_2, summon_3, summon_4, summon_5, skin_color, model_00, model_01, model_02, model_03, model_04, belt_00, belt_01, belt_02, belt_03, belt_04, belt_05, gold, chaos, client_info, flag_list, main_summon, sub_summon, remain_summon_time, pet, chat_block_time, guild_block_time, pkmode, job, jlv,client_info FROM `Character` WHERE name = ? AND account_id = ?", CONNECTION_SYNCH);
    PrepareStatement(CHARACTER_ADD_CHARACTER, "INSERT INTO `Character` (`name`,account,account_id,slot,x,y,z,layer,race,sex,lv,job,jlv,exp,hp,mp,skin_color,model_00,model_01,model_02,model_03,model_04,create_time,login_time,logout_time,otp_date,delete_time,sid,client_info)VALUES(?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?, NOW(),NOW(),NOW(),NOW(),CONVERT( '9999-12-31 23:59:59', DATETIME ),?,'QS=0,0,2,0|QS=0,1,2,2|QS=0,11,2,1|')", CONNECTION_SYNCH);
    PrepareStatement(CHARACTER_GET_NAMECHECK, "SELECT 1 FROM `Character` WHERE name = ?", CONNECTION_SYNCH);
    PrepareStatement(CHARACTER_UPDATE_CHARACTER, "UPDATE `Character` SET x = ?, y = ?, z = ?, layer = ?, exp = ?, lv = ?, hp = ?, mp = ?, stamina = ?, jlv = ?, jp = ?, total_jp = ?, job_0 = ?, job_1 = ?, job_2 = ?, jlv_0 = ?, jlv_1 = ?, jlv_2 = ?, permission = ?, job = ?, gold = ?, party_id = ?, guild_id = ?, summon_0 = ?, summon_1 = ?, summon_2 = ?, summon_3 = ?, summon_4 = ?, summon_5 = ?, main_summon = ?, sub_summon = ?, pet = ?, chaos = ?, client_info = ?, flag_list = ? WHERE sid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHARACTER_GET_ITEMLIST, "SELECT sid, idx, code, cnt, gcode, level, enhance, flag, summon_id, socket_0, socket_1, socket_2, socket_3, remain_time, wear_info FROM Item WHERE owner_id = ?", CONNECTION_SYNCH);
    PrepareStatement(CHARACTER_UPD_ITEM, "UPDATE `Item` SET owner_id = ?, account_id = ?, summon_id = ?, auction_id = ?, keeping_id = ?, idx = ?, cnt = ?, level = ?, enhance = ?, flag = ?, wear_info = ?, socket_0 = ?, socket_1 = ?, socket_2 = ?, socket_3 = ?, remain_time = ?, update_time = NOW() WHERE sid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHARACTER_GET_SUMMONLIST, "SELECT sid, account_id, code, card_uid, exp, jp, last_decreased_exp, name, transform, lv, jlv, max_level, fp, prev_level_01, prev_level_02, prev_id_01, prev_id_02, sp, hp, mp FROM Summon WHERE owner_id = ?", CONNECTION_SYNCH);
    PrepareStatement(CHARACTER_GET_SKILL, "SELECT sid, owner_id, summon_id, skill_id, skill_level, cool_time FROM Skill WHERE owner_id = ?", CONNECTION_SYNCH);
    PrepareStatement(CHARACTER_GET_EQUIP_ITEM, "SELECT sid, summon_id, wear_info FROM Item WHERE account_id = 0 AND owner_id = ? AND auction_id = 0 AND keeping_id = 0 AND wear_info > -1 ORDER BY summon_id, wear_info;", CONNECTION_SYNCH);
    //PrepareStatement(CHARACTER_ADD_ITEM, "INSERT INTO Item     (sid, owner_id, account_id, summon_id, auction_id, keeping_id, idx, code, cnt, level, enhance, endurance, flag, gcode, wear_info, socket_0, socket_1, socket_2, socket_3, socket_4, socket_5, remain_time, elemental_effect_type, elemental_effect_expire_time, elemental_effect_attack_point, elemental_effect_magic_point, create_time, update_time) VALUES(", CONNECTION_ASYNC);
    PrepareStatement(CHARACTER_ADD_ITEM, "INSERT INTO Item VALUES(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, NOW(), NOW())", CONNECTION_ASYNC);
    PrepareStatement(CHARACTER_ADD_DEFAULT_ITEM, "INSERT INTO Item VALUES(?, ?, 0, 0, 0, 0, 0, ?, 0, 0, 0, 0, 0, 0, ?, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, NOW(), NOW())", CONNECTION_ASYNC);
    PrepareStatement(CHARACTER_DEL_CHARACTER, "UPDATE `Character` SET name = CONCAT('@' , name , ' ', DATE_FORMAT(NOW(), '%d-%m-%y %H:%i:%s')) WHERE name = ? AND account_id = ?", CONNECTION_ASYNC);
    PrepareStatement(CHARACTER_ADD_SKILL, "INSERT INTO Skill VALUES (?,?,?,?,?,?);", CONNECTION_ASYNC);
    PrepareStatement(CHARACTER_UPD_SKILL, "UPDATE Skill SET skill_level = ?, cool_time = ? WHERE owner_id = ? AND sid = ?", CONNECTION_ASYNC);
    PrepareStatement(CHARACTER_ADD_SUMMON, "INSERT INTO Summon VALUES (?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?,?);", CONNECTION_ASYNC);
    PrepareStatement(CHARACTER_ADD_QUEST, "REPLACE INTO Quest VALUES(?, ?, ?, ?, ?, ?, ?, ?)", CONNECTION_ASYNC);
    PrepareStatement(CHARACTER_GET_QUEST, "SELECT id, code, start_id, status1, status2, status3, progress FROM Quest WHERE owner_id = ?", CONNECTION_SYNCH);
    PrepareStatement(CHARACTER_UPD_SUMMON, "UPDATE Summon SET account_id = ?, owner_id = ?, code = ?, exp = ?, jp = ?, last_decreased_exp = ?, name = ?, transform = ?, lv = ?, jlv = ?, max_level = ?, prev_level_01 = ?, prev_level_02 = ?, prev_id_01 = ?, prev_id_02 = ?, hp = ?, mp = ? WHERE sid = ?;", CONNECTION_ASYNC);
    PrepareStatement(CHARACTER_GET_MAX_QUEST_ID, "SELECT MAX(id) AS id FROM Quest WHERE owner_id = ?", CONNECTION_SYNCH);
}