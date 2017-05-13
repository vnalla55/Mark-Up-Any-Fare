//----------------------------------------------------------------------------
//          File:           QueryGetDepositsSQLStatement.h
//          Description:    QueryGetDepositsSQLStatement
//          Created:        11/01/2007
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
#include "DBAccess/Queries/QueryGetDeposits.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetDepositsSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    INHIBIT,
    OVERRIDEDATETBLITEMNO,
    TEXTTBLITEMNO,
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,INHIBIT,"
                  "       OVERRIDEDATETBLITEMNO,TEXTTBLITEMNO");
    this->From("=DEPOSITS ");
    this->Where("VENDOR = %1q "
                "    and ITEMNO = %2n "
                "    and EXPIREDATE >= %cd "
                "    and VALIDITYIND = 'Y' ");
    this->OrderBy("VENDOR,ITEMNO,CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::Deposits* mapRowToDeposits(Row* row)
  {
    Deposits* rec = new Deposits();

    rec->vendor() = row->getString(VENDOR);
    rec->itemNo() = row->getInt(ITEMNO);
    rec->createDate() = row->getDate(CREATEDATE);
    rec->expireDate() = row->getDate(EXPIREDATE);
    rec->inhibit() = row->getChar(INHIBIT);
    rec->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    rec->textTblItemNo() = row->getInt(TEXTTBLITEMNO);

    return rec;
  } // mapRowToDeposits()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetDepositsSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetDepositsHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetDepositsHistoricalSQLStatement : public QueryGetDepositsSQLStatement<QUERYCLASS>
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
}; // class QueryGetDepositsHistoricalSQLStatement
} // tse
