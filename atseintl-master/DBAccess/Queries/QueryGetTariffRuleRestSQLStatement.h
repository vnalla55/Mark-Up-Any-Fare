//----------------------------------------------------------------------------
//          File:           QueryGetTariffRuleRestSQLStatement.h
//          Description:    QueryGetTariffRuleRestSQLStatement
//          Created:        10/5/2007
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
#include "DBAccess/Queries/QueryGetTariffRuleRest.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetTariffRuleRestSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTariffRuleRestSQLStatement() {}
  virtual ~QueryGetTariffRuleRestSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    CATEGORY,
    ORDERNO,
    RULETARIFF,
    PRIMERULEIND,
    DEFAULTRULEIND,
    TRFRULEAPPLIND,
    SAMETRFRULEIND,
    RULE,
    GLOBALDIR,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select "
                  "r.VENDOR,r.ITEMNO,r.CREATEDATE,m.EXPIREDATE,r.CATEGORY,ORDERNO,RULETARIFF,"
                  "PRIMERULEIND,"
                  "DEFAULTRULEIND,TRFRULEAPPLIND,SAMETRFRULEIND,RULE,GLOBALDIR");
    this->From("=TARIFFRULECOMBSEQ r join =MINORCOMBSUBCAT m"
               " on  r.VENDOR = m.VENDOR "
               " and r.ITEMNO = m.ITEMNO "
               " and r.CATEGORY = m.CATEGORY "
               " and r.CREATEDATE = m.CREATEDATE ");
    this->Where(" r.VENDOR = %1q"
                " and r.ITEMNO = %2n "
                " and m.VALIDITYIND = 'Y'"
                " and m.EXPIREDATE >= %cd ");

    if (DataManager::forceSortOrder())
      this->OrderBy("r.VENDOR, r.ITEMNO, r.CREATEDATE, r.CATEGORY, r.ORDERNO");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TariffRuleRest* mapRowToTariffRuleRest(Row* row)
  {
    tse::TariffRuleRest* tariffR = new tse::TariffRuleRest;

    tariffR->vendor() = row->getString(VENDOR);
    tariffR->itemNo() = row->getInt(ITEMNO);
    tariffR->createDate() = row->getDate(CREATEDATE);
    tariffR->expireDate() = row->getDate(EXPIREDATE);
    tariffR->orderNo() = row->getInt(ORDERNO);
    tariffR->ruleTariff() = row->getInt(RULETARIFF);
    tariffR->primeRuleInd() = row->getChar(PRIMERULEIND);
    tariffR->defaultRuleInd() = row->getChar(DEFAULTRULEIND);
    tariffR->trfRuleApplInd() = row->getChar(TRFRULEAPPLIND);
    tariffR->sametrfRuleInd() = row->getChar(SAMETRFRULEIND);
    tariffR->rule() = row->getString(RULE);
    std::string gd = row->getString(GLOBALDIR);
    strToGlobalDirection(tariffR->globalDirection(), gd);

    return tariffR;
  } // mapRowToTariffRuleRest()
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTariffRuleRestHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTariffRuleRestHistoricalSQLStatement
    : public QueryGetTariffRuleRestSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" r.VENDOR = %1q"
                " and r.ITEMNO = %2n "
                " and m.VALIDITYIND = 'Y'"
                " and %3n <= m.EXPIREDATE"
                " and %4n >= r.CREATEDATE");
  }
}; // class QueryGetTariffRuleRestHistoricalSQLStatement
}
