//----------------------------------------------------------------------------
//          File:           QueryGetTravelRestrictionSQLStatement.h
//          Description:    QueryGetTravelRestrictionSQLStatement
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
#include "DBAccess/Queries/QueryGetTravelRestriction.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetTravelRestrictionSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTravelRestrictionSQLStatement() {}
  virtual ~QueryGetTravelRestrictionSQLStatement() {}

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
    STOPTIME,
    EARLIESTTVLSTARTDATE,
    LATESTTVLSTARTDATE,
    STOPDATE,
    RETURNTVLIND,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,UNAVAILTAG,"
                  "       TEXTTBLITEMNO,OVERRIDEDATETBLITEMNO,GEOTBLITEMNO,STOPTIME,"
                  "       EARLIESTTVLSTARTDATE,LATESTTVLSTARTDATE,STOPDATE,"
                  "       RETURNTVLIND,INHIBIT");
    this->From("=TVLREST");
    this->Where("VENDOR = %1q"
                "    and ITEMNO = %2n"
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

  static tse::TravelRestriction* mapRowToTravelRestriction(Row* row)
  {
    tse::TravelRestriction* tr = new tse::TravelRestriction;

    tr->vendor() = row->getString(VENDOR);
    tr->itemNo() = row->getInt(ITEMNO);
    tr->createDate() = row->getDate(CREATEDATE);
    tr->expireDate() = row->getDate(EXPIREDATE);
    tr->unavailTag() = row->getChar(UNAVAILTAG);
    tr->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    tr->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    tr->geoTblItemNo() = row->getInt(GEOTBLITEMNO);
    tr->stopTime() = row->getInt(STOPTIME);
    tr->earliestTvlStartDate() = row->getDate(EARLIESTTVLSTARTDATE);
    tr->latestTvlStartDate() = row->getDate(LATESTTVLSTARTDATE);
    tr->stopDate() = row->getDate(STOPDATE);
    tr->returnTvlInd() = row->getChar(RETURNTVLIND);
    tr->inhibit() = row->getChar(INHIBIT);

    return tr;
  } // mapRowToTravelRestriction()
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetTravelRestrictionSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTravelRestrictionHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTravelRestrictionHistoricalSQLStatement
    : public QueryGetTravelRestrictionSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" VENDOR = %1q"
                " and ITEMNO = %2n "
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  }
}; // class QueryGetTravelRestrictionHistoricalSQLStatement
}

