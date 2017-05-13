//----------------------------------------------------------------------------
//          File:           QueryGetMileageSQLStatement.h
//          Description:    QueryGetMileageSQLStatement
//          Created:        10/26/2007
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
#include "DBAccess/Queries/QueryGetMileage.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMileageSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMileageSQLStatement() {};
  virtual ~QueryGetMileageSQLStatement() {};

  enum ColumnIndexes
  {
    ORIGLOC = 0,
    DESTLOC,
    MILEAGETYPE,
    GLOBALDIR,
    CREATEDATE,
    EFFDATE,
    EXPIREDATE,
    DISCDATE,
    MILEAGE,
    VENDOR,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select ORIGLOC,DESTLOC,MILEAGETYPE,GLOBALDIR,CREATEDATE,"
                  "       EFFDATE,EXPIREDATE,DISCDATE,MILEAGE,VENDOR");
    this->From("=MILEAGE ");
    this->Where("ORIGLOC = %1q  "
                "    and DESTLOC = %2q "
                "    and MILEAGETYPE = %3q "
                "    and %cd <= EXPIREDATE ");

    if (DataManager::forceSortOrder())
      this->OrderBy("ORIGLOC, DESTLOC , MILEAGETYPE, GLOBALDIR, CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::Mileage* mapRowToMileage(Row* row)
  {
    tse::Mileage* mileage = new tse::Mileage;

    mileage->orig() = row->getString(ORIGLOC);
    mileage->dest() = row->getString(DESTLOC);
    mileage->mileageType() = row->getChar(MILEAGETYPE);
    strToGlobalDirection(mileage->globaldir(), row->getString(GLOBALDIR));
    mileage->createDate() = row->getDate(CREATEDATE);
    mileage->effDate() = row->getDate(EFFDATE);
    mileage->expireDate() = row->getDate(EXPIREDATE);
    mileage->discDate() = row->getDate(DISCDATE);
    mileage->mileage() = row->getInt(MILEAGE);

    return mileage;
  } // mapRowToMileage()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMileageSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMileageHistoricalSQLStatement : public QueryGetMileageSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("ORIGLOC = %1q  "
                " and DESTLOC = %2q "
                " and MILEAGETYPE = %3q "
                " and %4n <= EXPIREDATE"
                " and %5n >= CREATEDATE");
  };
}; // class QueryGetMileageHistoricalSQLStatement
} // tse
