//----------------------------------------------------------------------------
//          File:           QueryGetVoluntarySQLStatement.h
//          Description:    QueryGetVoluntarySQLStatement
//          Created:        10/27/2008
// Authors:         Dean Van Decker
//
//          Updates:
//
//     � 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetVoluntaryChangesConfig.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetVoluntaryChangesConfigSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetVoluntaryChangesConfigSQLStatement() {}
  virtual ~QueryGetVoluntaryChangesConfigSQLStatement() {}

  enum ColumnIndexes
  {
    CARRIER = 0,
    CREATEDATE,
    RULEAPPLICATIONDATE,
    EFFDATE,
    DISCDATE,
    EXPIREDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {

    this->Command("select CARRIER,CREATEDATE,RULEAPPLICATIONDATE,"
                  " EFFDATE,DISCDATE,EXPIREDATE ");
    this->From(" =VOLUNTARYCHANGESCONFIG");
    this->Where("CARRIER = %1q");
    this->OrderBy("EXPIREDATE desc");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::VoluntaryChangesConfig* mapRowToVoluntaryChangesConfig(Row* row)
  {
    tse::VoluntaryChangesConfig* voluntaryChangesConfig = new tse::VoluntaryChangesConfig;

    voluntaryChangesConfig->carrier() = row->getString(CARRIER);
    voluntaryChangesConfig->effDate() = row->getDate(EFFDATE);
    voluntaryChangesConfig->createDate() = row->getDate(CREATEDATE);
    voluntaryChangesConfig->expireDate() = row->getDate(EXPIREDATE);
    voluntaryChangesConfig->discDate() = row->getDate(DISCDATE);
    voluntaryChangesConfig->applDate() = row->getDate(RULEAPPLICATIONDATE);
    return voluntaryChangesConfig;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to get QueryGetAllVoluntaryChangesConfigSQLStatement
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllVoluntaryChangesConfigSQLStatement
    : public QueryGetVoluntaryChangesConfigSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("1 = 1");
    this->OrderBy("CARRIER,EXPIREDATE desc");
  }
};
}

