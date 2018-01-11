#ifndef _GAMEDATABASE_H_
#define _GAMEDATABASE_H_

#include "DatabaseWorkerPool.h"
#include "MySQLConnection.h"

class CharacterDatabaseConnection : public MySQLConnection
{
public:
	//- Constructors for sync and async connections
	CharacterDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo) {}
	CharacterDatabaseConnection(ACE_Activation_Queue* q, MySQLConnectionInfo& connInfo) : MySQLConnection(q, connInfo) {}

	//- Loads database type specific prepared statements
	void DoPrepareStatements();
};

typedef DatabaseWorkerPool<CharacterDatabaseConnection> CharacterDatabaseWorkerPool;

enum CharacterDatabaseStatements
{
	/*  Naming standard for defines:
	{DB}_{SET/DEL/ADD/REP}_{Summary of data changed}
	When updating more than one field, consider looking at the calling function
	name for a suiting suffix.
	*/
	CHARACTER_GET_CHARACTERLIST = 0,
	CHARACTER_GET_WEARINFO,
	CHARACTER_GET_CHARACTER,
	CHARACTER_ADD_CHARACTER,
	CHARACTER_GET_NAMECHECK,
	CHARACTER_GET_ITEMLIST,
	CHARACTER_UPDATE_CHARACTER,
	CHARACTER_ADD_ITEM,
	CHARACTER_ADD_DEFAULT_ITEM,
	CHARACTER_DEL_CHARACTER,
	CHARACTER_UPD_ITEM,
	CHARACTER_GET_SUMMONLIST,
	CHARACTER_ADD_SUMMON,
	CHARACTER_UPD_SUMMON,
	CHARACTER_GET_EQUIP_ITEM,
	CHARACTER_GET_SKILL,
	CHARACTER_ADD_SKILL,
	CHARACTER_UPD_SKILL,
	CHARACTER_ADD_QUEST,
	CHARACTER_GET_QUEST,
	CHARACTER_GET_MAX_QUEST_ID,
	MAX_CHARACTERDATABASE_STATEMENTS,
};

#endif // _GAMEDATABASE_H_