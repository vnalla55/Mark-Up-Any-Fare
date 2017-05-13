//----------------------------------------------------------------------------
//          File:           QueryGetAllServiceGroupSQLStatement.h
//          Description:    QueryGetAllServiceGroupSQLStatement
//          Created:        2/8/2010
// Authors:
//
//          Updates:
//
//      2010, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetServiceGroup.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetAllServiceGroupSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAllServiceGroupSQLStatement() {};
  virtual ~QueryGetAllServiceGroupSQLStatement() {};

  enum ColumnIndexes
  {
    SVCGROUP = 0,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    DEFINITION,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select SVCGROUP, CREATEDATE, EXPIREDATE, "
                  "       EFFDATE, DISCDATE, DEFINITION");

    this->From("=SERVICESGROUP");

    this->OrderBy("SVCGROUP");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  } // RegisterColumnsAndBaseSQL()

  static tse::ServiceGroupInfo* mapRowToServiceGroup(Row* row)
  {
    tse::ServiceGroupInfo* grIn = new tse::ServiceGroupInfo;

    grIn->svcGroup() = row->getString(SVCGROUP);
    grIn->createDate() = row->getDate(CREATEDATE);
    grIn->expireDate() = row->getDate(EXPIREDATE);
    grIn->effDate() = row->getDate(EFFDATE);
    grIn->discDate() = row->getDate(DISCDATE);
    grIn->definition() = row->getString(DEFINITION);

    return grIn;
  }; // mapRowToServiceGroup()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetServiceGroupSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllServiceGroupHistoricalSQLStatement
    : public QueryGetAllServiceGroupSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->OrderBy("SVCGROUP"); }
};
}
