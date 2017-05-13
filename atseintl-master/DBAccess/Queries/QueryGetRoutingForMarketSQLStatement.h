//----------------------------------------------------------------------------
//          File:           QueryGetRoutingForMarketSQLStatement.h
//          Description:    QueryGetRoutingForMarketSQLStatement
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
#include "DBAccess/Queries/QueryGetRoutingForMarket.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;
template <class QUERYCLASS>
class QueryGetRoutingForDomMarketSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetRoutingForDomMarketSQLStatement() {}
  virtual ~QueryGetRoutingForDomMarketSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    ROUTINGTARIFF,
    ROUTING,
    ROUTINGTARIFFCODE,
    CREATEDATE,
    EXPIREDATE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    //        this->Command("select distinct"
    //                      " r.VENDOR,r.CARRIER,r.ROUTINGTARIFF,r.ROUTING,"
    //                      " if (r.ROUTINGTARIFF = x.ROUTINGTARIFF1,"
    //                      "     x.ROUTINGTARIFF1CODE,x.ROUTINGTARIFF2CODE) ROUTINGTARIFFCODE,"
    //                      " r.CREATEDATE,r.EXPIREDATE");
    this->Command("select distinct"
                  " r.VENDOR,r.CARRIER,r.ROUTINGTARIFF,r.ROUTING,"
                  " (CASE when r.ROUTINGTARIFF = x.ROUTINGTARIFF1 then"
                  "    x.ROUTINGTARIFF1CODE else x.ROUTINGTARIFF2CODE end) ROUTINGTARIFFCODE,"
                  " r.CREATEDATE,r.EXPIREDATE");
    this->From("=DOMESTICFARE f, =TARIFFCROSSREF x, =ROUTING r ");
    this->Where(" f.MARKET1 = %1q"
                " and f.MARKET2 = %2q"
                " and f.CARRIER = %3q"
                " and x.VENDOR = f.VENDOR"
                " and x.TARIFFCROSSREFTYPE = 'D'"
                " and x.CARRIER = f.CARRIER"
                " and x.FARETARIFF = f.FARETARIFF"
                " and r.VENDOR = f.VENDOR"
                " and r.CARRIER = f.CARRIER"
                " and r.ROUTINGTARIFF in (x.ROUTINGTARIFF1,x.ROUTINGTARIFF2)"
                " and r.ROUTING = f.ROUTING");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,CARRIER,ROUTINGTARIFF,ROUTING,ROUTINGTARIFFCODE,CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static RoutingKeyInfo* mapRowToRoutingKeyInfo(Row* row)
  {
    tse::RoutingKeyInfo* routingKey = new tse::RoutingKeyInfo;

    routingKey->vendor() = row->getString(VENDOR);
    routingKey->carrier() = row->getString(CARRIER);
    routingKey->routingTariff() = row->getInt(ROUTINGTARIFF);
    routingKey->routing() = row->getString(ROUTING);
    routingKey->routingCode() = row->getString(ROUTINGTARIFFCODE);
    routingKey->createDate() = row->getDate(CREATEDATE);
    routingKey->expireDate() = row->getDate(EXPIREDATE);

    return routingKey;
  } // mapRowToRoutingKeyInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetRoutingForDomMarketHistoricalSQLStatement
    : public QueryGetRoutingForDomMarketSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    //        partialStatement.Command("("
    //                                   "select distinct"
    //                                   " rh.VENDOR,rh.CARRIER,rh.ROUTINGTARIFF,rh.ROUTING,"
    //                                   " if (rh.ROUTINGTARIFF = xh.ROUTINGTARIFF1,"
    //                                   "     xh.ROUTINGTARIFF1CODE,xh.ROUTINGTARIFF2CODE)
    // ROUTINGTARIFFCODE,"
    //                                   " rh.CREATEDATE,rh.EXPIREDATE");

    partialStatement.Command(
        "("
        "select distinct"
        " rh.VENDOR,rh.CARRIER,rh.ROUTINGTARIFF,rh.ROUTING,"
        " (CASE when rh.ROUTINGTARIFF = xh.ROUTINGTARIFF1 then"
        "    xh.ROUTINGTARIFF1CODE else xh.ROUTINGTARIFF2CODE end) ROUTINGTARIFFCODE,"
        " rh.CREATEDATE,rh.EXPIREDATE");

    partialStatement.From("=DOMESTICFAREH fh, =TARIFFCROSSREF xh, =ROUTINGH rh");
    partialStatement.Where("fh.MARKET1 = %1q"
                           " and fh.MARKET2 = %2q"
                           " and fh.CARRIER = %3q"
                           " and %4n <= fh.EXPIREDATE"
                           " and %5n >= fh.CREATEDATE"
                           " and %6n <= xh.EXPIREDATE"
                           " and (   %7n >=  xh.CREATEDATE"
                           "      or %8n >= xh.EFFDATE)"
                           " and %9n <= rh.EXPIREDATE"
                           " and (   %10n >=  rh.CREATEDATE"
                           "      or %11n >= rh.EFFDATE)"
                           " and xh.VENDOR = fh.VENDOR"
                           " and xh.TARIFFCROSSREFTYPE = 'D'"
                           " and xh.CARRIER = fh.CARRIER"
                           " and xh.FARETARIFF = fh.FARETARIFF"
                           " and rh.VENDOR = fh.VENDOR"
                           " and rh.CARRIER = fh.CARRIER"
                           " and rh.ROUTINGTARIFF in (xh.ROUTINGTARIFF1,xh.ROUTINGTARIFF2)"
                           " and rh.ROUTING = fh.ROUTING"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    //        partialStatement.Command(" union all"
    //                                 " ("
    //                                    "select distinct"
    //                                    " r.VENDOR,r.CARRIER,r.ROUTINGTARIFF,r.ROUTING,"
    //                                    " if (r.ROUTINGTARIFF = x.ROUTINGTARIFF1,"
    //                                    "     x.ROUTINGTARIFF1CODE,x.ROUTINGTARIFF2CODE)
    // ROUTINGTARIFFCODE,"
    //                                    " r.CREATEDATE,r.EXPIREDATE");
    partialStatement.Command(
        " union all"
        " ("
        "select distinct"
        " r.VENDOR,r.CARRIER,r.ROUTINGTARIFF,r.ROUTING,"
        " (CASE when r.ROUTINGTARIFF = x.ROUTINGTARIFF1 then"
        "    x.ROUTINGTARIFF1CODE else x.ROUTINGTARIFF2CODE end) ROUTINGTARIFFCODE,"
        " r.CREATEDATE,r.EXPIREDATE");
    partialStatement.From("=DOMESTICFARE f, =TARIFFCROSSREF x, =ROUTING r ");
    partialStatement.Where("f.MARKET1 = %12q"
                           " and f.MARKET2 = %13q"
                           " and f.CARRIER = %14q"
                           " and %15n <= f.EXPIREDATE"
                           " and %16n >= f.CREATEDATE"
                           " and %17n <= x.EXPIREDATE"
                           " and (   %18n >=  x.CREATEDATE"
                           "      or %19n >= x.EFFDATE)"
                           " and %20n <= r.EXPIREDATE"
                           " and (   %21n >=  r.CREATEDATE"
                           "      or %22n >= r.EFFDATE)"
                           " and x.VENDOR = f.VENDOR"
                           " and x.TARIFFCROSSREFTYPE = 'D'"
                           " and x.CARRIER = f.CARRIER"
                           " and x.FARETARIFF = f.FARETARIFF"
                           " and r.VENDOR = f.VENDOR"
                           " and r.CARRIER = f.CARRIER"
                           " and r.ROUTINGTARIFF in (x.ROUTINGTARIFF1,x.ROUTINGTARIFF2)"
                           " and r.ROUTING = f.ROUTING"
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

template <class QUERYCLASS>
class QueryGetRoutingForIntlMarketSQLStatement
    : public QueryGetRoutingForDomMarketSQLStatement<QUERYCLASS>
{
private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  void adjustBaseSQL() override
  {
    this->From("=INTERNATIONALFARE f, =TARIFFCROSSREF x, =ROUTING r ");
    this->Where("f.MARKET1 = %1q and f.MARKET2 = %2q and f.CARRIER = %3q "
                "    and x.VENDOR = f.VENDOR"
                "    and x.TARIFFCROSSREFTYPE = 'I'"
                "    and x.CARRIER = f.CARRIER"
                "    and x.FARETARIFF = f.FARETARIFF"
                "    and r.VENDOR = f.VENDOR"
                "    and r.CARRIER = f.CARRIER"
                "    and r.ROUTINGTARIFF in (x.ROUTINGTARIFF1,x.ROUTINGTARIFF2)"
                "    and r.ROUTING = f.ROUTING");
  }
};

template <class QUERYCLASS>
class QueryGetRoutingForIntlMarketHistoricalSQLStatement
    : public QueryGetRoutingForDomMarketSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    //        partialStatement.Command("("
    //                                   "select distinct"
    //                                    " rh.VENDOR,rh.CARRIER,rh.ROUTINGTARIFF,rh.ROUTING,"
    //                                    " if (rh.ROUTINGTARIFF = xh.ROUTINGTARIFF1,"
    //                                    "     xh.ROUTINGTARIFF1CODE,xh.ROUTINGTARIFF2CODE)
    // ROUTINGTARIFFCODE,"
    //                                    " rh.CREATEDATE,rh.EXPIREDATE");
    partialStatement.Command(
        "("
        "select distinct"
        " rh.VENDOR,rh.CARRIER,rh.ROUTINGTARIFF,rh.ROUTING,"
        " (CASE when rh.ROUTINGTARIFF = xh.ROUTINGTARIFF1 then"
        "    xh.ROUTINGTARIFF1CODE else xh.ROUTINGTARIFF2CODE end) ROUTINGTARIFFCODE,"
        " rh.CREATEDATE,rh.EXPIREDATE");
    partialStatement.From("=INTERNATIONALFAREH fh, =TARIFFCROSSREF xh, =ROUTINGH rh");
    partialStatement.Where("fh.MARKET1 = %1q"
                           " and fh.MARKET2 = %2q"
                           " and fh.CARRIER = %3q"
                           " and %4n <= fh.EXPIREDATE"
                           " and %5n >= fh.CREATEDATE"
                           " and %6n <= xh.EXPIREDATE"
                           " and (   %7n >=  xh.CREATEDATE"
                           "      or %8n >= xh.EFFDATE)"
                           " and %9n <= rh.EXPIREDATE"
                           " and (   %10n >=  rh.CREATEDATE"
                           "      or %11n >= rh.EFFDATE)"
                           " and xh.VENDOR = fh.VENDOR"
                           " and xh.TARIFFCROSSREFTYPE = 'D'"
                           " and xh.CARRIER = fh.CARRIER"
                           " and xh.FARETARIFF = fh.FARETARIFF"
                           " and rh.VENDOR = fh.VENDOR"
                           " and rh.CARRIER = fh.CARRIER"
                           " and rh.ROUTINGTARIFF in (xh.ROUTINGTARIFF1,xh.ROUTINGTARIFF2)"
                           " and rh.ROUTING = fh.ROUTING"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    //        partialStatement.Command(" union all"
    //                                 " ("
    //                                    "select distinct"
    //                                    " r.VENDOR,r.CARRIER,r.ROUTINGTARIFF,r.ROUTING,"
    //                                    " if (r.ROUTINGTARIFF = x.ROUTINGTARIFF1,"
    //                                    "     x.ROUTINGTARIFF1CODE,x.ROUTINGTARIFF2CODE)
    // ROUTINGTARIFFCODE,"
    //                                    " r.CREATEDATE,r.EXPIREDATE");
    partialStatement.Command(
        " union all"
        " ("
        "select distinct"
        " r.VENDOR,r.CARRIER,r.ROUTINGTARIFF,r.ROUTING,"
        " (CASE when r.ROUTINGTARIFF = x.ROUTINGTARIFF1 then"
        "    x.ROUTINGTARIFF1CODE else x.ROUTINGTARIFF2CODE end) ROUTINGTARIFFCODE,"
        " r.CREATEDATE,r.EXPIREDATE");
    partialStatement.From("=INTERNATIONALFARE f, =TARIFFCROSSREF x, =ROUTING r ");
    partialStatement.Where("f.MARKET1 = %12q"
                           " and f.MARKET2 = %13q"
                           " and f.CARRIER = %14q"
                           " and %15n <= f.EXPIREDATE"
                           " and %16n >= f.CREATEDATE"
                           " and %17n <= x.EXPIREDATE"
                           " and (   %18n >=  x.CREATEDATE"
                           "      or %19n >= x.EFFDATE)"
                           " and %20n <= r.EXPIREDATE"
                           " and (   %21n >=  r.CREATEDATE"
                           "      or %22n >= r.EFFDATE)"
                           " and x.VENDOR = f.VENDOR"
                           " and x.TARIFFCROSSREFTYPE = 'D'"
                           " and x.CARRIER = f.CARRIER"
                           " and x.FARETARIFF = f.FARETARIFF"
                           " and r.VENDOR = f.VENDOR"
                           " and r.CARRIER = f.CARRIER"
                           " and r.ROUTINGTARIFF in (x.ROUTINGTARIFF1,x.ROUTINGTARIFF2)"
                           " and r.ROUTING = f.ROUTING"
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

template <class QUERYCLASS>
class QueryGetDomMarkets_RtgSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetDomMarkets_RtgSQLStatement() {}
  virtual ~QueryGetDomMarkets_RtgSQLStatement() {}

  enum ColumnIndexes
  {
    MARKET = 0,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select distinct MARKET");
    this->From("=MARKET");
    this->Where(" NATION = 'CA' or NATION = 'US'");
    this->OrderBy(" MARKET");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static const char* mapRowToMarket(Row* row) { return row->getString(MARKET); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
}
