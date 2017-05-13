//----------------------------------------------------------------------------
//          File:           QueryGetSurfaceTransfersInfoSQLStatement.h
//          Description:    QueryGetSurfaceTransfersInfoSQLStatement
//          Created:        11/02/2007
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
#include "DBAccess/Queries/QueryGetSurfaceTransfersInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetSurfaceTransfersInfoSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSurfaceTransfersInfoSQLStatement() {};
  virtual ~QueryGetSurfaceTransfersInfoSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    RESTRICTION,
    ORIGINDEST,
    FAREBRKEMBEDDEDLOCTYPE,
    FAREBRKEMBEDDEDLOC,
    SURFACELOCTYPE,
    SURFACELOC,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,SEQNO,CREATEDATE,EXPIREDATE,"
                  "       RESTRICTION,ORIGINDEST,FAREBRKEMBEDDEDLOCTYPE,"
                  "       FAREBRKEMBEDDEDLOC,SURFACELOCTYPE,SURFACELOC,"
                  "       INHIBIT");
    this->From("=SURFACETRANSFERS");
    this->Where("VENDOR = %1q"
                "    and ITEMNO = %2n"
                "    and VALIDITYIND = 'Y'"
                "    and %cd <= EXPIREDATE");
    this->OrderBy("SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::SurfaceTransfersInfo* mapRowToSurfaceTransfersInfo(Row* row)
  {
    SurfaceTransfersInfo* ti = new tse::SurfaceTransfersInfo;

    ti->vendor() = row->getString(VENDOR);
    ti->itemNo() = row->getInt(ITEMNO);
    ti->seqNo() = row->getLong(SEQNO);
    ti->createDate() = row->getDate(CREATEDATE);
    ti->expireDate() = row->getDate(EXPIREDATE);
    ti->restriction() = row->getChar(RESTRICTION);
    ti->originDest() = row->getChar(ORIGINDEST);

    LocKey* tLoc = &ti->fareBrkEmbeddedLoc();
    tLoc->locType() = row->getChar(FAREBRKEMBEDDEDLOCTYPE);
    tLoc->loc() = row->getString(FAREBRKEMBEDDEDLOC);

    tLoc = &ti->surfaceLoc();
    tLoc->locType() = row->getChar(SURFACELOCTYPE);
    tLoc->loc() = row->getString(SURFACELOC);

    ti->inhibit() = row->getChar(INHIBIT);

    return ti;
  } // mapRowToSurfaceTransfersInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetSurfaceTransfersInfoSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetSurfaceTransfersInfoHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSurfaceTransfersInfoHistoricalSQLStatement
    : public QueryGetSurfaceTransfersInfoSQLStatement<QUERYCLASS>
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
}; // class QueryGetSurfaceTransfersInfoHistoricalSQLStatement
} // tse
