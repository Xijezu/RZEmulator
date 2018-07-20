#pragma once
#include "MySQLConnection.h"

enum LoginDatabaseStatements : uint32
{
    /*  Naming standard for defines:
    {DB}_{SET/DEL/ADD/REP}_{Summary of data changed}
    When updating more than one field, consider looking at the calling function
    name for a suiting suffix.
   */
            LOGIN_GET_ACCOUNT,
            MAX_LOGINDATABASE_STATEMENTS
};

class LoginDatabaseConnection : public MySQLConnection
{
    public:
        typedef LoginDatabaseStatements Statements;

//- Constructors for sync and async connections
        LoginDatabaseConnection(MySQLConnectionInfo &connInfo);
        LoginDatabaseConnection(ProducerConsumerQueue<SQLOperation *> *q, MySQLConnectionInfo &connInfo);
        ~LoginDatabaseConnection();

//- Loads database type specific prepared statements
        void DoPrepareStatements() override;
};