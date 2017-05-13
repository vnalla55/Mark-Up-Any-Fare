//----------------------------------------------------------------------------
//          File:           QueryGetTable989SQLStatement.h
//          Description:    QueryGetTable989SQLStatement
//          Created:        11/02/2007
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
#include "DBAccess/Queries/QueryGetTable989.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetTable989SQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTable989SQLStatement() {};
  virtual ~QueryGetTable989SQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    EXPIREDATE,
    CREATEDATE,
    BASEMINFARE1,
    BASEMAXFARE1,
    BASEMINFARE2,
    BASEMAXFARE2,
    NODEC1,
    NODEC2,
    BASERULETARIFF,
    INHIBIT,
    VALIDITYIND,
    BASEFAREAPPL,
    BASEOWRT,
    BASEPUBCALC,
    BASESEASONTYPE,
    BASEDOWTYPE,
    BASEPRICINGCATTYPE,
    BASEGLOBALDIR,
    CARRIER,
    BASERULENO,
    BASEFARECLASS,
    BASEFARETYPE,
    BASEPSGTYPE,
    BASEROUTINGAPPL,
    BASEROUTING,
    BASEFOOTNOTE1,
    BASEFOOTNOTE2,
    BOOKINGCODE1,
    BOOKINGCODE2,
    MARKET1,
    MARKET2,
    BASECUR1,
    BASECUR2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,SEQNO,EXPIREDATE,CREATEDATE,BASEMINFARE1,BASEMAXFARE1,"
                  "       BASEMINFARE2,BASEMAXFARE2,NODEC1,NODEC2,BASERULETARIFF,INHIBIT,"
                  "       VALIDITYIND,BASEFAREAPPL,BASEOWRT,BASEPUBCALC,BASESEASONTYPE,"
                  "       BASEDOWTYPE,BASEPRICINGCATTYPE,BASEGLOBALDIR,CARRIER,BASERULENO,"
                  "       BASEFARECLASS,BASEFARETYPE,BASEPSGTYPE,BASEROUTINGAPPL,BASEROUTING,"
                  "       BASEFOOTNOTE1,BASEFOOTNOTE2,BOOKINGCODE1,BOOKINGCODE2,"
                  "       MARKET1,MARKET2,BASECUR1,BASECUR2");
    this->From("=BASEFAREINFOTBL ");
    this->Where("VENDOR = %1q"
                "    and ITEMNO = %2n"
                "    and %cd <= EXPIREDATE "
                "    and VALIDITYIND = 'Y'"
                "    and INHIBIT = 'N'");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, SEQNO, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::BaseFareRule* mapRowToBaseFareRule(Row* row)
  {
    tse::BaseFareRule* table989 = new tse::BaseFareRule;

    table989->vendor() = row->getString(VENDOR);
    table989->itemNo() = row->getInt(ITEMNO);
    table989->seqNo() = row->getLong(SEQNO);
    table989->expireDate() = row->getDate(EXPIREDATE);
    table989->createDate() = row->getDate(CREATEDATE);
    table989->nodec1() = row->getInt(NODEC1);
    table989->nodec2() = row->getInt(NODEC2);
    table989->baseminFare1() =
        QUERYCLASS::adjustDecimal(row->getInt(BASEMINFARE1), table989->nodec1());
    table989->baseMaxFare1() =
        QUERYCLASS::adjustDecimal(row->getInt(BASEMAXFARE1), table989->nodec1());
    table989->baseminFare2() =
        QUERYCLASS::adjustDecimal(row->getInt(BASEMINFARE2), table989->nodec2());
    table989->baseMaxFare2() =
        QUERYCLASS::adjustDecimal(row->getInt(BASEMAXFARE2), table989->nodec2());
    table989->baseRuleTariff() = row->getInt(BASERULETARIFF);
    table989->inhibit() = row->getChar(INHIBIT);
    table989->validityInd() = row->getChar(VALIDITYIND);
    table989->baseFareAppl() = row->getChar(BASEFAREAPPL);
    table989->baseowrt() = row->getChar(BASEOWRT);
    table989->basepubcalc() = row->getChar(BASEPUBCALC);
    table989->baseseasonType() = row->getChar(BASESEASONTYPE);
    table989->basedowType() = row->getChar(BASEDOWTYPE);
    table989->basepricingcatType() = row->getChar(BASEPRICINGCATTYPE);

    std::string gd = row->getString(BASEGLOBALDIR);
    strToGlobalDirection(table989->baseglobalDir(), gd);

    table989->carrier() = row->getString(CARRIER);
    table989->baseRuleNo() = row->getString(BASERULENO);
    table989->baseFareClass() = row->getString(BASEFARECLASS);
    table989->baseFareType() = row->getString(BASEFARETYPE);
    table989->basepsgType() = row->getString(BASEPSGTYPE);
    table989->baseRoutingAppl() = row->getChar(BASEROUTINGAPPL);
    table989->baseRouting() = row->getString(BASEROUTING);
    table989->basefootNote1() = row->getString(BASEFOOTNOTE1);
    table989->basefootNote2() = row->getString(BASEFOOTNOTE2);
    table989->bookingCode1() = row->getString(BOOKINGCODE1);
    table989->bookingCode2() = row->getString(BOOKINGCODE2);
    table989->market1() = row->getString(MARKET1);
    table989->market2() = row->getString(MARKET2);
    table989->baseCur1() = row->getString(BASECUR1);
    table989->baseCur2() = row->getString(BASECUR2);

    return table989;
  } // mapRowToBaseFareRule()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetTable989SQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetTable989Historical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetTable989HistoricalSQLStatement : public QueryGetTable989SQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command(
        "       select VENDOR,ITEMNO,SEQNO,EXPIREDATE,CREATEDATE,BASEMINFARE1,BASEMAXFARE1,"
        "       BASEMINFARE2,BASEMAXFARE2,NODEC1,NODEC2,BASERULETARIFF,INHIBIT,"
        "       VALIDITYIND,BASEFAREAPPL,BASEOWRT,BASEPUBCALC,BASESEASONTYPE,"
        "       BASEDOWTYPE,BASEPRICINGCATTYPE,BASEGLOBALDIR,CARRIER,BASERULENO,"
        "       BASEFARECLASS,BASEFARETYPE,BASEPSGTYPE,BASEROUTINGAPPL,BASEROUTING,"
        "       BASEFOOTNOTE1,BASEFOOTNOTE2,BOOKINGCODE1,BOOKINGCODE2,"
        "       MARKET1,MARKET2,BASECUR1,BASECUR2");
    partialStatement.From("=BASEFAREINFOTBLH");
    partialStatement.Where("VENDOR = %1q "
                           " and ITEMNO = %2n"
                           " and VALIDITYIND = 'Y'"
                           " and INHIBIT = 'N'"
                           " and %3n <= EXPIREDATE"
                           " and %4n >= CREATEDATE");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union all"
        "       select VENDOR,ITEMNO,SEQNO,EXPIREDATE,CREATEDATE,BASEMINFARE1,BASEMAXFARE1,"
        "       BASEMINFARE2,BASEMAXFARE2,NODEC1,NODEC2,BASERULETARIFF,INHIBIT,"
        "       VALIDITYIND,BASEFAREAPPL,BASEOWRT,BASEPUBCALC,BASESEASONTYPE,"
        "       BASEDOWTYPE,BASEPRICINGCATTYPE,BASEGLOBALDIR,CARRIER,BASERULENO,"
        "       BASEFARECLASS,BASEFARETYPE,BASEPSGTYPE,BASEROUTINGAPPL,BASEROUTING,"
        "       BASEFOOTNOTE1,BASEFOOTNOTE2,BOOKINGCODE1,BOOKINGCODE2,"
        "       MARKET1,MARKET2,BASECUR1,BASECUR2");
    partialStatement.From("=BASEFAREINFOTBL ");
    partialStatement.Where("VENDOR = %5q "
                           " and ITEMNO = %6n"
                           " and VALIDITYIND = 'Y'"
                           " and INHIBIT = 'N'"
                           " and %7n <= EXPIREDATE"
                           " and %8n >= CREATEDATE");
    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, SEQNO, CREATEDATE");
  }
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class QueryGetTable989HistoricalSQLStatement
} // tse
