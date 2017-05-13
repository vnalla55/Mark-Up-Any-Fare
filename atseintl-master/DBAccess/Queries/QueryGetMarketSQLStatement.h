//----------------------------------------------------------------------------
//          File:           QueryGetMarketSQLStatement.h
//          Description:    QueryGetMarketSQLStatement
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
#include "DBAccess/Queries/QueryGetMarket.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMktSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMktSQLStatement() {};
  virtual ~QueryGetMktSQLStatement() {};

  enum ColumnIndexes
  {
    MARKET = 0,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    LATDEG,
    LATMIN,
    LATSEC,
    LNGDEG,
    LNGMIN,
    LNGSEC,
    SUBAREA,
    AREA,
    LATHEM,
    LNGHEM,
    ALASKAZONE,
    BUFFERZONEIND,
    CITYIND,
    MULTITRANSIND,
    RURALARPIND,
    TRANSTYPE,
    CITY,
    NATION,
    STATE,
    DSTGRP,
    DESCRIPTION,
    FARES,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("SELECT MARKET,m.CREATEDATE, m.EXPIREDATE,m.EFFDATE,m.DISCDATE,"
                  "       LATDEG,LATMIN,LATSEC,LNGDEG,LNGMIN,LNGSEC,SUBAREA,AREA,"
                  "       LATHEM,LNGHEM,ALASKAZONE,BUFFERZONEIND,CITYIND,MULTITRANSIND,"
                  "       RURALARPIND,TRANSTYPE,MULTITRANSCITY CITY,NATION,"
                  "       case STATE when ' ' then '' else concat(NATION,STATE) end AS STATE,"
                  "       DSTGRP,DESCRIPTION,FARES");
    this->From("=MARKET m left outer join =MULTITRANSPORT mac"
               "                   on m.MARKET = mac.MULTITRANSLOC "
               "                  and mac.CARRIER = '' "
               "                  and m.EXPIREDATE between mac.EFFDATE and mac.EXPIREDATE ");
    this->Where("m.MARKET = %1q "
                " and %cd <= m.EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("MARKET,CREATEDATE,MULTITRANSCITY,CARRIER,MULTITRANSLOC");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::Loc* mapRowToLoc(Row* row)
  {
    tse::Loc* pMkt = new tse::Loc();

    pMkt->loc() = row->getString(MARKET);
    pMkt->effDate() = row->getDate(EFFDATE);
    pMkt->createDate() = row->getDate(CREATEDATE);
    pMkt->discDate() = row->getDate(DISCDATE);
    pMkt->expireDate() = row->getDate(EXPIREDATE);
    pMkt->latdeg() = row->getInt(LATDEG);
    pMkt->latmin() = row->getInt(LATMIN);
    pMkt->latsec() = row->getInt(LATSEC);
    pMkt->lngdeg() = row->getInt(LNGDEG);
    pMkt->lngmin() = row->getInt(LNGMIN);
    pMkt->lngsec() = row->getInt(LNGSEC);
    pMkt->subarea() = row->getString(SUBAREA);
    pMkt->area() = row->getString(AREA);
    pMkt->lathem() = row->getChar(LATHEM);
    pMkt->lnghem() = row->getChar(LNGHEM);
    pMkt->alaskazone() = row->getChar(ALASKAZONE);
    pMkt->transtype() = row->getChar(TRANSTYPE);
    if (!row->isNull(CITY))
      pMkt->city() = row->getString(CITY);
    pMkt->nation() = row->getString(NATION);
    pMkt->state() = row->getString(STATE);
    pMkt->dstgrp() = row->getString(DSTGRP);
    pMkt->description() = row->getString(DESCRIPTION);

    pMkt->bufferZoneInd() = row->getChar(BUFFERZONEIND) == 'Y';
    pMkt->cityInd() = row->getChar(CITYIND) == 'Y';
    pMkt->multitransind() = row->getChar(MULTITRANSIND) == 'Y';
    pMkt->ruralarpind() = row->getChar(RURALARPIND) == 'Y';
    pMkt->faresind() = row->getChar(FARES) == 'Y';

    return pMkt;
  } // mapRowToLoc()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMktSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetMktHistoricalSQLStatement : public QueryGetMktSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("m.MARKET = %1q "
                " and %2n <= m.EXPIREDATE"
                " and %3n >= m.CREATEDATE");
  };
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllMktSQLStatement : public QueryGetMktSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("m.EXPIREDATE >= %cd"); }
};
} // tse
