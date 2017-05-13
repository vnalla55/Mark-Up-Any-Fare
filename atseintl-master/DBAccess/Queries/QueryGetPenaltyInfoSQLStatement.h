//----------------------------------------------------------------------------
//          File:           QueryGetPenaltyInfoSQLStatement.h
//          Description:    QueryGetPenaltyInfoSQLStatement
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
#include "DBAccess/Queries/QueryGetPenaltyInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetPenaltyInfoSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetPenaltyInfoSQLStatement() {}
  virtual ~QueryGetPenaltyInfoSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    UNAVAILTAG,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    GEOTBLITEMNO,
    PENALTYNODEC1,
    PENALTYAMT1,
    PENALTYNODEC2,
    PENALTYAMT2,
    PENALTYPERCENTNODEC,
    PENALTYPERCENT,
    VOLAPPL,
    INVOLAPPL,
    CANCELREFUNDAPPL,
    NOREFUNDIND,
    PENALTYCANCEL,
    PENALTYFAIL,
    PENALTYREISSUE,
    PENALTYEXCHANGE,
    PENALTYNOREISSUE,
    PENALTYREFUND,
    PENALTYPTA,
    PENALTYAPPL,
    PENALTYPORTION,
    PENALTYCUR1,
    PENALTYCUR2,
    PENALTYHLIND,
    WAIVERPSGDEATH,
    WAIVERPSGILL,
    WAIVERPSGFAMDEATH,
    WAIVERPSGFAMILL,
    WAIVERSCHEDCHG,
    WAIVERTKTUPGRADE,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,UNAVAILTAG,TEXTTBLITEMNO,"
                  "       OVERRIDEDATETBLITEMNO,GEOTBLITEMNO,PENALTYNODEC1,PENALTYAMT1,"
                  "       PENALTYNODEC2,PENALTYAMT2,PENALTYPERCENTNODEC,PENALTYPERCENT,"
                  "       VOLAPPL,INVOLAPPL,CANCELREFUNDAPPL,NOREFUNDIND,PENALTYCANCEL,"
                  "       PENALTYFAIL,PENALTYREISSUE,PENALTYEXCHANGE,PENALTYNOREISSUE,"
                  "       PENALTYREFUND,PENALTYPTA,PENALTYAPPL,PENALTYPORTION,PENALTYCUR1,"
                  "       PENALTYCUR2,PENALTYHLIND,WAIVERPSGDEATH,WAIVERPSGILL,"
                  "       WAIVERPSGFAMDEATH,WAIVERPSGFAMILL,WAIVERSCHEDCHG,WAIVERTKTUPGRADE,"
                  "       INHIBIT ");
    this->From("=PENALTIES ");
    this->Where("VENDOR = %1q"
                "    and ITEMNO = %2n"
                "    and VALIDITYIND = 'Y'"
                "    and %cd <= EXPIREDATE ");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,ITEMNO,CREATEDATE");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::PenaltyInfo* mapRowToPenaltyInfo(Row* row)
  {
    tse::PenaltyInfo* pi = new tse::PenaltyInfo;

    pi->vendor() = row->getString(VENDOR);
    pi->itemNo() = row->getInt(ITEMNO);
    pi->createDate() = row->getDate(CREATEDATE);
    pi->expireDate() = row->getDate(EXPIREDATE);
    pi->unavailTag() = row->getChar(UNAVAILTAG);
    pi->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    pi->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    pi->geoTblItemNo() = row->getInt(GEOTBLITEMNO);

    pi->penaltyNoDec1() = row->getInt(PENALTYNODEC1);
    pi->penaltyAmt1() = QUERYCLASS::adjustDecimal(row->getInt(PENALTYAMT1), pi->penaltyNoDec1());

    pi->penaltyNoDec2() = row->getInt(PENALTYNODEC2);
    pi->penaltyAmt2() = QUERYCLASS::adjustDecimal(row->getInt(PENALTYAMT2), pi->penaltyNoDec2());

    pi->penaltyPercentNoDec() = row->getInt(PENALTYPERCENTNODEC);
    pi->penaltyPercent() =
        QUERYCLASS::adjustDecimal(row->getInt(PENALTYPERCENT), pi->penaltyPercentNoDec());

    pi->volAppl() = row->getChar(VOLAPPL);
    pi->involAppl() = row->getChar(INVOLAPPL);
    pi->cancelRefundAppl() = row->getChar(CANCELREFUNDAPPL);
    pi->noRefundInd() = row->getChar(NOREFUNDIND);
    pi->penaltyCancel() = row->getChar(PENALTYCANCEL);
    pi->penaltyFail() = row->getChar(PENALTYFAIL);
    pi->penaltyReissue() = row->getChar(PENALTYREISSUE);
    pi->penaltyExchange() = row->getChar(PENALTYEXCHANGE);
    pi->penaltyNoReissue() = row->getChar(PENALTYNOREISSUE);
    pi->penaltyRefund() = row->getChar(PENALTYREFUND);
    pi->penaltyPta() = row->getChar(PENALTYPTA);
    pi->penaltyAppl() = row->getChar(PENALTYAPPL);
    pi->penaltyPortion() = row->getChar(PENALTYPORTION);
    pi->penaltyCur1() = row->getString(PENALTYCUR1);
    pi->penaltyCur2() = row->getString(PENALTYCUR2);
    pi->penaltyHlInd() = row->getChar(PENALTYHLIND);
    pi->waiverPsgDeath() = row->getChar(WAIVERPSGDEATH);
    pi->waiverPsgIll() = row->getChar(WAIVERPSGILL);
    pi->waiverPsgFamDeath() = row->getChar(WAIVERPSGFAMDEATH);
    pi->waiverPsgFamIll() = row->getChar(WAIVERPSGFAMILL);
    pi->waiverSchedChg() = row->getChar(WAIVERSCHEDCHG);
    pi->waiverTktUpgrade() = row->getChar(WAIVERTKTUPGRADE);
    pi->inhibit() = row->getChar(INHIBIT);

    return pi;
  } // mapRowToPenaltyInfo()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetPenaltyInfoSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetPenaltyInfoHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetPenaltyInfoHistoricalSQLStatement : public QueryGetPenaltyInfoSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command(
        "   select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,UNAVAILTAG,TEXTTBLITEMNO,"
        "   OVERRIDEDATETBLITEMNO,GEOTBLITEMNO,PENALTYNODEC1,PENALTYAMT1,"
        "   PENALTYNODEC2,PENALTYAMT2,PENALTYPERCENTNODEC,PENALTYPERCENT,"
        "   VOLAPPL,INVOLAPPL,CANCELREFUNDAPPL,NOREFUNDIND,PENALTYCANCEL,"
        "   PENALTYFAIL,PENALTYREISSUE,PENALTYEXCHANGE,PENALTYNOREISSUE,"
        "   PENALTYREFUND,PENALTYPTA,PENALTYAPPL,PENALTYPORTION,PENALTYCUR1,"
        "   PENALTYCUR2,PENALTYHLIND,WAIVERPSGDEATH,WAIVERPSGILL,"
        "   WAIVERPSGFAMDEATH,WAIVERPSGFAMILL,WAIVERSCHEDCHG,WAIVERTKTUPGRADE,"
        "   INHIBIT ");
    partialStatement.From("=PENALTIESH");
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
        "   select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,UNAVAILTAG,TEXTTBLITEMNO,"
        "   OVERRIDEDATETBLITEMNO,GEOTBLITEMNO,PENALTYNODEC1,PENALTYAMT1,"
        "   PENALTYNODEC2,PENALTYAMT2,PENALTYPERCENTNODEC,PENALTYPERCENT,"
        "   VOLAPPL,INVOLAPPL,CANCELREFUNDAPPL,NOREFUNDIND,PENALTYCANCEL,"
        "   PENALTYFAIL,PENALTYREISSUE,PENALTYEXCHANGE,PENALTYNOREISSUE,"
        "   PENALTYREFUND,PENALTYPTA,PENALTYAPPL,PENALTYPORTION,PENALTYCUR1,"
        "   PENALTYCUR2,PENALTYHLIND,WAIVERPSGDEATH,WAIVERPSGILL,"
        "   WAIVERPSGFAMDEATH,WAIVERPSGFAMILL,WAIVERSCHEDCHG,WAIVERTKTUPGRADE,"
        "   INHIBIT ");
    partialStatement.From("=PENALTIES ");
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
    this->OrderBy("");
  }
  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}

}; // class QueryGetPenaltyInfoHistoricalSQLStatement
}
