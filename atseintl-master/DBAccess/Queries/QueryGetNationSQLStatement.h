//----------------------------------------------------------------------------
//          File:           QueryGetNationSQLStatement.h
//          Description:    QueryGetNationSQLStatement
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
#include "DBAccess/Queries/QueryGetNation.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetNationSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetNationSQLStatement() {};
  virtual ~QueryGetNationSQLStatement() {};

  enum ColumnIndexes
  {
    NATION = 0,
    CREATEDATE,
    EFFDATE,
    EXPIREDATE,
    DISCDATE,
    ISONUMERICCODE,
    SUBAREA,
    AREA,
    ISOALPHACODE,
    PRIMECUR,
    ALTERNATECUR,
    CONVERSIONCUR,
    DESCRIPTION,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select NATION,CREATEDATE,EFFDATE,EXPIREDATE,DISCDATE,"
                  "       ISONUMERICCODE,SUBAREA,AREA,ISOALPHACODE,PRIMECUR,"
                  "       ALTERNATECUR,CONVERSIONCUR,DESCRIPTION");
    this->From("=NATION ");
    this->Where("NATION = %1q "
                " and %cd <= EXPIREDATE ");

    if (DataManager::forceSortOrder())
      this->OrderBy("NATION,CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::Nation* mapRowToNation(Row* row)
  {
    tse::Nation* nation = new tse::Nation;

    nation->nation() = row->getString(NATION);
    nation->effDate() = row->getDate(EFFDATE);
    nation->discDate() = row->getDate(DISCDATE);
    nation->createDate() = row->getDate(CREATEDATE);
    nation->expireDate() = row->getDate(EXPIREDATE);
    nation->isonumericCode() = row->getInt(ISONUMERICCODE);
    nation->subArea() = row->getString(SUBAREA);
    nation->area() = row->getString(AREA);
    nation->isoalphaCode() = row->getString(ISOALPHACODE);
    nation->primeCur() = row->getString(PRIMECUR);
    nation->alternateCur() = row->getString(ALTERNATECUR);
    nation->conversionCur() = row->getString(CONVERSIONCUR);
    nation->description() = row->getString(DESCRIPTION);

    return nation;
  } // mapRowToNation()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetNationsSQLStatement : public QueryGetNationSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE ");
    this->OrderBy("DESCRIPTION ");
  };
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where clause
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetNationHistoricalSQLStatement : public QueryGetNationSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("NATION = %1q "); };
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetNationsHistoricalSQLStatement : public QueryGetNationSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    if (DataManager::forceSortOrder())
      this->OrderBy("DESCRIPTION,NATION,CREATEDATE");
    else
      this->OrderBy("DESCRIPTION ");
  };
};

} // tse
