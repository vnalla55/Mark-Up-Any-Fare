//----------------------------------------------------------------------------
//          File:           QueryGetCabinSQLStatement.h
//          Description:    QueryGetCabinSQLStatement
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetCabin.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCabinSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCabinSQLStatement() {};
  virtual ~QueryGetCabinSQLStatement() {};

  enum ColumnIndexes
  {
    CARRIER = 0,
    CABIN,
    CLASSOFSERVICE,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {

    this->Command("select C.CARRIER,CABIN,CLASSOFSERVICE,C.CREATEDATE,EXPIREDATE,"
                  "       EFFDATE,DISCDATE ");
    this->From("=CABIN C, =CABINSEG S ");
    this->Where("C.CARRIER = S.CARRIER "
                "    and C.CREATEDATE = S.CREATEDATE "
                "    and C.CARRIER = %1q "
                "    and %cd <= EXPIREDATE ");

    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER,CABIN,CREATEDATE,CLASSOFSERVICE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  };

  static tse::Cabin* mapRowToCabin(Row* row)
  {
    tse::Cabin* cabin = new tse::Cabin;

    cabin->carrier() = row->getString(CARRIER);
    cabin->effDate() = row->getDate(EFFDATE);
    cabin->createDate() = row->getDate(CREATEDATE);
    cabin->expireDate() = row->getDate(EXPIREDATE);
    cabin->discDate() = row->getDate(DISCDATE);
    char cab = row->getChar(CABIN);
    cabin->cabin().setClass(cab);
    cabin->classOfService() = row->getString(CLASSOFSERVICE);
    return cabin;
  };

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to get QueryGetCabinHistoricalSQLStatement
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetCabinHistoricalSQLStatement : public QueryGetCabinSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("C.CARRIER = S.CARRIER"
                "    and C.CREATEDATE = S.CREATEDATE"
                "    and C.CARRIER = %1q ");
    if (DataManager::forceSortOrder())
      this->OrderBy("CABIN,C.CREATEDATE,CLASSOFSERVICE");
    else
      this->OrderBy("");
  };
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCabinSQLStatement : public QueryGetCabinSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("C.CARRIER = S.CARRIER"
                "    and C.CREATEDATE = S.CREATEDATE"
                "    and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER,CABIN,CREATEDATE,CLASSOFSERVICE");
    else
      this->OrderBy("1");
  };
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to get QueryGetAllCabinHistoricalSQLStatement
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllCabinHistoricalSQLStatement : public QueryGetCabinSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("C.CARRIER = S.CARRIER"
                "  and C.CREATEDATE = S.CREATEDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER,CABIN,CREATEDATE,CLASSOFSERVICE");
    else
      this->OrderBy("1");
  };
};
}
