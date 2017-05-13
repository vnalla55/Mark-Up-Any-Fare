//----------------------------------------------------------------------------
//          File:           QueryGetDateOverrideRuleSQLStatement.h
//          Description:    QueryGetDateOverrideRuleSQLStatement
//          Created:        11/01/2007
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
#include "DBAccess/Queries/QueryGetDateOverrideRule.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetDateOverrideRuleSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetDateOverrideRuleSQLStatement() {};
  virtual ~QueryGetDateOverrideRuleSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    EXPIREDATE,
    TVLEFFDATE,
    TVLDISCDATE,
    TKTEFFDATE,
    TKTDISCDATE,
    RESEFFDATE,
    RESDISCDATE,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,EXPIREDATE,TVLEFFDATE,TVLDISCDATE,"
                  "       TKTEFFDATE,TKTDISCDATE,RESEFFDATE,RESDISCDATE,INHIBIT");
    this->From("=OVERRIDEDATE  ");
    this->Where("VENDOR = %1q "
                "    and ITEMNO = %2n "
                "    and VALIDITYIND = 'Y' "
                "    and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::DateOverrideRuleItem* mapRowToDateOverrideRuleItem(Row* row)
  {
    tse::DateOverrideRuleItem* dor = new tse::DateOverrideRuleItem;

    dor->vendor() = row->getString(VENDOR);
    dor->itemNo() = row->getInt(ITEMNO);
    dor->expireDate() = row->getDate(EXPIREDATE);
    dor->tvlEffDate() = row->getDate(TVLEFFDATE);

    dor->tvlDiscDate() = row->getDate(TVLDISCDATE);
    if (dor->tvlDiscDate().isNegInfinity() || dor->tvlDiscDate().isEmptyDate())
      dor->tvlDiscDate() = boost::date_time::pos_infin;

    dor->tktEffDate() = row->getDate(TKTEFFDATE);

    dor->tktDiscDate() = row->getDate(TKTDISCDATE);
    if (dor->tktDiscDate().isNegInfinity() || dor->tktDiscDate().isEmptyDate())
      dor->tktDiscDate() = boost::date_time::pos_infin;

    dor->resEffDate() = row->getDate(RESEFFDATE);

    dor->resDiscDate() = row->getDate(RESDISCDATE);
    if (dor->resDiscDate().isNegInfinity() || dor->resDiscDate().isEmptyDate())
      dor->resDiscDate() = boost::date_time::pos_infin;

    dor->inhibit() = row->getChar(INHIBIT);

    return dor;
  } // mapRowToDateOverrideRuleItem()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetDateOverrideRuleSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetDateOverrideRuleHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetDateOverrideRuleHistoricalSQLStatement
    : public QueryGetDateOverrideRuleSQLStatement<QUERYCLASS>
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
}; // class QueryGetDateOverrideRuleHistoricalSQLStatement
} // tse
