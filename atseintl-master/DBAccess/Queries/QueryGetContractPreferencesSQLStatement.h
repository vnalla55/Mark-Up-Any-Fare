//----------------------------------------------------------------------------
//          File:           QueryGetContractPreferencesSQLStatement.h
//          Description:    QueryGetContractPreferencesSQLStatement
//          Created:        10/30/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetContractPreferences.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetContractPreferencesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetContractPreferencesSQLStatement() {};
  virtual ~QueryGetContractPreferencesSQLStatement() {};

  enum ColumnIndexes
  {
    PSEUDOCITY = 0,
    CARRIER,
    RULE,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    APPLYROUNDINGEXCEPTION,
    ALGORITHMNAME,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(" select PSEUDOCITY,CARRIER,RULE,CREATEDATE,EXPIREDATE,EFFDATE,"
                  "        DISCDATE,APPLYROUNDINGEXCEPTION,ALGORITHMNAME");
    this->From("=CONTRACTPREFERENCE");
    this->Where("PSEUDOCITY = %1q"
                "    and CARRIER = %2q"
                "    and RULE = %3q"
                "    and %cd <= EXPIREDATE");
    this->OrderBy("PSEUDOCITY,CARRIER,RULE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::ContractPreference* mapRowToContractPreference(Row* row)
  {
    tse::ContractPreference* cp = new tse::ContractPreference;

    cp->pseudoCity() = row->getString(PSEUDOCITY);
    cp->carrier() = row->getString(CARRIER);
    cp->rule() = row->getString(RULE);
    cp->createDate() = row->getDate(CREATEDATE);
    cp->expireDate() = row->getDate(EXPIREDATE);
    cp->effDate() = row->getDate(EFFDATE);
    cp->discDate() = row->getDate(DISCDATE);
    cp->applyRoundingException() = row->getChar(APPLYROUNDINGEXCEPTION);
    cp->algorithmName() = row->getString(ALGORITHMNAME);

    return cp;
  } // mapRowToContractPreference()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL where clause
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetContractPreferencesHistoricalSQLStatement
    : public QueryGetContractPreferencesSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("PSEUDOCITY = %1q"
                "    and CARRIER = %2q"
                "    and RULE = %3q");
  }
};
////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL where clause
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllContractPreferencesHistoricalSQLStatement
    : public QueryGetContractPreferencesSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    if (DataManager::forceSortOrder())
      this->OrderBy("PSEUDOCITY,CARRIER,RULE,CREATEDATE");
  }
};

} // tse
