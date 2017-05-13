//----------------------------------------------------------------------------
//          File:           QueryGetTable993SQLStatement.h
//          Description:    QueryGetTable993SQLStatement
//          Created:        11/02/2007
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
#include "DBAccess/Queries/QueryGetTable993.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetTable993SQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTable993SQLStatement() {};
  virtual ~QueryGetTable993SQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    INHIBIT,
    VALIDITYIND,
    MKT1,
    MKT2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,SEQNO,CREATEDATE,EXPIREDATE,INHIBIT,"
                  "       VALIDITYIND,MKT1,MKT2 ");
    this->From("=SAMEPOINTS ");
    this->Where("VENDOR = %1q "
                "    and ITEMNO = %2n "
                "    and %cd <= EXPIREDATE "
                "    and VALIDITYIND = 'Y'");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, SEQNO, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::SamePoint* mapRowToSamePoint(Row* row)
  {
    tse::SamePoint* table993 = new tse::SamePoint;

    table993->vendor() = row->getString(VENDOR);
    table993->itemNo() = row->getInt(ITEMNO);
    table993->seqNo() = row->getLong(SEQNO);
    table993->createDate() = row->getDate(CREATEDATE);
    table993->expireDate() = row->getDate(EXPIREDATE);
    table993->inhibit() = row->getChar(INHIBIT);
    table993->validityInd() = row->getChar(VALIDITYIND);
    table993->mkt1() = row->getString(MKT1);
    table993->mkt2() = row->getString(MKT2);

    return table993;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetTable993SQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTable993Historical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTable993HistoricalSQLStatement : public QueryGetTable993SQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q "
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  }
}; // class QueryGetTable993HistoricalSQLStatement
} // tse
