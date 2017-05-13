//----------------------------------------------------------------------------
//          File:           QueryGetTaxAkHiFactorSQLStatement.h
//          Description:    QueryGetTaxAkHiFactorSQLStatement
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
#include "DBAccess/Queries/QueryGetTaxAkHiFactor.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetOneTaxAkHiFactorSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetOneTaxAkHiFactorSQLStatement() {}
  virtual ~QueryGetOneTaxAkHiFactorSQLStatement() {}

  enum ColumnIndexes
  {
    CITY = 0,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    ZONEANODEC,
    ZONEAPERCENT,
    ZONEBNODEC,
    ZONEBPERCENT,
    ZONECNODEC,
    ZONECPERCENT,
    ZONEDNODEC,
    ZONEDPERCENT,
    HAWAIINODEC,
    HAWAIIPERCENT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select CITY,c.CREATEDATE,EXPIREDATE,EFFDATE,DISCDATE,ZONEANODEC,"
                  "       ZONEAPERCENT,ZONEBNODEC,ZONEBPERCENT,ZONECNODEC,ZONECPERCENT,"
                  "       ZONEDNODEC,ZONEDPERCENT,HAWAIINODEC,HAWAIIPERCENT");
    this->From(" =TAXAKHIFACTOR f, =TAXAKHICITY c");
    this->Where(" c.CITY = %1q"
                "    and c.VERSIONDATE = f.VERSIONDATE"
                "    and c.CREATEDATE  = f.CREATEDATE"
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("f.VERSIONDATE, f.CREATEDATE, c.CITY");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TaxAkHiFactor* mapRowToTaxAkHiFactor(Row* row)
  {
    TaxAkHiFactor* tahf = new tse::TaxAkHiFactor;

    tahf->city() = row->getString(CITY);
    tahf->createDate() = row->getDate(CREATEDATE);
    tahf->expireDate() = row->getDate(EXPIREDATE);
    tahf->effDate() = row->getDate(EFFDATE);
    tahf->discDate() = row->getDate(DISCDATE);

    tahf->zoneANoDec() = row->getInt(ZONEANODEC);
    tahf->zoneAPercent() = QUERYCLASS::adjustDecimal(row->getInt(ZONEAPERCENT), tahf->zoneANoDec());

    tahf->zoneBNoDec() = row->getInt(ZONEBNODEC);
    tahf->zoneBPercent() = QUERYCLASS::adjustDecimal(row->getInt(ZONEBPERCENT), tahf->zoneBNoDec());

    tahf->zoneCNoDec() = row->getInt(ZONECNODEC);
    tahf->zoneCPercent() = QUERYCLASS::adjustDecimal(row->getInt(ZONECPERCENT), tahf->zoneCNoDec());

    tahf->zoneDNoDec() = row->getInt(ZONEDNODEC);
    tahf->zoneDPercent() = QUERYCLASS::adjustDecimal(row->getInt(ZONEDPERCENT), tahf->zoneDNoDec());

    tahf->hawaiiNoDec() = row->getInt(HAWAIINODEC);
    tahf->hawaiiPercent() =
        QUERYCLASS::adjustDecimal(row->getInt(HAWAIIPERCENT), tahf->hawaiiNoDec());

    return tahf;
  } // mapRowToTaxAkHiFactor()
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllTaxAkHiFactorSQLStatement : public QueryGetOneTaxAkHiFactorSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("c.VERSIONDATE = f.VERSIONDATE and "
                "c.CREATEDATE = f.CREATEDATE and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("f.VERSIONDATE, f.CREATEDATE, c.CITY");
    else
      this->OrderBy("CITY");
  }
};

template <class QUERYCLASS>
class QueryGetOneTaxAkHiFactorHistoricalSQLStatement
    : public QueryGetOneTaxAkHiFactorSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where(" c.CITY = %1q"
                "    and c.VERSIONDATE = f.VERSIONDATE"
                "    and c.CREATEDATE  = f.CREATEDATE"
                "    and %2n <= f.EXPIREDATE"
                "    and (%3n >= f.CREATEDATE"
                "     or %4n >= f.EFFDATE)");
  }
};

} // tse
