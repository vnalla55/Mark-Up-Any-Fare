//----------------------------------------------------------------------------
//          File:           QueryGetCarrierApplicationInfoSQLStatement.h
//          Description:    QueryGetCarrierApplicationInfoSQLStatement
//          Created:        10/29/2007
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
#include "DBAccess/Queries/QueryGetCarrierApplicationInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCarrierApplicationInfoSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCarrierApplicationInfoSQLStatement() {};
  virtual ~QueryGetCarrierApplicationInfoSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    ORDERNO,
    CREATEDATE,
    EXPIREDATE,
    APPLIND,
    CARRIER,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,ORDERNO,CREATEDATE,EXPIREDATE,APPLIND,"
                  "       CARRIER,INHIBIT");
    this->From("=CARRIERAPPLICATION");
    this->Where("VENDOR = %1q"
                "    and ITEMNO = %2n"
                "    and %cd <= EXPIREDATE "
                "    and VALIDITYIND = 'Y'");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::CarrierApplicationInfo* mapRowToCarrierApplicationInfo(Row* row)
  {
    tse::CarrierApplicationInfo* cai = new tse::CarrierApplicationInfo;

    cai->vendor() = row->getString(VENDOR);
    cai->itemNo() = row->getInt(ITEMNO);
    cai->orderNo() = row->getInt(ORDERNO);
    cai->createDate() = row->getDate(CREATEDATE);
    cai->expireDate() = row->getDate(EXPIREDATE);
    cai->applInd() = row->getChar(APPLIND);
    cai->carrier() = row->getString(CARRIER);
    cai->inhibit() = row->getChar(INHIBIT);

    return cai;
  } // mapRowToCarrierApplicationInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetCarrierApplicationInfoSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetCarrierApplicationInfoHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCarrierApplicationInfoHistoricalSQLStatement
    : public QueryGetCarrierApplicationInfoSQLStatement<QUERYCLASS>
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
}; // class QueryGetCarrierApplicationInfoHistoricalSQLStatement
} // tse
