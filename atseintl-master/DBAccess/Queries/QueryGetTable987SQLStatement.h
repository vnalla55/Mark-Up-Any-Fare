//----------------------------------------------------------------------------
//          File:           QueryGetTable987SQLStatement.h
//          Description:    QueryGetTable987SQLStatement
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
#include "DBAccess/Queries/QueryGetTable987.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetTable987SQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTable987SQLStatement() {};
  virtual ~QueryGetTable987SQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    WAIVER,
    EXPIREDATE,
    LASTMODDATE,
    CREATORID,
    CREATORBUSINESSUNIT,
    INHIBIT,
    VALIDITYIND,
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR, ITEMNO, CREATEDATE, WAIVER, EXPIREDATE, LASTMODDATE,"
                  "       CREATORID, CREATORBUSINESSUNIT, INHIBIT, VALIDITYIND");
    this->From("=WAIVER ");
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, CREATEDATE, WAIVER");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::Waiver* mapRowToWaiver(Row* row)
  {
    tse::Waiver* table987 = new tse::Waiver;

    table987->vendor() = row->getString(VENDOR);
    table987->itemNo() = row->getInt(ITEMNO);
    table987->createDate() = row->getDate(CREATEDATE);
    table987->expireDate() = row->getDate(EXPIREDATE);
    table987->waiver() = row->getInt(WAIVER);
    table987->inhibit() = row->getChar(INHIBIT);
    table987->validityInd() = row->getChar(VALIDITYIND);
    table987->creatorId() = row->getString(CREATORID);
    table987->creatorBusinessUnit() = row->getString(CREATORBUSINESSUNIT);

    return table987;
  } // mapRowToWaiver()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTable987Historical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTable987HistoricalSQLStatement : public QueryGetTable987SQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'");
  }
}; // class QueryGetTable987HistoricalSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAllTable987Historical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAllTable987HistoricalSQLStatement : public QueryGetTable987SQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override { this->Where("VALIDITYIND = 'Y'"); }
}; // class QueryGetAllTable987HistoricalSQLStatement
} // tse
