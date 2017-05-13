//----------------------------------------------------------------------------
//          File:           QueryGetMaxStayRestrictionSQLStatement.h
//          Description:    QueryGetMaxStayRestrictionSQLStatement
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
#include "DBAccess/Queries/QueryGetMaxStayRestriction.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMaxStayRestrictionSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMaxStayRestrictionSQLStatement() {};
  virtual ~QueryGetMaxStayRestrictionSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    MAXSTAYDATE,
    WAIVERDATE,
    UNAVAILTAG,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    GEOTBLITEMNOFROM,
    GEOTBLITEMNOTO,
    TOD,
    RETURNTRVIND,
    MAXSTAY,
    MAXSTAYUNIT,
    TKTISSUEIND,
    MAXSTAYWAIVER,
    EARLIERLATERIND,
    WAIVEREARLIERLATERIND,
    WAIVERPERIOD,
    WAIVERUNIT,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,MAXSTAYDATE,WAIVERDATE,"
                  "       UNAVAILTAG,TEXTTBLITEMNO,OVERRIDEDATETBLITEMNO,GEOTBLITEMNOFROM,"
                  "       GEOTBLITEMNOTO,TOD,RETURNTRVIND,MAXSTAY,MAXSTAYUNIT,TKTISSUEIND,"
                  "       MAXSTAYWAIVER,EARLIERLATERIND,WAIVEREARLIERLATERIND,WAIVERPERIOD,"
                  "       WAIVERUNIT,INHIBIT");
    this->From("=MAXSTAY");
    this->Where("VENDOR = %1q"
                "    and ITEMNO = %2n"
                "    and %cd <= EXPIREDATE "
                "    and VALIDITYIND = 'Y'");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR , ITEMNO, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MaxStayRestriction* mapRowToMaxStayRestriction(Row* row)
  {
    tse::MaxStayRestriction* msr = new tse::MaxStayRestriction;

    msr->vendor() = row->getString(VENDOR);
    msr->itemNo() = row->getInt(ITEMNO);
    msr->createDate() = row->getDate(CREATEDATE);
    msr->expireDate() = row->getDate(EXPIREDATE);
    msr->maxStayDate() = row->getDate(MAXSTAYDATE);
    msr->waiverDate() = row->getDate(WAIVERDATE);
    msr->unavailTag() = row->getChar(UNAVAILTAG);
    msr->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    msr->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    msr->geoTblItemNoFrom() = row->getInt(GEOTBLITEMNOFROM);
    msr->geoTblItemNoTo() = row->getInt(GEOTBLITEMNOTO);
    msr->tod() = row->getInt(TOD);
    msr->returnTrvInd() = row->getChar(RETURNTRVIND);
    msr->maxStay() = row->getString(MAXSTAY);
    msr->maxStayUnit() = row->getString(MAXSTAYUNIT);
    msr->tktIssueInd() = row->getChar(TKTISSUEIND);
    msr->maxStayWaiver() = row->getChar(MAXSTAYWAIVER);
    msr->earlierLaterInd() = row->getChar(EARLIERLATERIND);
    msr->waiverEarlierLaterInd() = row->getChar(WAIVEREARLIERLATERIND);
    msr->waiverPeriod() = row->getString(WAIVERPERIOD);
    msr->waiverUnit() = row->getChar(WAIVERUNIT);
    msr->inhibit() = row->getChar(INHIBIT);

    return msr;
  } // mapRowToMaxStayRestriction()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMaxStayRestrictionSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetMaxStayRestrictionHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMaxStayRestrictionHistoricalSQLStatement
    : public QueryGetMaxStayRestrictionSQLStatement<QUERYCLASS>
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
}; // class QueryGetMaxStayRestrictionHistoricalSQLStatement
} // tse
