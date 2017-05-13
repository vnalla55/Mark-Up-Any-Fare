//----------------------------------------------------------------------------
//          File:           QueryGetGeneralRuleSQLStatement.h
//          Description:    QueryGetGeneralRuleSQLStatement
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

#include "Common/Logger.h"
#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetGeneralRule.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetGeneralRuleSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetGeneralRuleSQLStatement() {};
  virtual ~QueryGetGeneralRuleSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    RULETARIFF,
    RULE,
    CREATEDATE,
    EXPIREDATE,
    EFFDATE,
    DISCDATE,
    ORDERNO,
    GENERALRULE,
    GENERALRULETARIFF,
    CATEGORY,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select gr.VENDOR,gr.CARRIER,gr.RULETARIFF,gr.RULE,gr.CREATEDATE,gr.EXPIREDATE,"
                  "gr.EFFDATE,gr.DISCDATE,ORDERNO,GENERALRULE,GENERALRULETARIFF,CATEGORY");
    this->From("=GENERALRULE gr, =GENERALRULESEG grs");
    this->Where("gr.vendor = %1q  "
                " and gr.carrier= %2q "
                " and gr.RULETARIFF= %3n "
                " and gr.rule= %4q "
                " and gr.vendor = grs.vendor "
                " and gr.carrier = grs.carrier "
                " and gr.rule = grs.rule "
                " and gr.ruletariff = grs.ruletariff "
                " and gr.createdate = grs.createdate"
                " and %cd <= EXPIREDATE ");

    if (DataManager::forceSortOrder())
      this->OrderBy("gr.VENDOR ,gr.CARRIER, gr.RULETARIFF, gr.RULE, gr.CREATEDATE, grs.ORDERNO");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }; // RegisterColumnsAndBaseSQL()

  static tse::GeneralRuleApp* mapRowToGeneralRuleApp(Row* row)
  {
    tse::GeneralRuleApp* gen = new tse::GeneralRuleApp;

    gen->createDate() = row->getDate(CREATEDATE);
    gen->expireDate() = row->getDate(EXPIREDATE);
    gen->vendor() = row->getString(VENDOR);
    gen->carrier() = row->getString(CARRIER);
    gen->ruleTariff() = row->getInt(RULETARIFF);
    gen->effDate() = row->getDate(EFFDATE);
    gen->discDate() = row->getDate(DISCDATE);
    gen->rule() = row->getString(RULE);
    gen->orderNo() = row->getInt(ORDERNO);
    gen->generalRule() = row->getString(GENERALRULE);
    gen->generalRuleTariff() = row->getInt(GENERALRULETARIFF);
    gen->category() = row->getInt(CATEGORY);

    return gen;
  }; // mapRowToGeneralRuleApp()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetGeneralRuleSQLStatement

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetGeneralRuleHistoricalSQLStatement : public QueryGetGeneralRuleSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("gr.vendor = %1q  "
                " and gr.carrier= %2q "
                " and gr.RULETARIFF= %3n "
                " and gr.rule= %4q "
                " and gr.vendor = grs.vendor "
                " and gr.carrier = grs.carrier "
                " and gr.rule = grs.rule "
                " and gr.ruletariff = grs.ruletariff "
                " and gr.createdate = grs.createdate"
                " and %5n <= gr.EXPIREDATE"
                " and %6n >= gr.CREATEDATE");

    if (DataManager::forceSortOrder())
    {
      this->OrderBy("gr.CREATEDATE,ORDERNO");
    }
    else
    {
      this->OrderBy("gr.CREATEDATE");
    }
  };
};
}
