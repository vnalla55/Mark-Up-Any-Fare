//----------------------------------------------------------------------------
//     � 2010, Sabre Inc.  All rights reserved.  This software/documentation is
//     the confidential and proprietary product of Sabre Inc. Any unauthorized
//     use, reproduction, or transfer of this software/documentation, in any
//     medium, or incorporation of this software/documentation into any system
//     or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/BaggageSectorException.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetBaggageSectorExceptionSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetBaggageSectorExceptionSQLStatement() {};
  virtual ~QueryGetBaggageSectorExceptionSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    DIRECTIONALIND,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER, CREATEDATE, EXPIREDATE, EFFDATE, DISCDATE,"
                  "DIRECTIONALIND, LOC1TYPE, LOC1, LOC2TYPE, LOC2");
    this->From("=BAGGAGESECTOREXCEPTION ");
    this->Where("CARRIER = %1q "
                "   and %cd <= EXPIREDATE ");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static BaggageSectorException* mapRowToBaggageSectorException(Row* row)
  {
    BaggageSectorException* exception = new BaggageSectorException;

    exception->carrier() = row->getString(CARRIER);
    exception->createDate() = row->getDate(CREATEDATE);
    exception->expireDate() = row->getDate(EXPIREDATE);
    exception->effDate() = row->getDate(EFFDATE);
    exception->discDate() = row->getDate(DISCDATE);
    std::string dir = row->getString(DIRECTIONALIND);
    if (dir == "F")
      exception->directionality() = FROM;
    else if (dir == "W")
      exception->directionality() = WITHIN;
    else if (dir == "O")
      exception->directionality() = ORIGIN;
    else if (dir == "X")
      exception->directionality() = TERMINATE;
    else if (dir == "B")
      exception->directionality() = BETWEEN;
    else
      exception->directionality() = BETWEEN;

    if (!row->isNull(LOC1TYPE) && row->getChar(LOC1TYPE) != ' ')
      exception->loc1().locType() = LocType(row->getChar(LOC1TYPE));
    else
      exception->loc1().locType() = LocType(UNKNOWN_LOC);

    if (!row->isNull(LOC1) && row->getChar(LOC1) != ' ')
      exception->loc1().loc() = row->getString(LOC1);
    else
      exception->loc1().loc() = "";

    if (!row->isNull(LOC2TYPE) && row->getChar(LOC2TYPE) != ' ')
      exception->loc2().locType() = LocType(row->getChar(LOC2TYPE));
    else
      exception->loc2().locType() = LocType(UNKNOWN_LOC);

    if (!row->isNull(LOC2) && row->getChar(LOC2) != ' ')
      exception->loc2().loc() = row->getString(LOC2);
    else
      exception->loc2().loc() = "";

    return exception;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetBaggageSectorExceptionHistoricalSQLStaement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetBaggageSectorExceptionHistoricalSQLStatement
    : public QueryGetBaggageSectorExceptionSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("CARRIER = %1q ");
    if (DataManager::forceSortOrder())
    {
      this->OrderBy("CARRIER,CREATEDATE");
    }
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllBaggageSectorExceptionSQLStatement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllBaggageSectorExceptionSQLStatement
    : public QueryGetBaggageSectorExceptionSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    this->OrderBy("CARRIER");
  };
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllBaggageSectorExceptionHistoricalSQLStaement
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllBaggageSectorExceptionHistoricalSQLStatement
    : public QueryGetBaggageSectorExceptionSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("1=1");
    this->OrderBy("CARRIER, CREATEDATE");
  };
};
} // tse
