//----------------------------------------------------------------------------
//          File:           QueryGetRoundTripRuleSQLStatement.h
//          Description:    QueryGetRoundTripRuleSQLStatement
//          Created:        10/8/2007
//          Authors:         Mike Lillis
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
#include "DBAccess/Queries/QueryGetRoundTripRule.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetRoundTripRuleSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetRoundTripRuleSQLStatement() {}
  virtual ~QueryGetRoundTripRuleSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    TEXTTBLITEMNO,
    UNAVAILTAG,
    OVERRIDEDATETBLITEMNO,
    SAMECARRIERIND,
    ATWIND,
    APPLIND,
    INHIBIT,
    HIGHRTIND,
    NUMBEROFCOLUMNS
  }; // enum
  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,TEXTTBLITEMNO,UNAVAILTAG,"
                  "       OVERRIDEDATETBLITEMNO,SAMECARRIERIND,ATWIND,APPLIND,INHIBIT,HIGHRTIND");
    this->From("=CT2COMP ");
    this->Where(" VENDOR = %1q"
                "    and ITEMNO = %2n"
                "    and %cd <= EXPIREDATE"
                "    and VALIDITYIND = 'Y'");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::RoundTripRuleItem* mapRowToRoundTripRuleItem(Row* row)
  {
    tse::RoundTripRuleItem* round = new tse::RoundTripRuleItem;

    round->vendor() = row->getString(VENDOR);
    round->itemNo() = row->getInt(ITEMNO);
    round->expireDate() = row->getDate(EXPIREDATE);
    round->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    round->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    round->unavailTag() = row->getChar(UNAVAILTAG);
    round->sameCarrierInd() = row->getChar(SAMECARRIERIND);
    round->atwInd() = row->getChar(ATWIND);
    round->applInd() = row->getChar(APPLIND);
    round->inhibit() = row->getChar(INHIBIT);
    round->highrtInd() = row->getChar(HIGHRTIND);

    return round;
  } // mapRowToRoundTripRuleItem()
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetRoundTripRuleSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetRoundTripRuleHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetRoundTripRuleHistoricalSQLStatement
    : public QueryGetRoundTripRuleSQLStatement<QUERYCLASS>
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
}; // class QueryGetRoundTripRuleHistoricalSQLStatement
}

