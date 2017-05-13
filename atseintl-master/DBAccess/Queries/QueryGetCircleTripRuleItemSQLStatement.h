//----------------------------------------------------------------------------
//          File:           QueryGetCircleTripRuleItemSQLStatement.h
//          Description:    QueryGetCircleTripRuleItemSQLStatement
//          Created:        10/30/2007
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
#include "DBAccess/Queries/QueryGetCircleTripRuleItem.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetCircleTripRuleItemSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetCircleTripRuleItemSQLStatement() {};
  virtual ~QueryGetCircleTripRuleItemSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    INHIBIT,
    UNAVAILTAG,
    SAMECARRIERIND,
    ATWIND,
    BREAKPOINTS,
    STOPOVERCNT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,TEXTTBLITEMNO, "
                  "       OVERRIDEDATETBLITEMNO,INHIBIT,UNAVAILTAG,"
                  "       SAMECARRIERIND,ATWIND,BREAKPOINTS,STOPOVERCNT");
    this->From("=CT2PLUSCOMP ");
    this->Where("VENDOR = %1q "
                "    and ITEMNO = %2n "
                "    and %cd <= EXPIREDATE "
                "    and VALIDITYIND = 'Y'");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::CircleTripRuleItem* mapRowToCircleTripRuleItem(Row* row)
  {
    tse::CircleTripRuleItem* circle = new tse::CircleTripRuleItem;

    circle->vendor() = row->getString(VENDOR);
    circle->itemNo() = row->getInt(ITEMNO);
    circle->expireDate() = row->getDate(EXPIREDATE);
    circle->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    circle->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    circle->unavailtag() = row->getChar(UNAVAILTAG);
    circle->sameCarrierInd() = row->getChar(SAMECARRIERIND);
    circle->atwInd() = row->getChar(ATWIND);
    circle->breakPoints() = row->getChar(BREAKPOINTS);
    circle->stopoverCnt() = row->getString(STOPOVERCNT);
    circle->inhibit() = row->getChar(INHIBIT);

    return circle;
  } // mapRowToCircleTripRuleItem()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetCircleTripRuleItemSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetCircleTripRuleItemHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetCircleTripRuleItemHistoricalSQLStatement
    : public QueryGetCircleTripRuleItemSQLStatement<QUERYCLASS>
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
}; // class QueryGetCircleTripRuleItemHistoricalSQLStatement
} // tse
