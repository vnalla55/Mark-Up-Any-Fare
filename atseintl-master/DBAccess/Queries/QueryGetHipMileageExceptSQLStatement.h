//----------------------------------------------------------------------------
//          File:           QueryGetHipMileageExceptSQLStatement.h
//          Description:    QueryGetHipMileageExceptSQLStatement
//          Created:        3/2/2006
// Authors:         Mike Lillis
//
//          Updates:
//
//     (C) 2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetHipMileageExcept.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetHipMileageExceptSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetHipMileageExceptSQLStatement() {};
  virtual ~QueryGetHipMileageExceptSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    CONNECTSTOPIND,
    NOHIPIND,
    UNAVAILTAG,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,TEXTTBLITEMNO,"
                  " OVERRIDEDATETBLITEMNO,LOC1TYPE,LOC1,LOC2TYPE,LOC2,"
                  " CONNECTSTOPIND,NOHIPIND,UNAVAILTAG,INHIBIT");
    this->From("=HIPMILEAGEEXCEPT");
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR , ITEMNO , CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::HipMileageExceptInfo* mapRowToHipMileageExceptInfo(Row* row)
  {
    tse::HipMileageExceptInfo* hme = new tse::HipMileageExceptInfo;

    hme->vendor() = row->getString(VENDOR);
    hme->itemNo() = row->getInt(ITEMNO);
    hme->createDate() = row->getDate(CREATEDATE);
    hme->expireDate() = row->getDate(EXPIREDATE);
    hme->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    hme->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);

    LocKey* loc = &hme->loc1();
    loc->locType() = row->getChar(LOC1TYPE);
    loc->loc() = row->getString(LOC1);

    loc = &hme->loc2();
    loc->locType() = row->getChar(LOC2TYPE);
    loc->loc() = row->getString(LOC2);

    hme->connectStopInd() = row->getChar(CONNECTSTOPIND);
    hme->noHipInd() = row->getChar(NOHIPIND);
    hme->unavailTag() = row->getChar(UNAVAILTAG);
    hme->inhibit() = row->getChar(INHIBIT);

    return hme;
  } // mapRowToHipMileageExceptInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetHipMileageExceptSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetHipMileageExceptHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetHipMileageExceptHistoricalSQLStatement
    : public QueryGetHipMileageExceptSQLStatement<QUERYCLASS>
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
}; // class QueryGetHipMileageExceptHistoricalSQLStatement
} // tse
