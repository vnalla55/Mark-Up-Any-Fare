//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesCurrencySQLStatement.h
//          Description:    QueryGetSvcFeesCurrencySQLStatement
//          Created:        11/10/2009
// Authors:
//
//          Updates:
//
//      2009, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSvcFeesCurrency.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetSvcFeesCurrencySQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSvcFeesCurrencySQLStatement() {};
  virtual ~QueryGetSvcFeesCurrencySQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    VALIDITYIND,
    POSLOCTYPE,
    POSLOC,
    CUR,
    FEEAMOUNT,
    NODEC,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, ITEMNO, SEQNO, "
                  "       CREATEDATE, EXPIREDATE, VALIDITYIND, "
                  "       POSLOCTYPE, POSLOC, CUR, FEEAMOUNT, NODEC");

    this->From("=SVCFEESCURRENCY");

    this->Where("VENDOR      = %1q  "
                " and ITEMNO = %2n "
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE ");
    this->OrderBy("SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  } // RegisterColumnsAndBaseSQL()

  static tse::SvcFeesCurrencyInfo* mapRowToSvcFeesCurrencyInfo(Row* row)
  {
    tse::SvcFeesCurrencyInfo* crcy = new tse::SvcFeesCurrencyInfo;

    crcy->vendor() = row->getString(VENDOR);
    crcy->itemNo() = row->getInt(ITEMNO);
    crcy->seqNo() = row->getLong(SEQNO);
    crcy->createDate() = row->getDate(CREATEDATE);
    if (!row->isNull(EXPIREDATE))
      crcy->expireDate() = row->getDate(EXPIREDATE);
    LocKey* loc = &crcy->posLoc();
    if (!row->isNull(POSLOCTYPE))
      loc->locType() = row->getChar(POSLOCTYPE);
    if (!row->isNull(POSLOC))
      loc->loc() = row->getString(POSLOC);
    if (!row->isNull(CUR))
      crcy->currency() = row->getString(CUR);
    if (!row->isNull(NODEC))
      crcy->noDec() = row->getInt(NODEC);
    if (!row->isNull(FEEAMOUNT))
      crcy->feeAmount() = QUERYCLASS::adjustDecimal(row->getInt(FEEAMOUNT), crcy->noDec());

    return crcy;
  }; // mapRowToSvcFeesCurrencyInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetSvcFeesCurrencySQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSvcFeesCurrencyHistoricalSQLStatement
    : public QueryGetSvcFeesCurrencySQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q  "
                " and ITEMNO= %2n "
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
    this->OrderBy("SEQNO, CREATEDATE");
  };
};
}
