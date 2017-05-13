//----------------------------------------------------------------------------
//          File:           QueryGetBSRSQLStatement.h
//          Description:    QueryGetBSRSQLStatement
//          Created:        10/29/2007
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
#include "DBAccess/Queries/QueryGetBSR.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetBSRSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetBSRSQLStatement() {};
  virtual ~QueryGetBSRSQLStatement() {};

  enum ColumnIndexes
  {
    PRIMECUR = 0,
    CUR,
    RATE,
    MEMONO,
    RATENODEC,
    EFFDATE,
    DISCDATE,
    CREATEDATE,
    EXPIREDATE,
    RATETYPE,
    AGENTSINE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select PRIMECUR,CUR,RATE,MEMONO,RATENODEC,EFFDATE,DISCDATE,"
                  "       CREATEDATE, EXPIREDATE,RATETYPE,AGENTSINE");
    this->From("=BANKERSSELLINGRATE");
    this->Where("PRIMECUR = %1q"
                " and %2n <= EXPIREDATE");
    this->OrderBy("PRIMECUR, CUR, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::BankerSellRate* mapRowToBSR(Row* row)
  {
    tse::BankerSellRate* BSR = new tse::BankerSellRate;

    BSR->primeCur() = row->getString(PRIMECUR);
    BSR->cur() = row->getString(CUR);
    BSR->effDate() = row->getDate(EFFDATE);
    BSR->discDate() = row->getDate(DISCDATE);
    BSR->createDate() = row->getDate(CREATEDATE);
    BSR->expireDate() = row->getDate(EXPIREDATE);
    BSR->rateNodec() = row->getInt(RATENODEC);
    BSR->rate() = QUERYCLASS::adjustDecimal(row->getLong(RATE), BSR->rateNodec());
    BSR->rateType() = row->getChar(RATETYPE);
    BSR->agentSine() = row->getString(AGENTSINE);

    return BSR;
  } // mapRowToBSR()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetBSRHistorical
////////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetBSRHistoricalSQLStatement : public QueryGetBSRSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select PRIMECUR,CUR,RATE,MEMONO,RATENODEC,EFFDATE,DISCDATE,"
                             "       CREATEDATE, EXPIREDATE,RATETYPE,AGENTSINE");
    partialStatement.From("=BANKERSSELLINGRATEH");
    partialStatement.Where("PRIMECUR = %1q"
                           " and %2n <= EXPIREDATE"
                           " and (   %3n >= CREATEDATE"
                           "      or %4n >= EFFDATE)"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select PRIMECUR,CUR,RATE,MEMONO,RATENODEC,EFFDATE,DISCDATE,"
                             "       CREATEDATE, EXPIREDATE,RATETYPE,AGENTSINE");
    partialStatement.From("=BANKERSSELLINGRATE");
    partialStatement.Where("PRIMECUR = %5q"
                           " and %6n <= EXPIREDATE"
                           " and (   %7n >= CREATEDATE"
                           "      or %8n >= EFFDATE)"
                           ")");

    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

} // tse
