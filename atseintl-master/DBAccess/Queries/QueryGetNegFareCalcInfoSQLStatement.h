//----------------------------------------------------------------------------
//          File:           QueryGetNegFareCalcInfoSQLStatement.h
//          Description:    QueryGetNegFareCalcInfoSQLStatement
//          Created:        10/26/2007
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
#include "DBAccess/Queries/QueryGetNegFareCalcInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetNegFareCalcInfoSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetNegFareCalcInfoSQLStatement() {};
  virtual ~QueryGetNegFareCalcInfoSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    DIRECTIONALITY,
    LOC1TYPE,
    LOC1,
    USERDEFZONE1,
    LOC2TYPE,
    LOC2,
    USERDEFZONE2,
    BUNDLEDIND,
    NETSELLINGIND,
    FAREIND,
    SELLINGPERCENTNODEC,
    SELLINGPERCENT,
    SELLINGNODEC1,
    SELLINGFAREAMT1,
    SELLINGCUR1,
    SELLINGNODEC2,
    SELLINGFAREAMT2,
    SELLINGCUR2,
    CALCPERCENTMINNODEC,
    CALCPERCENTMIN,
    CALCPERCENTMAXNODEC,
    CALCPERCENTMAX,
    CALCNODEC1,
    CALCMINFAREAMT1,
    CALCMAXFAREAMT1,
    CALCCUR1,
    CALCNODEC2,
    CALCMINFAREAMT2,
    CALCMAXFAREAMT2,
    CALCCUR2,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select VENDOR,ITEMNO,SEQNO,CREATEDATE,EXPIREDATE,DIRECTIONALITY,LOC1TYPE,LOC1,"
        "       USERDEFZONE1,LOC2TYPE,LOC2,USERDEFZONE2,BUNDLEDIND,NETSELLINGIND,FAREIND,"
        "       SELLINGPERCENTNODEC,SELLINGPERCENT,SELLINGNODEC1,SELLINGFAREAMT1,SELLINGCUR1,"
        "       SELLINGNODEC2,SELLINGFAREAMT2,SELLINGCUR2,CALCPERCENTMINNODEC,CALCPERCENTMIN,"
        "       CALCPERCENTMAXNODEC,CALCPERCENTMAX,CALCNODEC1,CALCMINFAREAMT1,CALCMAXFAREAMT1,"
        "       CALCCUR1,CALCNODEC2,CALCMINFAREAMT2,CALCMAXFAREAMT2,CALCCUR2,INHIBIT");
    this->From("=NEGFARECALC");
    this->Where("VENDOR = %1q"
                "    and ITEMNO = %2n"
                "    and VALIDITYIND = 'Y'"
                "    and %cd <= EXPIREDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,ITEMNO,SEQNO,CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::NegFareCalcInfo* mapRowToNegFareCalcInfo(Row* row)
  {
    tse::NegFareCalcInfo* nfc = new tse::NegFareCalcInfo;

    nfc->vendor() = row->getString(VENDOR);
    nfc->itemNo() = row->getInt(ITEMNO);
    nfc->seqNo() = row->getLong(SEQNO);
    nfc->createDate() = row->getDate(CREATEDATE);
    nfc->expireDate() = row->getDate(EXPIREDATE);
    nfc->directionality() = row->getChar(DIRECTIONALITY);

    LocKey* loc = &nfc->loc1();
    loc->locType() = row->getChar(LOC1TYPE);
    loc->loc() = row->getString(LOC1);
    nfc->userDefZone1() = row->getString(USERDEFZONE1);

    loc = &nfc->loc2();
    loc->locType() = row->getChar(LOC2TYPE);
    loc->loc() = row->getString(LOC2);
    nfc->userDefZone2() = row->getString(USERDEFZONE2);

    nfc->bundledInd() = row->getChar(BUNDLEDIND);
    nfc->netSellingInd() = row->getChar(NETSELLINGIND);
    nfc->fareInd() = row->getChar(FAREIND);

    nfc->sellingPercentNoDec() = row->getInt(SELLINGPERCENTNODEC);
    nfc->sellingPercent() =
        QUERYCLASS::adjustDecimal(row->getInt(SELLINGPERCENT), nfc->sellingPercentNoDec());

    nfc->sellingNoDec1() = row->getInt(SELLINGNODEC1);
    nfc->sellingFareAmt1() =
        QUERYCLASS::adjustDecimal(row->getInt(SELLINGFAREAMT1), nfc->sellingNoDec1());
    nfc->sellingCur1() = row->getString(SELLINGCUR1);

    nfc->sellingNoDec2() = row->getInt(SELLINGNODEC2);
    nfc->sellingFareAmt2() =
        QUERYCLASS::adjustDecimal(row->getInt(SELLINGFAREAMT2), nfc->sellingNoDec2());
    nfc->sellingCur2() = row->getString(SELLINGCUR2);

    nfc->calcPercentMinNoDec() = row->getInt(CALCPERCENTMINNODEC);
    nfc->calcPercentMin() =
        QUERYCLASS::adjustDecimal(row->getInt(CALCPERCENTMIN), nfc->calcPercentMinNoDec());

    nfc->calcPercentMaxNoDec() = row->getInt(CALCPERCENTMAXNODEC);
    nfc->calcPercentMax() =
        QUERYCLASS::adjustDecimal(row->getInt(CALCPERCENTMAX), nfc->calcPercentMaxNoDec());

    nfc->calcNoDec1() = row->getInt(CALCNODEC1);
    nfc->calcMinFareAmt1() =
        QUERYCLASS::adjustDecimal(row->getInt(CALCMINFAREAMT1), nfc->calcNoDec1());
    nfc->calcMaxFareAmt1() =
        QUERYCLASS::adjustDecimal(row->getInt(CALCMAXFAREAMT1), nfc->calcNoDec1());
    nfc->calcCur1() = row->getString(CALCCUR1);

    nfc->calcNoDec2() = row->getInt(CALCNODEC2);
    nfc->calcMinFareAmt2() =
        QUERYCLASS::adjustDecimal(row->getInt(CALCMINFAREAMT2), nfc->calcNoDec2());
    nfc->calcMaxFareAmt2() =
        QUERYCLASS::adjustDecimal(row->getInt(CALCMAXFAREAMT2), nfc->calcNoDec2());
    nfc->calcCur2() = row->getString(CALCCUR2);

    nfc->inhibit() = row->getChar(INHIBIT);

    return nfc;
  } // mapRowToNegFareCalcInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetNegFareCalcInfoSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetNegFareCalcInfoHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetNegFareCalcInfoHistoricalSQLStatement
    : public QueryGetNegFareCalcInfoSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command(
        "select VENDOR,ITEMNO,SEQNO,CREATEDATE,EXPIREDATE,DIRECTIONALITY,LOC1TYPE,LOC1,"
        "       USERDEFZONE1,LOC2TYPE,LOC2,USERDEFZONE2,BUNDLEDIND,NETSELLINGIND,FAREIND,"
        "       SELLINGPERCENTNODEC,SELLINGPERCENT,SELLINGNODEC1,SELLINGFAREAMT1,SELLINGCUR1,"
        "       SELLINGNODEC2,SELLINGFAREAMT2,SELLINGCUR2,CALCPERCENTMINNODEC,CALCPERCENTMIN,"
        "       CALCPERCENTMAXNODEC,CALCPERCENTMAX,CALCNODEC1,CALCMINFAREAMT1,CALCMAXFAREAMT1,"
        "       CALCCUR1,CALCNODEC2,CALCMINFAREAMT2,CALCMAXFAREAMT2,CALCCUR2,INHIBIT");
    partialStatement.From("=NEGFARECALCH");
    partialStatement.Where("VENDOR = %1q "
                           " and ITEMNO = %2n"
                           " and VALIDITYIND = 'Y'"
                           " and %3n <= EXPIREDATE"
                           " and %4n >= CREATEDATE");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union all"
        " select VENDOR,ITEMNO,SEQNO,CREATEDATE,EXPIREDATE,DIRECTIONALITY,LOC1TYPE,LOC1,"
        "       USERDEFZONE1,LOC2TYPE,LOC2,USERDEFZONE2,BUNDLEDIND,NETSELLINGIND,FAREIND,"
        "       SELLINGPERCENTNODEC,SELLINGPERCENT,SELLINGNODEC1,SELLINGFAREAMT1,SELLINGCUR1,"
        "       SELLINGNODEC2,SELLINGFAREAMT2,SELLINGCUR2,CALCPERCENTMINNODEC,CALCPERCENTMIN,"
        "       CALCPERCENTMAXNODEC,CALCPERCENTMAX,CALCNODEC1,CALCMINFAREAMT1,CALCMAXFAREAMT1,"
        "       CALCCUR1,CALCNODEC2,CALCMINFAREAMT2,CALCMAXFAREAMT2,CALCCUR2,INHIBIT");
    partialStatement.From("=NEGFARECALC ");
    partialStatement.Where("VENDOR = %5q "
                           " and ITEMNO = %6n"
                           " and VALIDITYIND = 'Y'"
                           " and %7n <= EXPIREDATE"
                           " and %8n >= CREATEDATE");

    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,ITEMNO,SEQNO,CREATEDATE");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class QueryGetNegFareCalcInfoHistoricalSQLStatement
} // tse
