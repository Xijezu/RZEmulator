#pragma once
/*
 * Copyright (C) 2008-2018 TrinityCore <https://www.trinitycore.org/>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
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
#include "Define.h"
#include "DatabaseEnvFwd.h"
#include <vector>

enum class DatabaseFieldTypes : uint8_t
{
    Null,
    Int8,
    Int16,
    Int32,
    Int64,
    Float,
    Double,
    Decimal,
    Date,
    Binary
};

/**
    @class Field

    @brief Class used to access individual fields of database query result

    Guideline on field type matching:

    |   MySQL type           |  method to use                         |
    |------------------------|----------------------------------------|
    | TINYINT                | GetBool, GetInt8, Getuint8_t             |
    | SMALLINT               | GetInt16, Getuint16_t                    |
    | MEDIUMINT, INT         | GetInt32, Getuint32_t                    |
    | BIGINT                 | GetInt64, GetUInt64                    |
    | FLOAT                  | GetFloat                               |
    | DOUBLE, DECIMAL        | GetDouble                              |
    | CHAR, VARCHAR,         | GetCString, GetString                  |
    | TINYTEXT, MEDIUMTEXT,  | GetCString, GetString                  |
    | TEXT, LONGTEXT         | GetCString, GetString                  |
    | TINYBLOB, MEDIUMBLOB,  | GetBinary, GetString                   |
    | BLOB, LONGBLOB         | GetBinary, GetString                   |
    | BINARY, VARBINARY      | GetBinary                              |

    Return types of aggregate functions:

    | Function |       Type        |
    |----------|-------------------|
    | MIN, MAX | Same as the field |
    | SUM, AVG | DECIMAL           |
    | COUNT    | BIGINT            |
*/
class Field
{
    friend class ResultSet;
    friend class PreparedResultSet;

  public:
    Field();
    ~Field();

    bool GetBool() const // Wrapper, actually gets integer
    {
        return GetUInt8() == 1 ? true : false;
    }

    uint8_t GetUInt8() const;
    int8_t GetInt8() const;
    uint16_t GetUInt16() const;
    int16_t GetInt16() const;
    uint32_t GetUInt32() const;
    int32_t GetInt32() const;
    uint64_t GetUInt64() const;
    int64_t GetInt64() const;
    float GetFloat() const;
    double GetDouble() const;
    char const *GetCString() const;
    std::string GetString() const;
    std::vector<uint8_t> GetBinary() const;

    bool IsNull() const
    {
        return data.value == NULL;
    }

    struct Metadata
    {
        char const *TableName;
        char const *TableAlias;
        char const *Name;
        char const *Alias;
        char const *Type;
        uint32_t Index;
    };

  protected:
#pragma pack(push, 1)
    struct
    {
        uint32_t length;         // Length (prepared strings only)
        void *value;             // Actual data in memory
        DatabaseFieldTypes type; // Field type
        bool raw;                // Raw bytes? (Prepared statement or ad hoc)
    } data;
#pragma pack(pop)

    void SetByteValue(void *newValue, DatabaseFieldTypes newType, uint32_t length);
    void SetStructuredValue(char *newValue, DatabaseFieldTypes newType, uint32_t length);

    void CleanUp()
    {
        // Field does not own the data if fetched with prepared statement
        if (!data.raw)
            delete[]((char *)data.value);
        data.value = NULL;
    }

    bool IsType(DatabaseFieldTypes type) const;

    bool IsNumeric() const;

  private:
#ifdef NGEMITY_DEBUG
    void LogWrongType(char *getter) const;
    void SetMetadata(MYSQL_FIELD *field, uint32_t fieldIndex);
    Metadata meta;
#endif
};