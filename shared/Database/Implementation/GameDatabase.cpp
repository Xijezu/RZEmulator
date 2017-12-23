/*
* Copyright (C) 2016-2016 Xijezu <http://xijezu.com/>
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

#include "GameDatabase.h"

void GameDatabaseConnection::DoPrepareStatements()
{
	if (!m_reconnecting)
		m_stmts.resize(MAX_GAMEDATABASE_STATEMENTS);

	PrepareStatement(GAME_GET_NPC, "SELECT id, x, y, z, face, local_flag, contact_script FROM NPCResource;", CONNECTION_SYNCH); // Note: In Epic 4 roaming/etc is always 0 in official, so no need to use it yet
}