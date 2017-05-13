//----------------------------------------------------------------------------
//          File:           QueryGetAdvResTktSQLStatement.h
//          Description:    QueryGetAdvResTktSQLStatement
//          Created:        10/25/2007
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
#include "DBAccess/Queries/QueryGetAdvResTkt.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetAdvResTktSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAdvResTktSQLStatement() {};
  virtual ~QueryGetAdvResTktSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    UNAVAILTAG,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    ADVTKTDEPART,
    GEOTBLITEMNO,
    FIRSTRESTOD,
    LASTRESTOD,
    ADVTKTTOD,
    ADVTKTEXCPTIME,
    TKTTSI,
    RESTSI,
    FIRSTRESPERIOD,
    FIRSTRESUNIT,
    LASTRESPERIOD,
    LASTRESUNIT,
    PERMITTED,
    TICKETED,
    STANDBY,
    CONFIRMEDSECTOR,
    EACHSECTOR,
    ADVTKTPERIOD,
    ADVTKTUNIT,
    ADVTKTOPT,
    ADVTKTDEPARTUNIT,
    ADVTKTBOTH,
    ADVTKTEXCPUNIT,
    WAIVERRESDATE,
    WAIVERTKTDATE,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,UNAVAILTAG,TEXTTBLITEMNO,"
                  "       OVERRIDEDATETBLITEMNO,ADVTKTDEPART,GEOTBLITEMNO,FIRSTRESTOD,"
                  "       LASTRESTOD,ADVTKTTOD,ADVTKTEXCPTIME,TKTTSI,RESTSI,FIRSTRESPERIOD,"
                  "       FIRSTRESUNIT,LASTRESPERIOD,LASTRESUNIT,PERMITTED,TICKETED,STANDBY,"
                  "       CONFIRMEDSECTOR,EACHSECTOR,ADVTKTPERIOD,ADVTKTUNIT,ADVTKTOPT,"
                  "       ADVTKTDEPARTUNIT,ADVTKTBOTH,ADVTKTEXCPUNIT,WAIVERRESDATE,"
                  "       WAIVERTKTDATE,INHIBIT");
    this->From("=ADVRESTKT");
    this->Where("VENDOR = %1q"
                "   and ITEMNO = %2n"
                "   and %cd <= EXPIREDATE"
                "   and VALIDITYIND = 'Y'");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::AdvResTktInfo* mapRowToAdvResTktInfo(Row* row)
  {
    tse::AdvResTktInfo* adv = new tse::AdvResTktInfo;

    adv->vendor() = row->getString(VENDOR);
    adv->itemNo() = row->getInt(ITEMNO);
    adv->createDate() = row->getDate(CREATEDATE);
    adv->expireDate() = row->getDate(EXPIREDATE);
    adv->unavailtag() = row->getChar(UNAVAILTAG);
    adv->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    adv->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    adv->advTktdepart() = row->getInt(ADVTKTDEPART);
    adv->geoTblItemNo() = row->getInt(GEOTBLITEMNO);
    adv->firstResTod() = row->getInt(FIRSTRESTOD);
    adv->lastResTod() = row->getInt(LASTRESTOD);
    adv->advTktTod() = row->getInt(ADVTKTTOD);
    adv->advTktExcpTime() = row->getInt(ADVTKTEXCPTIME);
    adv->tktTSI() = row->getInt(TKTTSI);
    adv->resTSI() = row->getInt(RESTSI);
    adv->firstResPeriod() = row->getString(FIRSTRESPERIOD);
    adv->firstResUnit() = row->getString(FIRSTRESUNIT);
    adv->lastResPeriod() = row->getString(LASTRESPERIOD);
    adv->lastResUnit() = row->getString(LASTRESUNIT);
    adv->permitted() = row->getChar(PERMITTED);
    adv->ticketed() = row->getChar(TICKETED);
    adv->standby() = row->getChar(STANDBY);
    adv->confirmedSector() = row->getChar(CONFIRMEDSECTOR);
    adv->eachSector() = row->getChar(EACHSECTOR);
    adv->advTktPeriod() = row->getString(ADVTKTPERIOD);
    adv->advTktUnit() = row->getString(ADVTKTUNIT);
    adv->advTktOpt() = row->getChar(ADVTKTOPT);
    adv->advTktDepartUnit() = row->getChar(ADVTKTDEPARTUNIT);
    adv->advTktBoth() = row->getChar(ADVTKTBOTH);
    adv->advTktExcpUnit() = row->getChar(ADVTKTEXCPUNIT);
    adv->waiverResDate() = row->getDate(WAIVERRESDATE);
    adv->waiverTktDate() = row->getDate(WAIVERTKTDATE);
    adv->inhibit() = row->getChar(INHIBIT);

    return adv;
  } // mapRowToAdvResTktInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetAdvResTktSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAdvResTktHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAdvResTktHistoricalSQLStatement : public QueryGetAdvResTktSQLStatement<QUERYCLASS>
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
}; // class QueryGetAdvResTktHistoricalSQLStatement
} // tse
