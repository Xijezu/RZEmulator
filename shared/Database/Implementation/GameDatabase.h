#ifndef _GAMEDATABASE_H
#define _GAMEDATABASE_H

#include "DatabaseWorkerPool.h"
#include "MySQLConnection.h"

class GameDatabaseConnection : public MySQLConnection
{
public:
	//- Constructors for sync and async connections
	GameDatabaseConnection(MySQLConnectionInfo& connInfo) : MySQLConnection(connInfo) {}
	GameDatabaseConnection(ACE_Activation_Queue* q, MySQLConnectionInfo& connInfo) : MySQLConnection(q, connInfo) {}

	//- Loads database type specific prepared statements
	void DoPrepareStatements();
};

typedef DatabaseWorkerPool<GameDatabaseConnection> GameDatabaseWorkerPool;

enum GameDatabaseStatements
{
	/*  Naming standard for defines:
	{DB}_{SET/DEL/ADD/REP}_{Summary of data changed}
	When updating more than one field, consider looking at the calling function
	name for a suiting suffix.
	*/
	GAME_GET_NPC = 0,
// 	GAME_GET_MONSTER,
// 	GAME_GET_ITEM,
	MAX_GAMEDATABASE_STATEMENTS
};

#endif