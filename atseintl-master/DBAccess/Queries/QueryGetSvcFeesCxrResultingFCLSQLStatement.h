//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesCxrResultingFCLSQLStatement.h
//          Description:    QueryGetSvcFeesCxrResultingFCLSQLStatement
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
#include "DBAccess/Queries/QueryGetSvcFeesCxrResultingFCL.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetSvcFeesCxrResultingFCLSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSvcFeesCxrResultingFCLSQLStatement() {};
  virtual ~QueryGetSvcFeesCxrResultingFCLSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    VALIDITYIND,
    CARRIER,
    RESULTINGFCL,
    FARETYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, ITEMNO, SEQNO,  "
                  "       CREATEDATE, EXPIREDATE, VALIDITYIND, "
                  "       CARRIER, RESULTINGFCL, FARETYPE");

    this->From("=SVCFEESCXRRESULTINGFCL");

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

  static tse::SvcFeesCxrResultingFCLInfo* mapRowToSvcFeesCxrResultingFCLInfo(Row* row)
  {
    tse::SvcFeesCxrResultingFCLInfo* cxrC = new tse::SvcFeesCxrResultingFCLInfo;

    cxrC->vendor() = row->getString(VENDOR);
    cxrC->itemNo() = row->getInt(ITEMNO);
    cxrC->seqNo() = row->getLong(SEQNO);
    cxrC->createDate() = row->getDate(CREATEDATE);
    if (!row->isNull(EXPIREDATE))
      cxrC->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(CARRIER))
      cxrC->carrier() = row->getString(CARRIER);
    if (!row->isNull(RESULTINGFCL))
      cxrC->resultingFCL() = row->getString(RESULTINGFCL);
    if (!row->isNull(FARETYPE))
      cxrC->fareType() = row->getString(FARETYPE);

    return cxrC;
  }; // mapRowToSvcFeesCxrResultingFCLInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetSvcFeesCxrResultingFCLSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSvcFeesCxrResultingFCLHistoricalSQLStatement
    : public QueryGetSvcFeesCxrResultingFCLSQLStatement<QUERYCLASS>
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
