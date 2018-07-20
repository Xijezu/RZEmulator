#pragma once
#include "MySQLConnection.h"

enum GameDatabaseStatements : uint32
{
    /*  Naming standard for defines:
    {DB}_{SET/DEL/ADD/REP}_{Summary of data changed}
    When updating more than one field, consider looking at the calling function
    name for a suiting suffix.
    */
            MAX_GAMEDATABASE_STATEMENTS
};

class GameDatabaseConnection : public MySQLConnection
{
    public:
        typedef GameDatabaseStatements Statements;

//- Constructors for sync and async connections
        GameDatabaseConnection(MySQLConnectionInfo &connInfo);
        GameDatabaseConnection(ProducerConsumerQueue<SQLOperation *> *q, MySQLConnectionInfo &connInfo);
        ~GameDatabaseConnection();

//- Loads database type specific prepared statements
        void DoPrepareStatements() override;
};