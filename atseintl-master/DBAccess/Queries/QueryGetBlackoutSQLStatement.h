//----------------------------------------------------------------------------
//          File:           QueryGetBlackoutSQLStatement.h
//          Description:    QueryGetBlackoutSQLStatement
//          Created:        10/26/2007
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
#include "DBAccess/Queries/QueryGetBlackout.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetBlackoutSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetBlackoutSQLStatement() {};
  virtual ~QueryGetBlackoutSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    CREATORID,
    CREATORBUSINESSUNIT,
    VALIDITYIND,
    INHIBIT,
    UNAVAILTAG,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    GEOTBLITEMNOBETWEEN,
    GEOTBLITEMNOAND,
    BLACKOUTAPPL,
    TVLSTARTYEAR,
    TVLSTARTMONTH,
    TVLSTARTDAY,
    TVLSTOPYEAR,
    TVLSTOPMONTH,
    TVLSTOPDAY,
    INTLREST,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,CREATORID,"
                  "       CREATORBUSINESSUNIT,VALIDITYIND,INHIBIT,UNAVAILTAG,"
                  "       TEXTTBLITEMNO,OVERRIDEDATETBLITEMNO,GEOTBLITEMNOBETWEEN,"
                  "       GEOTBLITEMNOAND,BLACKOUTAPPL,TVLSTARTYEAR,TVLSTARTMONTH,"
                  "       TVLSTARTDAY,TVLSTOPYEAR,TVLSTOPMONTH,TVLSTOPDAY,INTLREST");
    this->From("=BLACKOUTDATES");
    this->Where("VENDOR = %1q "
                "    and ITEMNO = %2n "
                "    and %cd <= EXPIREDATE "
                "    and VALIDITYIND = 'Y'");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::BlackoutInfo* mapRowToBlackoutInfo(Row* row)
  {
    tse::BlackoutInfo* blk = new tse::BlackoutInfo;

    blk->vendor() = row->getString(VENDOR);
    blk->itemNo() = row->getInt(ITEMNO);
    blk->createDate() = row->getDate(CREATEDATE);
    blk->expireDate() = row->getDate(EXPIREDATE);
    blk->unavailtag() = row->getChar(UNAVAILTAG);
    blk->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    blk->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    blk->geoTblItemNoBetween() = row->getInt(GEOTBLITEMNOBETWEEN);
    blk->geoTblItemNoAnd() = row->getInt(GEOTBLITEMNOAND);
    blk->blackoutAppl() = row->getChar(BLACKOUTAPPL);
    const char* startYear = row->getString(TVLSTARTYEAR);
    if (isdigit(startYear[0]))
    {
      blk->tvlStartYear() = QUERYCLASS::stringToInteger(startYear, __LINE__);
    }
    else
    {
      blk->tvlStartYear() = -1;
    }
    blk->tvlStartMonth() = row->getInt(TVLSTARTMONTH);
    blk->tvlStartDay() = row->getInt(TVLSTARTDAY);
    const char* stopYear = row->getString(TVLSTOPYEAR);
    if (isdigit(stopYear[0]))
    {
      blk->tvlStopYear() = QUERYCLASS::stringToInteger(stopYear, __LINE__);
    }
    else
    {
      blk->tvlStopYear() = -1;
    }
    blk->tvlStopMonth() = row->getInt(TVLSTOPMONTH);
    blk->tvlStopDay() = row->getInt(TVLSTOPDAY);
    blk->intlRest() = row->getChar(INTLREST);
    blk->inhibit() = row->getChar(INHIBIT);

    return blk;
  } // mapRowToBlackoutInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetBlackoutSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetBlackoutHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetBlackoutHistoricalSQLStatement : public QueryGetBlackoutSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  }
}; // class QueryGetBlackoutHistoricalSQLStatement
} // tse
