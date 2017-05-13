//----------------------------------------------------------------------------
//          File:           QueryGetCorpIdSQLStatement.h
//          Description:    QueryGetCorpIdSQLStatement
//          Created:        11/01/2007
//          Authors:        Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCorpId.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCorpIdSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCorpIdSQLStatement() {};
  virtual ~QueryGetCorpIdSQLStatement() {};

  enum ColumnIndexes
  {
    CORPID = 0,
    VENDOR,
    CARRIER,
    RULETARIFF,
    RULE,
    ACCOUNTCODE,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CORPID,VENDOR,CARRIER,RULETARIFF,RULE,ACCOUNTCODE,"
                  "       CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE");
    this->From("=CORPID");
    this->Where("CORPID = %1q"
                "    and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("ACCOUNTCODE,RULETARIFF,CORPID,VENDOR,CARRIER,RULE,CREATEDATE,EXPIREDATE");
    else
      this->OrderBy("ACCOUNTCODE, RULETARIFF");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::CorpId* mapRowToCorpId(Row* row)
  {
    CorpId* rec = new CorpId();

    rec->corpId() = row->getString(CORPID);
    rec->vendor() = row->getString(VENDOR);
    rec->carrier() = row->getString(CARRIER);
    rec->ruleTariff() = row->getInt(RULETARIFF);
    rec->rule() = row->getString(RULE);
    rec->accountCode() = row->getString(ACCOUNTCODE);
    rec->createDate() = row->getDate(CREATEDATE);
    rec->expireDate() = row->getDate(EXPIREDATE);
    rec->effDate() = row->getDate(EFFDATE);
    rec->discDate() = row->getDate(DISCDATE);

    return rec;
  } // mapRowToCorpId()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetCorpIdSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to get replace Where clause and add an OrderBy
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCorpIdHistoricalSQLStatement : public QueryGetCorpIdSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("CORPID = %1q"
                "  and %2n <= EXPIREDATE"
                "  and %3n >= CREATEDATE");
  }
}; // class QueryGetCorpIdHistoricalSQLStatement
} // tse
