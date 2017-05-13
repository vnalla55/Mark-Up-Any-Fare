//----------------------------------------------------------------------------
//          File:           QueryGetAddonMarketCarriersSQLStatement.h
//          Description:    QueryGetAddonMarketCarriersSQLStatement
//          Created:        10/25/2007
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
#include "DBAccess/Queries/QueryGetAddonMarketCarriers.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetAddonMarketCarriersSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetAddonMarketCarriersSQLStatement() {};
  virtual ~QueryGetAddonMarketCarriersSQLStatement() {};

  enum ColumnIndexes
  {
    GATEWAYMARKET = 0,
    INTERIORMARKET,
    CARRIER,
    CREATEDATE,
    EFFDATE,
    EXPIREDATE,
    DISCDATE,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select GATEWAYMARKET, INTERIORMARKET, CARRIER,"
                  " min(CREATEDATE) CREATEDATE, min(EFFDATE) EFFDATE,"
                  " max(EXPIREDATE) EXPIREDATE, max(DISCDATE) DISCDATE, "
                  " INHIBIT ");
    this->From("=ADDONFARE");
    this->Where("GATEWAYMARKET = %1q"
                "  and INTERIORMARKET = %2q"
                "  and VALIDITYIND = 'Y'"
                "  and %cd <= DISCDATE"
                "  and %cd <= EXPIREDATE"
                "  and EFFDATE <= DISCDATE"
                " group by GATEWAYMARKET, INTERIORMARKET, CARRIER, INHIBIT");
    //        this->GroupBy("GATEWAYMARKET, INTERIORMARKET, CARRIER");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MarketCarrier* mapRowToAddOnMarketCarrier(Row* row)
  {
    tse::MarketCarrier* mc = new tse::MarketCarrier;

    mc->market1() = row->getString(GATEWAYMARKET);
    mc->market2() = row->getString(INTERIORMARKET);
    mc->carrier() = row->getString(CARRIER);
    mc->effDate() = row->getDate(EFFDATE);
    mc->discDate() = row->getDate(DISCDATE);
    mc->createDate() = row->getDate(CREATEDATE);
    mc->expireDate() = row->getDate(EXPIREDATE);
    mc->inhibit() = row->getChar(INHIBIT);

    return mc;
  } // mapRowToAddOnMarketCarrier()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetAddonMarketCarriersHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetAddonMarketCarriersHistoricalSQLStatement
    : public QueryGetAddonMarketCarriersSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command("("
                             "select GATEWAYMARKET, INTERIORMARKET, CARRIER,"
                             " min(CREATEDATE) CREATEDATE, min(EFFDATE) EFFDATE,"
                             " max(EXPIREDATE) EXPIREDATE, max(DISCDATE) DISCDATE, "
                             " INHIBIT ");
    partialStatement.From("=ADDONFAREH");
    partialStatement.Where("GATEWAYMARKET = %1q"
                           "  and INTERIORMARKET = %2q"
                           "  and VALIDITYIND = 'Y'"
                           "  and %3n <= EXPIREDATE"
                           "  and (   %4n >=  CREATEDATE"
                           "       or %5n >= EFFDATE)"
                           " group by GATEWAYMARKET, INTERIORMARKET, CARRIER, INHIBIT"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(" union all"
                             " ("
                             "select GATEWAYMARKET, INTERIORMARKET, CARRIER,"
                             " min(CREATEDATE) CREATEDATE, min(EFFDATE) EFFDATE,"
                             " max(EXPIREDATE) EXPIREDATE, max(DISCDATE) DISCDATE, "
                             " INHIBIT ");
    partialStatement.From("=ADDONFARE");
    partialStatement.Where("GATEWAYMARKET = %6q"
                           "  and INTERIORMARKET = %7q"
                           "  and VALIDITYIND = 'Y'"
                           "  and %8n <= EXPIREDATE"
                           "  and (   %9n >=  CREATEDATE"
                           "       or %10n >= EFFDATE)"
                           " group by GATEWAYMARKET, INTERIORMARKET, CARRIER, INHIBIT"
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
}; // class QueryGetAddonMarketCarriersHistoricalSQLStatement
} // tse
