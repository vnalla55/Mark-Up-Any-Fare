//----------------------------------------------------------------------------
//          File:           QueryGetMinStayRestrictionSQLStatement.h
//          Description:    QueryGetMinStayRestrictionSQLStatement
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
#include "DBAccess/Queries/QueryGetMinStayRestriction.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMinStayRestrictionSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMinStayRestrictionSQLStatement() {};
  virtual ~QueryGetMinStayRestrictionSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    MINSTAYDATE,
    WAIVERDATE,
    UNAVAILTAG,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    GEOTBLITEMNOFROM,
    GEOTBLITEMNOTO,
    TOD,
    ORIGINDOW,
    MINSTAY,
    MINSTAYUNIT,
    MINSTAYWAIVER,
    EARLIERLATERIND,
    WAIVEREARLIERLATERIND,
    WAIVERPERIOD,
    WAIVERUNIT,
    MILEAGEAPPL,
    MILEAGEGREATERTHAN,
    MILEAGELESSTHAN,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,MINSTAYDATE,WAIVERDATE,"
                  "       UNAVAILTAG,TEXTTBLITEMNO,OVERRIDEDATETBLITEMNO,GEOTBLITEMNOFROM,"
                  "       GEOTBLITEMNOTO,TOD,ORIGINDOW,MINSTAY,MINSTAYUNIT,MINSTAYWAIVER,"
                  "       EARLIERLATERIND,WAIVEREARLIERLATERIND,WAIVERPERIOD,WAIVERUNIT,"
                  "       MILEAGEAPPL,MILEAGEGREATERTHAN,MILEAGELESSTHAN,INHIBIT");
    this->From("=MINSTAY");
    this->Where("VENDOR = %1q"
                "    and ITEMNO = %2n"
                "    and %cd <= EXPIREDATE "
                "    and VALIDITYIND = 'Y'");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,ITEMNO,CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MinStayRestriction* mapRowToMinStayRestriction(Row* row)
  {
    tse::MinStayRestriction* msr = new tse::MinStayRestriction;

    msr->vendor() = row->getString(VENDOR);
    msr->itemNo() = row->getInt(ITEMNO);
    msr->createDate() = row->getDate(CREATEDATE);
    msr->expireDate() = row->getDate(EXPIREDATE);
    msr->minStayDate() = row->getDate(MINSTAYDATE);
    msr->waiverDate() = row->getDate(WAIVERDATE);
    msr->unavailTag() = row->getChar(UNAVAILTAG);
    msr->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    msr->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    msr->geoTblItemNoFrom() = row->getInt(GEOTBLITEMNOFROM);
    msr->geoTblItemNoTo() = row->getInt(GEOTBLITEMNOTO);
    msr->tod() = row->getInt(TOD);
    msr->originDow() = row->getString(ORIGINDOW);
    msr->minStay() = row->getString(MINSTAY);
    msr->minStayUnit() = row->getString(MINSTAYUNIT);
    msr->minStayWaiver() = row->getChar(MINSTAYWAIVER);
    msr->earlierLaterInd() = row->getChar(EARLIERLATERIND);
    msr->waiverEarlierLaterInd() = row->getChar(WAIVEREARLIERLATERIND);
    msr->waiverPeriod() = row->getString(WAIVERPERIOD);
    msr->waiverUnit() = row->getChar(WAIVERUNIT);
    msr->mileageAppl() = row->getChar(MILEAGEAPPL);
    msr->mileageGreaterThan() = row->getInt(MILEAGEGREATERTHAN);
    msr->mileageLessThan() = row->getInt(MILEAGELESSTHAN);
    msr->inhibit() = row->getChar(INHIBIT);

    return msr;
  } // mapRowToMinStayRestriction()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMinStayRestrictionSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetMinStayRestrictionHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMinStayRestrictionHistoricalSQLStatement
    : public QueryGetMinStayRestrictionSQLStatement<QUERYCLASS>
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
}; // class QueryGetMinStayRestrictionHistoricalSQLStatement
} // tse
