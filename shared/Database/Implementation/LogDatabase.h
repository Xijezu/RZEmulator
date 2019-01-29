#pragma once

#include "MySQLConnection.h"

enum LogDatabaseStatements : uint32_t
{
    /*  Naming standard for defines:
    {DB}_{SET/DEL/ADD/REP}_{Summary of data changed}
    When updating more than one field, consider looking at the calling function
    name for a suiting suffix.
   */
    LOG_REP_USER_COUNT,
    MAX_LOGDATABASE_STATEMENTS
};

class LogDatabaseConnection : public MySQLConnection
{
  public:
    typedef LogDatabaseStatements Statements;

    //- Constructors for sync and async connections
    LogDatabaseConnection(MySQLConnectionInfo &connInfo);
    LogDatabaseConnection(ProducerConsumerQueue<SQLOperation *> *q, MySQLConnectionInfo &connInfo);
    ~LogDatabaseConnection();

    //- Loads database type specific prepared statements
    void DoPrepareStatements() override;
};