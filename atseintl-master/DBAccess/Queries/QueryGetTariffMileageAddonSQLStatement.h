//----------------------------------------------------------------------------
//          File:           QueryGetTariffMileageAddonSQLStatement.h
//          Description:    QueryGetTariffMileageAddonSQLStatement
//          Created:        10/5/2007
//          Authors:         Mike Lillis
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
#include "DBAccess/Queries/QueryGetTariffMileageAddon.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetTariffMileageAddonSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTariffMileageAddonSQLStatement() {}
  virtual ~QueryGetTariffMileageAddonSQLStatement() {}

  enum ColumnIndexes
  {
    CARRIER = 0,
    UNPUBLISHEDADDONLOC,
    GLOBALDIR,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    MILESADDED,
    PUBLISHEDLOC,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CARRIER,UNPUBLISHEDADDONLOC,GLOBALDIR,CREATEDATE,"
                  "       EXPIREDATE,EFFDATE,DISCDATE,MILESADDED,PUBLISHEDLOC");
    this->From(" =TARIFFMILEAGEADDON");
    this->Where("CARRIER = %1q "
                "    and UNPUBLISHEDADDONLOC = %2q "
                "    and GLOBALDIR = %3q "
                "    and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER,UNPUBLISHEDADDONLOC,GLOBALDIR,CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TariffMileageAddon* mapRowToTariffMileageAddon(Row* row)
  {
    tse::TariffMileageAddon* tma = new tse::TariffMileageAddon;

    tma->carrier() = row->getString(CARRIER);
    tma->unpublishedAddonLoc() = row->getString(UNPUBLISHEDADDONLOC);

    std::string gd = row->getString(GLOBALDIR);
    strToGlobalDirection(tma->globalDir(), gd);

    tma->effDate() = row->getDate(EFFDATE);
    tma->createDate() = row->getDate(CREATEDATE);
    tma->expireDate() = row->getDate(EXPIREDATE);
    tma->discDate() = row->getDate(DISCDATE);
    tma->milesAdded() = row->getInt(MILESADDED);
    tma->publishedLoc() = row->getString(PUBLISHEDLOC);

    return tma;
  } // mapRowToTariffMileageAddon()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
////////////////////////////////////////////////////////////////////////
//
//   Template used to replace Where clause
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetTariffMileageAddonHistoricalSQLStatement
    : public QueryGetTariffMileageAddonSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("CARRIER = %1q "
                "    and UNPUBLISHEDADDONLOC = %2q "
                "    and GLOBALDIR = %3q ");
    this->OrderBy("CREATEDATE");
  }
};
////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllTariffMileageAddonSQLStatement
    : public QueryGetTariffMileageAddonSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("CARRIER,UNPUBLISHEDADDONLOC,GLOBALDIR,CREATEDATE");
    else
      this->OrderBy("1, 2, 3");
  }
};
////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllTariffMileageAddonHistoricalSQLStatement
    : public QueryGetTariffMileageAddonSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("");
    this->OrderBy("CARRIER,UNPUBLISHEDADDONLOC,GLOBALDIR,CREATEDATE");
  }
};
}

