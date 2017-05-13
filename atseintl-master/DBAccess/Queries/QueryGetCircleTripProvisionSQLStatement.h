//----------------------------------------------------------------------------
//          File:           QueryGetCircleTripProvisionSQLStatement.h
//          Description:    QueryGetCircleTripProvisionSQLStatement
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
#include "DBAccess/Queries/QueryGetCircleTripProvision.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCircleTripProvisionSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCircleTripProvisionSQLStatement() {};
  virtual ~QueryGetCircleTripProvisionSQLStatement() {};

  enum ColumnIndexes
  {
    MARKET1 = 0,
    MARKET2,
    EFFDATE,
    CREATEDATE,
    EXPIREDATE,
    DISCDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select MARKET1,MARKET2,EFFDATE,CREATEDATE,EXPIREDATE,DISCDATE");
    this->From("=CIRCLETRIPPROVISION");
    this->Where("MARKET1 = %1q "
                "    and MARKET2 = %2q "
                "    and %cd <= EXPIREDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::CircleTripProvision* mapRowToCircleTripProvision(Row* row)
  {
    tse::CircleTripProvision* ctp = new tse::CircleTripProvision;

    ctp->market1() = row->getString(MARKET1);
    ctp->market2() = row->getString(MARKET2);
    ctp->effDate() = row->getDate(EFFDATE);
    ctp->createDate() = row->getDate(CREATEDATE);
    ctp->expireDate() = row->getDate(EXPIREDATE);
    ctp->discDate() = row->getDate(DISCDATE);

    return ctp;
  } // mapRowToCircleTripProvision()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetCircleTripProvisionHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetCircleTripProvisionHistoricalSQLStatement
    : public QueryGetCircleTripProvisionSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("MARKET1 = %1q "
                "    and MARKET2 = %2q ");
    this->OrderBy("CREATEDATE");
  };
};
////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCircleTripProvision
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCircleTripProvisionSQLStatement
    : public QueryGetCircleTripProvisionSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    this->OrderBy("1,2");
  };
};
////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllCircleTripProvisionHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCircleTripProvisionHistoricalSQLStatement
    : public QueryGetCircleTripProvisionSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("1,2");
  };
};
} // tse
