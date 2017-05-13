//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesResBkgDesigSQLStatement.h
//          Description:    QueryGetSvcFeesResBkgDesigSQLStatement
//          Created:        11/12/2009
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
#include "DBAccess/Queries/QueryGetSvcFeesResBkgDesig.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetSvcFeesResBkgDesigSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSvcFeesResBkgDesigSQLStatement() {};
  virtual ~QueryGetSvcFeesResBkgDesigSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    VALIDITYIND,
    MKGOPERIND,
    CARRIER,
    BOOKINGCODE1,
    BOOKINGCODE2,
    BOOKINGCODE3,
    BOOKINGCODE4,
    BOOKINGCODE5,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, ITEMNO, SEQNO,  "
                  "       CREATEDATE, EXPIREDATE, VALIDITYIND, MKGOPERIND, CARRIER, "
                  "       BOOKINGCODE1, BOOKINGCODE2, BOOKINGCODE3, BOOKINGCODE4, BOOKINGCODE5");

    this->From("=SVCFEESRESBKGDESIG");

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

  static tse::SvcFeesResBkgDesigInfo* mapRowToSvcFeesResBkgDesigInfo(Row* row)
  {
    tse::SvcFeesResBkgDesigInfo* resBD = new tse::SvcFeesResBkgDesigInfo;

    resBD->vendor() = row->getString(VENDOR);
    resBD->itemNo() = row->getInt(ITEMNO);
    resBD->seqNo() = row->getLong(SEQNO);
    resBD->createDate() = row->getDate(CREATEDATE);
    if (!row->isNull(EXPIREDATE))
      resBD->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(MKGOPERIND))
      resBD->mkgOperInd() = row->getChar(MKGOPERIND);
    if (!row->isNull(CARRIER))
      resBD->carrier() = row->getString(CARRIER);
    if (!row->isNull(BOOKINGCODE1))
      resBD->bookingCode1() = row->getString(BOOKINGCODE1);
    if (!row->isNull(BOOKINGCODE2))
      resBD->bookingCode2() = row->getString(BOOKINGCODE2);
    if (!row->isNull(BOOKINGCODE3))
      resBD->bookingCode3() = row->getString(BOOKINGCODE3);
    if (!row->isNull(BOOKINGCODE4))
      resBD->bookingCode4() = row->getString(BOOKINGCODE4);
    if (!row->isNull(BOOKINGCODE5))
      resBD->bookingCode5() = row->getString(BOOKINGCODE5);

    return resBD;
  }; // mapRowToSvcFeesResBkgDesigInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetSvcFeesResBkgDesigSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSvcFeesResBkgDesigHistoricalSQLStatement
    : public QueryGetSvcFeesResBkgDesigSQLStatement<QUERYCLASS>
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
