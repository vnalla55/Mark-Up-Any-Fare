//----------------------------------------------------------------------------
//          File:           QueryGetNegFareRestSQLStatement.h
//          Description:    QueryGetNegFareRestSQLStatement
//          Created:        10/26/2007
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
#include "DBAccess/Queries/QueryGetNegFareRest.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetNegFareRestSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetNegFareRestSQLStatement() {};
  virtual ~QueryGetNegFareRestSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    UNAVAILTAG,
    TEXTTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    NEGFARESECURITYTBLITEMNO,
    NEGFARECALCTBLITEMNO,
    FAREAMT1,
    FAREAMT2,
    COMMAMT1,
    COMMAMT2,
    COMMPERCENT,
    COMMPERCENTNODEC,
    NODEC1,
    NODEC2,
    NOSEGS,
    NODEC11,
    RULETARIFF1,
    NODEC21,
    RULETARIFF2,
    NETREMITMETHOD,
    NETGROSSIND,
    UPGRADEIND,
    BAGTYPEIND,
    VALIDATIONIND,
    TKTAPPL,
    PSGTYPE,
    CUR1,
    CUR2,
    BAGNO,
    CARRIER,
    COUPONIND1,
    TOURBOXCODETYPE1,
    TKTFAREDATAIND1,
    OWRT1,
    SEASONTYPE1,
    DOWTYPE1,
    TOURBOXCODE1,
    TKTDESIGNATOR1,
    GLOBALDIR1,
    CARRIER11,
    RULE1,
    FARECLASS,
    FARETYPE1,
    BETWCITY1,
    ANDCITY1,
    FAREBOXTEXT1,
    CUR11,
    COUPONIND2,
    TOURBOXCODETYPE2,
    TKTFAREDATAIND2,
    OWRT2,
    SEASONTYPE2,
    DOWTYPE2,
    TOURBOXCODE2,
    TKTDESIGNATOR2,
    GLOBALDIR2,
    CARRIER21,
    RULE2,
    FARECLASS2,
    FARETYPE2,
    BETWCITY2,
    ANDCITY2,
    FAREBOXTEXT2,
    CUR21,
    INHIBIT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,UNAVAILTAG,"
                  "       TEXTTBLITEMNO,OVERRIDEDATETBLITEMNO,NEGFARESECURITYTBLITEMNO,"
                  "       NEGFARECALCTBLITEMNO,FAREAMT1,FAREAMT2,COMMAMT1,COMMAMT2,"
                  "       COMMPERCENT,COMMPERCENTNODEC,NODEC1,NODEC2,NOSEGS,NODEC11,"
                  "       RULETARIFF1,NODEC21,RULETARIFF2,NETREMITMETHOD,NETGROSSIND,UPGRADEIND,"
                  "       BAGTYPEIND,VALIDATIONIND,TKTAPPL,PSGTYPE,CUR1,CUR2,BAGNO,CARRIER,"
                  "       COUPONIND1,TOURBOXCODETYPE1,TKTFAREDATAIND1,OWRT1,SEASONTYPE1,DOWTYPE1,"
                  "       TOURBOXCODE1,TKTDESIGNATOR1,GLOBALDIR1,CARRIER11,RULE1,FARECLASS,"
                  "       FARETYPE1,BETWCITY1,ANDCITY1,FAREBOXTEXT1,CUR11,COUPONIND2,"
                  "       TOURBOXCODETYPE2,TKTFAREDATAIND2,OWRT2,SEASONTYPE2,DOWTYPE2,"
                  "       TOURBOXCODE2,TKTDESIGNATOR2,GLOBALDIR2,CARRIER21,RULE2,FARECLASS2,"
                  "       FARETYPE2,BETWCITY2,ANDCITY2,FAREBOXTEXT2,CUR21,INHIBIT");
    this->From("=NEGFAREREST");
    this->Where("VENDOR = %1q"
                "    and ITEMNO = %2n"
                "    and %cd <= EXPIREDATE "
                "    and VALIDITYIND = 'Y'");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,ITEMNO,CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::NegFareRest* mapRowToNegFareRest(Row* row)
  {
    tse::NegFareRest* nfr = new tse::NegFareRest;

    nfr->vendor() = row->getString(VENDOR);
    nfr->itemNo() = row->getInt(ITEMNO);
    nfr->createDate() = row->getDate(CREATEDATE);
    nfr->expireDate() = row->getDate(EXPIREDATE);
    nfr->unavailTag() = row->getChar(UNAVAILTAG);
    nfr->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    nfr->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    nfr->negFareSecurityTblItemNo() = row->getInt(NEGFARESECURITYTBLITEMNO);
    nfr->negFareCalcTblItemNo() = row->getInt(NEGFARECALCTBLITEMNO);

    nfr->noDec11() = row->getInt(NODEC11);
    nfr->fareAmt1() = QUERYCLASS::adjustDecimal(row->getInt(FAREAMT1), nfr->noDec11());

    nfr->noDec21() = row->getInt(NODEC21);
    nfr->fareAmt2() = QUERYCLASS::adjustDecimal(row->getInt(FAREAMT2), nfr->noDec21());

    nfr->noDec1() = row->getInt(NODEC1);
    nfr->commAmt1() = QUERYCLASS::adjustDecimal(row->getInt(COMMAMT1), nfr->noDec1());

    nfr->noDec2() = row->getInt(NODEC2);
    nfr->commAmt2() = QUERYCLASS::adjustDecimal(row->getInt(COMMAMT2), nfr->noDec2());

    nfr->commPercentNoDec() = row->getInt(COMMPERCENTNODEC);
    nfr->commPercent() =
        QUERYCLASS::adjustDecimal(row->getInt(COMMPERCENT), nfr->commPercentNoDec());

    nfr->noSegs() = row->getInt(NOSEGS);
    nfr->ruleTariff1() = row->getInt(RULETARIFF1);
    nfr->ruleTariff2() = row->getInt(RULETARIFF2);
    nfr->netRemitMethod() = row->getChar(NETREMITMETHOD);
    nfr->netGrossInd() = row->getChar(NETGROSSIND);
    nfr->upgradeInd() = row->getChar(UPGRADEIND);
    nfr->bagTypeInd() = row->getChar(BAGTYPEIND);
    nfr->validationInd() = row->getChar(VALIDATIONIND);
    nfr->tktAppl() = row->getChar(TKTAPPL);
    nfr->psgType() = row->getString(PSGTYPE);
    nfr->cur1() = row->getString(CUR1);
    nfr->cur2() = row->getString(CUR2);
    nfr->bagNo() = row->getString(BAGNO);
    nfr->carrier() = row->getString(CARRIER);
    nfr->couponInd1() = row->getChar(COUPONIND1);
    nfr->tourBoxCodeType1() = row->getChar(TOURBOXCODETYPE1);
    nfr->tktFareDataInd1() = row->getChar(TKTFAREDATAIND1);
    nfr->owrt1() = row->getChar(OWRT1);
    nfr->seasonType1() = row->getChar(SEASONTYPE1);
    nfr->dowType1() = row->getChar(DOWTYPE1);
    nfr->tourBoxCode1() = row->getString(TOURBOXCODE1);
    nfr->tktDesignator1() = row->getString(TKTDESIGNATOR1);

    std::string gd = row->getString(GLOBALDIR1);
    strToGlobalDirection(nfr->globalDir1(), gd);

    nfr->carrier11() = row->getString(CARRIER11);
    nfr->rule1() = row->getString(RULE1);
    nfr->fareClass1() = row->getString(FARECLASS);
    nfr->fareType1() = row->getString(FARETYPE1);
    nfr->betwCity1() = row->getString(BETWCITY1);
    nfr->andCity1() = row->getString(ANDCITY1);
    nfr->fareBoxText1() = row->getString(FAREBOXTEXT1);
    nfr->cur11() = row->getString(CUR11);
    nfr->couponInd2() = row->getChar(COUPONIND2);
    nfr->tourBoxCodeType2() = row->getChar(TOURBOXCODETYPE2);
    nfr->tktFareDataInd2() = row->getChar(TKTFAREDATAIND2);
    nfr->owrt2() = row->getChar(OWRT2);
    nfr->seasonType2() = row->getChar(SEASONTYPE2);
    nfr->dowType2() = row->getChar(DOWTYPE2);
    nfr->tourBoxCode2() = row->getString(TOURBOXCODE2);
    nfr->tktDesignator2() = row->getString(TKTDESIGNATOR2);

    gd = row->getString(GLOBALDIR2);
    strToGlobalDirection(nfr->globalDir2(), gd);

    nfr->carrier21() = row->getString(CARRIER21);
    nfr->rule2() = row->getString(RULE2);
    nfr->fareClass2() = row->getString(FARECLASS2);
    nfr->fareType2() = row->getString(FARETYPE2);
    nfr->betwCity2() = row->getString(BETWCITY2);
    nfr->andCity2() = row->getString(ANDCITY2);
    nfr->fareBoxText2() = row->getString(FAREBOXTEXT2);
    nfr->cur21() = row->getString(CUR21);
    nfr->inhibit() = row->getChar(INHIBIT);

    return nfr;
  } // mapRowToNegFareRest()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetNegFareRestSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetNegFareRestHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetNegFareRestHistoricalSQLStatement : public QueryGetNegFareRestSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command(
        "("
        "select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,UNAVAILTAG,"
        "       TEXTTBLITEMNO,OVERRIDEDATETBLITEMNO,NEGFARESECURITYTBLITEMNO,"
        "       NEGFARECALCTBLITEMNO,FAREAMT1,FAREAMT2,COMMAMT1,COMMAMT2,"
        "       COMMPERCENT,COMMPERCENTNODEC,NODEC1,NODEC2,NOSEGS,NODEC11,"
        "       RULETARIFF1,NODEC21,RULETARIFF2,NETREMITMETHOD,NETGROSSIND,UPGRADEIND,"
        "       BAGTYPEIND,VALIDATIONIND,TKTAPPL,PSGTYPE,CUR1,CUR2,BAGNO,CARRIER,"
        "       COUPONIND1,TOURBOXCODETYPE1,TKTFAREDATAIND1,OWRT1,SEASONTYPE1,DOWTYPE1,"
        "       TOURBOXCODE1,TKTDESIGNATOR1,GLOBALDIR1,CARRIER11,RULE1,FARECLASS,"
        "       FARETYPE1,BETWCITY1,ANDCITY1,FAREBOXTEXT1,CUR11,COUPONIND2,"
        "       TOURBOXCODETYPE2,TKTFAREDATAIND2,OWRT2,SEASONTYPE2,DOWTYPE2,"
        "       TOURBOXCODE2,TKTDESIGNATOR2,GLOBALDIR2,CARRIER21,RULE2,FARECLASS2,"
        "       FARETYPE2,BETWCITY2,ANDCITY2,FAREBOXTEXT2,CUR21,INHIBIT");
    partialStatement.From("=NEGFARERESTH");
    partialStatement.Where("VENDOR = %1q"
                           " and ITEMNO = %2n"
                           " and VALIDITYIND = 'Y'"
                           " and %3n <= EXPIREDATE"
                           " and %4n >= CREATEDATE"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union all"
        " ("
        "select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,UNAVAILTAG,"
        "       TEXTTBLITEMNO,OVERRIDEDATETBLITEMNO,NEGFARESECURITYTBLITEMNO,"
        "       NEGFARECALCTBLITEMNO,FAREAMT1,FAREAMT2,COMMAMT1,COMMAMT2,"
        "       COMMPERCENT,COMMPERCENTNODEC,NODEC1,NODEC2,NOSEGS,NODEC11,"
        "       RULETARIFF1,NODEC21,RULETARIFF2,NETREMITMETHOD,NETGROSSIND,UPGRADEIND,"
        "       BAGTYPEIND,VALIDATIONIND,TKTAPPL,PSGTYPE,CUR1,CUR2,BAGNO,CARRIER,"
        "       COUPONIND1,TOURBOXCODETYPE1,TKTFAREDATAIND1,OWRT1,SEASONTYPE1,DOWTYPE1,"
        "       TOURBOXCODE1,TKTDESIGNATOR1,GLOBALDIR1,CARRIER11,RULE1,FARECLASS,"
        "       FARETYPE1,BETWCITY1,ANDCITY1,FAREBOXTEXT1,CUR11,COUPONIND2,"
        "       TOURBOXCODETYPE2,TKTFAREDATAIND2,OWRT2,SEASONTYPE2,DOWTYPE2,"
        "       TOURBOXCODE2,TKTDESIGNATOR2,GLOBALDIR2,CARRIER21,RULE2,FARECLASS2,"
        "       FARETYPE2,BETWCITY2,ANDCITY2,FAREBOXTEXT2,CUR21,INHIBIT");
    partialStatement.From("=NEGFAREREST");
    partialStatement.Where("VENDOR = %5q"
                           " and ITEMNO = %6n"
                           " and VALIDITYIND = 'Y'"
                           " and %7n <= EXPIREDATE"
                           " and %8n >= CREATEDATE"
                           ")");

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

}; // class QueryGetNegFareRestHistoricalSQLStatement
} // tse
