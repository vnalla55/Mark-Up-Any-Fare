//----------------------------------------------------------------------------
//          File:           QueryGetDayTimeApplicationsSQLStatement.h
//          Description:    QueryGetDayTimeApplicationsSQLStatement
//          Created:        11/01/2007
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
#include "DBAccess/Queries/QueryGetDayTimeApplications.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetDayTimeApplicationsSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetDayTimeApplicationsSQLStatement() {};
  virtual ~QueryGetDayTimeApplicationsSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    UNAVAILTAG,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    GEOTBLITEMNO,
    STARTTIME,
    STOPTIME,
    TODAPPL,
    DOW,
    DOWSAME,
    DOWOCCUR,
    DAYTIMENEG,
    DAYTIMEAPPL,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,UNAVAILTAG,TEXTTBLITEMNO,"
                  "       OVERRIDEDATETBLITEMNO,GEOTBLITEMNO,STARTTIME,STOPTIME,TODAPPL,"
                  "       DOW,DOWSAME,DOWOCCUR,DAYTIMENEG,DAYTIMEAPPL,INHIBIT");
    this->From("=DAYTIMEAPP");
    this->Where("VENDOR = %1q"
                "    and ITEMNO = %2n"
                "    and %cd <= EXPIREDATE "
                "    and VALIDITYIND = 'Y'");
    if (DataManager::forceSortOrder())
      this->OrderBy("vendor, itemno, createdate");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::DayTimeAppInfo* mapRowToDayTimeAppInfo(Row* row)
  {
    tse::DayTimeAppInfo* dtApp = new tse::DayTimeAppInfo;

    dtApp->vendor() = row->getString(VENDOR);
    dtApp->itemNo() = row->getInt(ITEMNO);
    dtApp->createDate() = row->getDate(CREATEDATE);
    dtApp->expireDate() = row->getDate(EXPIREDATE);
    dtApp->unavailtag() = row->getChar(UNAVAILTAG);
    dtApp->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    dtApp->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    dtApp->geoTblItemNo() = row->getInt(GEOTBLITEMNO);
    dtApp->startTime() = row->getInt(STARTTIME);
    dtApp->stopTime() = row->getInt(STOPTIME);
    dtApp->todAppl() = row->getChar(TODAPPL);
    dtApp->dow() = row->getString(DOW);
    dtApp->dowSame() = row->getChar(DOWSAME);
    dtApp->dowOccur() = row->getInt(DOWOCCUR);
    dtApp->dayTimeNeg() = row->getChar(DAYTIMENEG);
    dtApp->dayTimeAppl() = row->getChar(DAYTIMEAPPL);
    dtApp->inhibit() = row->getChar(INHIBIT);

    return dtApp;
  } // mapRowToDayTimeAppInfo

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetDayTimeApplicationsSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetDayTimeApplicationsHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetDayTimeApplicationsHistoricalSQLStatement
    : public QueryGetDayTimeApplicationsSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                "  and ITEMNO = %2n"
                "  and VALIDITYIND = 'Y'"
                "  and %3n <= EXPIREDATE"
                "  and %4n >= CREATEDATE");
  }
}; // class QueryGetDayTimeApplicationsHistoricalSQLStatement
} // tse
