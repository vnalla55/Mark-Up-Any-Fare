//----------------------------------------------------------------------------
//          File:           QueryGetSeasonalApplSQLStatement.h
//          Description:    QueryGetSeasonalApplSQLStatement
//          Created:        11/02/2007
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
#include "DBAccess/Queries/QueryGetSeasonalAppl.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetSeasonalApplSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSeasonalApplSQLStatement() {};
  virtual ~QueryGetSeasonalApplSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    VALIDITYIND,
    INHIBIT,
    UNAVAILTAG,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    GEOTBLITEMNO,
    SEASONDATEAPPL,
    TVLSTARTYEAR,
    TVLSTARTMONTH,
    TVLSTARTDAY,
    TVLSTOPYEAR,
    TVLSTOPMONTH,
    TVLSTOPDAY,
    ASSUMPTIONOVERRIDE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,VALIDITYIND,INHIBIT,"
                  "       UNAVAILTAG,TEXTTBLITEMNO,OVERRIDEDATETBLITEMNO,GEOTBLITEMNO,"
                  "       SEASONDATEAPPL,TVLSTARTYEAR,TVLSTARTMONTH,TVLSTARTDAY,"
                  "       TVLSTOPYEAR,TVLSTOPMONTH,TVLSTOPDAY,ASSUMPTIONOVERRIDE");
    this->From("=SEASONALAPP ");
    this->Where("VENDOR = %1q "
                "    and ITEMNO = %2n "
                "    and %cd <= EXPIREDATE "
                "    and VALIDITYIND = 'Y'");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::SeasonalAppl* mapRowToSeasonalAppl(Row* row)
  {
    tse::SeasonalAppl* seasAppl = new tse::SeasonalAppl;

    seasAppl->vendor() = row->getString(VENDOR);
    seasAppl->itemNo() = row->getInt(ITEMNO);
    seasAppl->createDate() = row->getDate(CREATEDATE);
    seasAppl->expireDate() = row->getDate(EXPIREDATE);
    seasAppl->validityInd() = row->getChar(VALIDITYIND);
    seasAppl->inhibit() = row->getChar(INHIBIT);
    seasAppl->unavailtag() = row->getChar(UNAVAILTAG);
    seasAppl->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    seasAppl->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    seasAppl->geoTblItemNo() = row->getInt(GEOTBLITEMNO);
    seasAppl->seasonDateAppl() = row->getChar(SEASONDATEAPPL);
    const char* startYear = row->getString(TVLSTARTYEAR);
    if (isdigit(startYear[0]))
    {
      seasAppl->tvlstartyear() = row->getInt(TVLSTARTYEAR);
      if (seasAppl->tvlstartyear() < 2000)
        seasAppl->tvlstartyear() += 2000;
    }
    else
    {
      seasAppl->tvlstartyear() = 0;
    }
    seasAppl->tvlstartmonth() = row->getInt(TVLSTARTMONTH);
    seasAppl->tvlstartDay() = row->getInt(TVLSTARTDAY);
    const char* stopYear = row->getString(TVLSTOPYEAR);
    if (isdigit(stopYear[0]))
    {
      seasAppl->tvlStopyear() = row->getInt(TVLSTOPYEAR);
      if (seasAppl->tvlStopyear() < 2000)
        seasAppl->tvlStopyear() += 2000;
    }
    else
    {
      seasAppl->tvlStopyear() = 0;
    }
    seasAppl->tvlStopmonth() = row->getInt(TVLSTOPMONTH);
    seasAppl->tvlStopDay() = row->getInt(TVLSTOPDAY);
    seasAppl->assumptionOverride() = row->getChar(ASSUMPTIONOVERRIDE);

    return seasAppl;
  } // mapRowToSeasonalAppl()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetSeasonalApplSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetSeasonalApplHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSeasonalApplHistoricalSQLStatement
    : public QueryGetSeasonalApplSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q "
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  }
}; // class QueryGetSeasonalApplHistoricalSQLStatement
} // tse
