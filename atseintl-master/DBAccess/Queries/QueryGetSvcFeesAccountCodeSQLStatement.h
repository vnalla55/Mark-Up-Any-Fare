//----------------------------------------------------------------------------
//          File:           QueryGetSvcFeesAccountCodeSQLStatement.h
//          Description:    QueryGetSvcFeesAccountCodeSQLStatement
//          Created:        3/6/2009
// Authors:
//
//          Updates:
//
//      2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetSvcFeesAccountCode.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetSvcFeesAccountCodeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetSvcFeesAccountCodeSQLStatement() {};
  virtual ~QueryGetSvcFeesAccountCodeSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    VALIDITYIND,
    ACCOUNTCODE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, ITEMNO, SEQNO,  "
                  "       CREATEDATE, EXPIREDATE, VALIDITYIND, ACCOUNTCODE");

    this->From("=SVCFEESACCOUNTCODE");

    this->Where("VENDOR      = %1q  "
                " and ITEMNO = %2n "
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE ");
    if (DataManager::forceSortOrder())
      this->OrderBy("CREATEDATE, VENDOR, ITEMNO, SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  } // RegisterColumnsAndBaseSQL()

  static tse::SvcFeesAccCodeInfo* mapRowToSvcFeesAccountCodeInfo(Row* row)
  {
    tse::SvcFeesAccCodeInfo* accC = new tse::SvcFeesAccCodeInfo;

    accC->vendor() = row->getString(VENDOR);
    accC->itemNo() = row->getInt(ITEMNO);
    accC->seqNo() = row->getLong(SEQNO);
    accC->createDate() = row->getDate(CREATEDATE);
    if (!row->isNull(EXPIREDATE))
      accC->expireDate() = row->getDate(EXPIREDATE);
    if (!row->isNull(ACCOUNTCODE))
      accC->accountCode() = row->getString(ACCOUNTCODE);

    return accC;
  }; // mapRowToSvcFeesAccountCodeInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetSvcFeesAccountCodeSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetSvcFeesAccountCodeHistoricalSQLStatement
    : public QueryGetSvcFeesAccountCodeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q  "
                " and ITEMNO= %2n "
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  };
};
}
