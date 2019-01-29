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

#include "LogDatabase.h"
#include "PreparedStatement.h"

void LogDatabaseConnection::DoPrepareStatements()
{
    if (!m_reconnecting)
        m_stmts.resize(MAX_LOGDATABASE_STATEMENTS);

    //PrepareStatement(LOGIN_GET_ACCOUNT, "SELECT account_id, login_name, last_login_server_idx, block, permission FROM Accounts WHERE login_name = ? AND password = ?", CONNECTION_SYNCH);
    PrepareStatement(LOG_REP_USER_COUNT, "REPLACE INTO ServerMonitor (`name`, `scrape_date`, `user_count`) VALUES (?, NOW(), ?)", CONNECTION_ASYNC);
}

LogDatabaseConnection::LogDatabaseConnection(MySQLConnectionInfo &connInfo) : MySQLConnection(connInfo)
{
}

LogDatabaseConnection::LogDatabaseConnection(ProducerConsumerQueue<SQLOperation *> *q, MySQLConnectionInfo &connInfo) : MySQLConnection(q, connInfo)
{
}

LogDatabaseConnection::~LogDatabaseConnection()
{
}