//----------------------------------------------------------------------------
//          File:           QueryGetGeoRuleSQLStatement.h
//          Description:    QueryGetGeoRuleSQLStatement
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
#include "DBAccess/Queries/QueryGetGeoRule.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetGeoRuleSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetGeoRuleSQLStatement() {};
  virtual ~QueryGetGeoRuleSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    TSI,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,TSI,LOC1TYPE,LOC1,"
                  " LOC2TYPE,LOC2,INHIBIT ");
    this->From("=GEOSPEC");
    this->Where("VENDOR = %1q "
                " and ITEMNO = %2n "
                " and VALIDITYIND = 'Y' "
                " and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::GeoRuleItem* mapRowToGeoRuleItem(Row* row)
  {
    tse::GeoRuleItem* geoRule = new tse::GeoRuleItem;

    geoRule->vendor() = row->getString(VENDOR);
    geoRule->itemNo() = row->getInt(ITEMNO);
    geoRule->createDate() = row->getDate(CREATEDATE);
    geoRule->expireDate() = row->getDate(EXPIREDATE);
    geoRule->tsi() = row->getInt(TSI);
    geoRule->loc1().locType() = row->getChar(LOC1TYPE);
    geoRule->loc1().loc() = row->getString(LOC1);
    geoRule->loc2().locType() = row->getChar(LOC2TYPE);
    geoRule->loc2().loc() = row->getString(LOC2);
    geoRule->inhibit() = row->getChar(INHIBIT);

    return geoRule;
  }; // mapRowToGeoRuleItem()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetGeoRuleSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetGeoRuleHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetGeoRuleHistoricalSQLStatement : public QueryGetGeoRuleSQLStatement<QUERYCLASS>
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
}; // class QueryGetGeoRuleHistoricalSQLStatement
} // tse
