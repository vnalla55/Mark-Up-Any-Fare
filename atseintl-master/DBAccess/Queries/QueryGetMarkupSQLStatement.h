//-------------------------------------------------------------------------------
// Copyright 2010, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetMarkup.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetMarkupSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetMarkupSQLStatement() {};
  virtual ~QueryGetMarkupSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    RULETARIFF,
    RULE,
    SEQNO,
    CREATORPSEUDOCITY,
    MARKUPTYPE,
    OWNERPSEUDOCITYTYPE,
    OWNERPSEUDOCITY,
    CREATEDATE,
    EXPIREDATE,
    REQUESTDATE,
    SECONDARYSELLERID,
    ACCOUNTCODE,
    OWNERID,
    REDISTRIBUTETAG,
    UPDATETAG,
    SELLTAG,
    TKTTAG,
    VIEWNETIND,
    STATUS,
    ORDERNO,
    TVLEFFDATE,
    TVLDISCDATE,
    NEGFARECALCSEQ,
    DIRECTIONALITY,
    LOC1TYPE,
    LOC1,
    LOC2TYPE,
    LOC2,
    BUNDLEDIND,
    SELLINGFAREIND,
    SELLINGPERCENTNODEC,
    SELLINGPERCENT,
    NETSELLINGIND,
    SELLINGNODEC1,
    SELLINGFAREAMT1,
    SELLINGCUR1,
    SELLINGNODEC2,
    SELLINGFAREAMT2,
    SELLINGCUR2,
    PERCENTMINNODEC,
    PERCENTMIN,
    PERCENTMAXNODEC,
    PERCENTMAX,
    MARKUPFAREIND,
    MARKUPNODEC1,
    MARKUPMINAMT1,
    MARKUPMAXAMT1,
    MARKUPCUR1,
    MARKUPNODEC2,
    MARKUPMINAMT2,
    MARKUPMAXAMT2,
    MARKUPCUR2,
    FARECLASS,
    FARETYPE,
    SEASONTYPE,
    DOWTYPE,
    WHOLESALERNODEC1,
    WHOLESALERFAREAMT1,
    WHOLESALERCUR1,
    WHOLESALERNODEC2,
    WHOLESALERFAREAMT2,
    WHOLESALERCUR2,
    WHOLESALERFAREIND,
    WHOLESALERPERCENTNODEC,
    WHOLESALERPERCENT,
    PSGTYPE,
    NOSELLIND,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    std::string hint(" /*+ leading(m) */ ");
    this->Command("select " + hint +
                  " m.VENDOR,m.CARRIER,m.RULETARIFF,m.RULE,m.SEQNO,m.CREATORPSEUDOCITY,"
                  " m.MARKUPTYPE,m.OWNERPSEUDOCITYTYPE,m.OWNERPSEUDOCITY,m.CREATEDATE,"
                  " EXPIREDATE,m.REQUESTDATE,m.SECONDARYSELLERID,m.ACCOUNTCODE,m.OWNERID,"
                  " m.REDISTRIBUTETAG,UPDATETAG,SELLTAG,TKTTAG,VIEWNETIND,STATUS,"
                  " c.ORDERNO,c.TVLEFFDATE,c.TVLDISCDATE,NEGFARECALCSEQ,DIRECTIONALITY,"
                  " c.LOC1TYPE,c.LOC1,c.LOC2TYPE,c.LOC2,BUNDLEDIND,SELLINGFAREIND,"
                  " c.SELLINGPERCENTNODEC,SELLINGPERCENT,NETSELLINGIND,SELLINGNODEC1,"
                  " c.SELLINGFAREAMT1,SELLINGCUR1,SELLINGNODEC2,SELLINGFAREAMT2,"
                  " c.SELLINGCUR2,PERCENTMINNODEC,PERCENTMIN,PERCENTMAXNODEC,PERCENTMAX,"
                  " c.MARKUPFAREIND,MARKUPNODEC1,MARKUPMINAMT1,MARKUPMAXAMT1,"
                  " MARKUPCUR1,c.MARKUPNODEC2,MARKUPMINAMT2,MARKUPMAXAMT2,MARKUPCUR2,"
                  " FARECLASS,c.FARETYPE,SEASONTYPE,DOWTYPE,WHOLESALERNODEC1,"
                  " WHOLESALERFAREAMT1,c.WHOLESALERCUR1,WHOLESALERNODEC2,"
                  " WHOLESALERFAREAMT2,WHOLESALERCUR2,c.WHOLESALERFAREIND,"
                  " WHOLESALERPERCENTNODEC,WHOLESALERPERCENT,PSGTYPE,c.NOSELLIND");
    this->From("=MARKUPCONTROL m /*! use index (PRIMARY) */, =MARKUPCALCULATE c");
    this->Where("m.VENDOR    = %1q"
                " and m.CARRIER   = %2q"
                " and m.RULETARIFF= %3n"
                " and m.RULE      = %4q"
                " and m.SEQNO     = %5n"
                " and m.OWNERPSEUDOCITY     = %6q"
                " and m.OWNERPSEUDOCITYTYPE in ('T','U') "
                " and m.MARKUPTYPE in ('U','R') "
                " and m.STATUS not in ('P','D') "
                " and %cd<= m.EXPIREDATE"
                " and m.VENDOR                = c.VENDOR"
                " and m.CARRIER               = c.CARRIER"
                " and m.RULETARIFF            = c.RULETARIFF"
                " and m.RULE                  = c.RULE"
                " and m.SEQNO                 = c.SEQNO"
                " and m.CREATORPSEUDOCITY     = c.CREATORPSEUDOCITY"
                " and m.MARKUPTYPE            = c.MARKUPTYPE"
                " and m.OWNERPSEUDOCITYTYPE   = c.OWNERPSEUDOCITYTYPE"
                " and m.OWNERPSEUDOCITY       = c.OWNERPSEUDOCITY"
                " and m.SECONDARYSELLERID     = c.SECONDARYSELLERID"
                " and m.ACCOUNTCODE           = c.ACCOUNTCODE"
                " and m.VIEWERSHIPGROUPLABEL  = c.VIEWERSHIPGROUPLABEL"
                " and m.CREATEDATE            = c.CREATEDATE");

    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR,CARRIER,RULETARIFF,RULE,SEQNO,CREATORPSEUDOCITY,MARKUPTYPE,"
                    "OWNERPSEUDOCITYTYPE,OWNERPSEUDOCITY,SECONDARYSELLERID,ACCOUNTCODE,c."
                    "VIEWERSHIPGROUPLABEL,CREATEDATE,ORDERNO");
    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::MarkupControl* mapRowToMarkupControl(Row* row, MarkupControl* mcPrev)
  {
    VendorCode vendor = row->getString(VENDOR);
    CarrierCode carrier = row->getString(CARRIER);
    TariffNumber ruleTariff = row->getInt(RULETARIFF);
    RuleNumber rule = row->getString(RULE);
    long seqNo = row->getLong(SEQNO);
    PseudoCityCode creatorPseudoCity = row->getString(CREATORPSEUDOCITY);
    Indicator markupType = row->getChar(MARKUPTYPE);
    Indicator ownerPseudoCityType = row->getChar(OWNERPSEUDOCITYTYPE);
    PseudoCityCode ownerPseudoCity = row->getString(OWNERPSEUDOCITY);
    DateTime createDate = row->getDate(CREATEDATE);
    long secondarySellerId = row->getLong(SECONDARYSELLERID);
    AccountCode accountCode = row->getString(ACCOUNTCODE);

    tse::MarkupControl* mc;

    // Parent not changed, just have a new child row
    if (mcPrev != nullptr && mcPrev->vendor() == vendor && mcPrev->carrier() == carrier &&
        mcPrev->ruleTariff() == ruleTariff && mcPrev->rule() == rule && mcPrev->seqNo() == seqNo &&
        mcPrev->creatorPseudoCity() == creatorPseudoCity && mcPrev->markupType() == markupType &&
        mcPrev->ownerPseudoCityType() == ownerPseudoCityType &&
        mcPrev->ownerPseudoCity() == ownerPseudoCity && mcPrev->createDate() == createDate &&
        mcPrev->secondarySellerId() == secondarySellerId && mcPrev->accountCode() == accountCode)
    {
      mc = mcPrev;
    }
    else
    { // Create new parent
      mc = new tse::MarkupControl;

      mc->vendor() = vendor;
      mc->carrier() = carrier;
      mc->ruleTariff() = ruleTariff;
      mc->rule() = rule;
      mc->seqNo() = seqNo;
      mc->creatorPseudoCity() = creatorPseudoCity;
      mc->markupType() = markupType;
      mc->ownerPseudoCityType() = ownerPseudoCityType;
      mc->ownerPseudoCity() = ownerPseudoCity;
      mc->createDate() = createDate;

      mc->expireDate() = row->getDate(EXPIREDATE);
      mc->requestDate() = row->getDate(REQUESTDATE);
      mc->secondarySellerId() = row->getLong(SECONDARYSELLERID);
      mc->accountCode() = row->getString(ACCOUNTCODE);
      mc->ownerId() = row->getString(OWNERID);
      mc->redistributeTag() = row->getChar(REDISTRIBUTETAG);
      mc->updateTag() = row->getChar(UPDATETAG);
      mc->sellTag() = row->getChar(SELLTAG);
      mc->tktTag() = row->getChar(TKTTAG);
      mc->viewNetInd() = row->getChar(VIEWNETIND);
      mc->status() = row->getChar(STATUS);
    }

    // Add new Carrier & return
    if (!row->isNull(ORDERNO))
    {
      MarkupCalculate* calc = new MarkupCalculate;

      calc->orderNo() = row->getInt(ORDERNO);
      calc->tvlEffDate() = row->getDate(TVLEFFDATE);
      calc->tvlDiscDate() = row->getDate(TVLDISCDATE);
      calc->negFareCalcSeq() = row->getInt(NEGFARECALCSEQ);
      calc->directionality() = row->getChar(DIRECTIONALITY);

      LocKey* loc = &calc->loc1();
      loc->locType() = row->getChar(LOC1TYPE);
      loc->loc() = row->getString(LOC1);

      loc = &calc->loc2();
      loc->locType() = row->getChar(LOC2TYPE);
      loc->loc() = row->getString(LOC2);

      calc->bundledInd() = row->getChar(BUNDLEDIND);
      calc->sellingFareInd() = row->getChar(SELLINGFAREIND);

      calc->sellingPercentNoDec() = row->getInt(SELLINGPERCENTNODEC);
      calc->sellingPercent() =
          QUERYCLASS::adjustDecimal(row->getLong(SELLINGPERCENT), calc->sellingPercentNoDec());

      calc->netSellingInd() = row->getChar(NETSELLINGIND);

      calc->sellingNoDec1() = row->getInt(SELLINGNODEC1);
      calc->sellingFareAmt1() =
          QUERYCLASS::adjustDecimal(row->getLong(SELLINGFAREAMT1), calc->sellingNoDec1());
      calc->sellingCur1() = row->getString(SELLINGCUR1);

      calc->sellingNoDec2() = row->getInt(SELLINGNODEC2);
      calc->sellingFareAmt2() =
          QUERYCLASS::adjustDecimal(row->getLong(SELLINGFAREAMT2), calc->sellingNoDec2());
      calc->sellingCur2() = row->getString(SELLINGCUR2);

      calc->percentMinNoDec() = row->getInt(PERCENTMINNODEC);
      calc->percentMin() =
          QUERYCLASS::adjustDecimal(row->getLong(PERCENTMIN), calc->percentMinNoDec());

      calc->percentMaxNoDec() = row->getInt(PERCENTMAXNODEC);
      calc->percentMax() =
          QUERYCLASS::adjustDecimal(row->getLong(PERCENTMAX), calc->percentMaxNoDec());

      calc->markupFareInd() = row->getChar(MARKUPFAREIND);

      calc->markupNoDec1() = row->getInt(MARKUPNODEC1);
      calc->markupMinAmt1() =
          QUERYCLASS::adjustDecimal(row->getLong(MARKUPMINAMT1), calc->markupNoDec1());
      calc->markupMaxAmt1() =
          QUERYCLASS::adjustDecimal(row->getLong(MARKUPMAXAMT1), calc->markupNoDec1());
      calc->markupCur1() = row->getString(MARKUPCUR1);

      calc->markupNoDec2() = row->getInt(MARKUPNODEC2);
      calc->markupMinAmt2() =
          QUERYCLASS::adjustDecimal(row->getLong(MARKUPMINAMT2), calc->markupNoDec2());
      calc->markupMaxAmt2() =
          QUERYCLASS::adjustDecimal(row->getLong(MARKUPMAXAMT2), calc->markupNoDec2());
      calc->markupCur2() = row->getString(MARKUPCUR2);

      calc->fareClass() = row->getString(FARECLASS);
      calc->fareType() = row->getString(FARETYPE);
      calc->seasonType() = row->getChar(SEASONTYPE);
      calc->dowType() = row->getChar(DOWTYPE);

      calc->wholesalerNoDec1() = row->getInt(WHOLESALERNODEC1);
      calc->wholesalerFareAmt1() =
          QUERYCLASS::adjustDecimal(row->getLong(WHOLESALERFAREAMT1), calc->wholesalerNoDec1());
      calc->wholesalerCur1() = row->getString(WHOLESALERCUR1);

      calc->wholesalerNoDec2() = row->getInt(WHOLESALERNODEC2);
      calc->wholesalerFareAmt2() =
          QUERYCLASS::adjustDecimal(row->getLong(WHOLESALERFAREAMT2), calc->wholesalerNoDec2());
      calc->wholesalerCur2() = row->getString(WHOLESALERCUR2);

      calc->wholesalerFareInd() = row->getChar(WHOLESALERFAREIND);

      calc->wholesalerPercentNoDec() = row->getInt(WHOLESALERPERCENTNODEC);
      calc->wholesalerPercent() = QUERYCLASS::adjustDecimal(row->getLong(WHOLESALERPERCENT),
                                                            calc->wholesalerPercentNoDec());

      calc->psgType() = row->getString(PSGTYPE);
      calc->noSellInd() = row->getChar(NOSELLIND);

      mc->calcs().push_back(calc);
    }

    return mc;
  } // mapRowToMarkupControl()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetMarkupSQLStatement

template <class QUERYCLASS>
class QueryGetMarkupHistoricalSQLStatement : public QueryGetMarkupSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    std::string hint1(" /*+ leading(mh) */ ");
    partialStatement.Command(
        "("
        "select " +
        hint1 + " mh.VENDOR,mh.CARRIER,mh.RULETARIFF,mh.RULE,mh.SEQNO,mh.CREATORPSEUDOCITY,"
                " mh.MARKUPTYPE,mh.OWNERPSEUDOCITYTYPE,mh.OWNERPSEUDOCITY,mh.CREATEDATE,"
                " EXPIREDATE,mh.REQUESTDATE,mh.SECONDARYSELLERID,mh.ACCOUNTCODE,mh.OWNERID,"
                " mh.REDISTRIBUTETAG,UPDATETAG,SELLTAG,TKTTAG,VIEWNETIND,STATUS,"
                " ch.ORDERNO,ch.TVLEFFDATE,ch.TVLDISCDATE,NEGFARECALCSEQ,DIRECTIONALITY,"
                " ch.LOC1TYPE,ch.LOC1,ch.LOC2TYPE,ch.LOC2,BUNDLEDIND,SELLINGFAREIND,"
                " ch.SELLINGPERCENTNODEC,SELLINGPERCENT,NETSELLINGIND,SELLINGNODEC1,"
                " ch.SELLINGFAREAMT1,SELLINGCUR1,SELLINGNODEC2,SELLINGFAREAMT2,"
                " ch.SELLINGCUR2,PERCENTMINNODEC,PERCENTMIN,PERCENTMAXNODEC,PERCENTMAX,"
                " ch.MARKUPFAREIND,MARKUPNODEC1,MARKUPMINAMT1,MARKUPMAXAMT1,"
                " MARKUPCUR1,ch.MARKUPNODEC2,MARKUPMINAMT2,MARKUPMAXAMT2,MARKUPCUR2,"
                " FARECLASS,ch.FARETYPE,SEASONTYPE,DOWTYPE,WHOLESALERNODEC1,"
                " WHOLESALERFAREAMT1,ch.WHOLESALERCUR1,WHOLESALERNODEC2,"
                " WHOLESALERFAREAMT2,WHOLESALERCUR2,ch.WHOLESALERFAREIND,"
                " WHOLESALERPERCENTNODEC,WHOLESALERPERCENT,PSGTYPE,ch.NOSELLIND");
    partialStatement.From("=MARKUPCONTROLH mh /*! use index (PRIMARY) */, =MARKUPCALCULATEH ch");
    partialStatement.Where("mh.VENDOR    = %1q"
                           " and mh.CARRIER   = %2q"
                           " and mh.RULETARIFF= %3n"
                           " and mh.RULE      = %4q"
                           " and mh.SEQNO     = %5n"
                           " and mh.OWNERPSEUDOCITY     = %6q"
                           " and mh.OWNERPSEUDOCITYTYPE in ('T','U') "
                           " and mh.MARKUPTYPE in ('U','R') "
                           " and mh.STATUS not in ('P','D') "
                           " and %7n <= mh.EXPIREDATE"
                           " and %8n >= mh.CREATEDATE"
                           " and mh.VENDOR              = ch.VENDOR"
                           " and mh.CARRIER             = ch.CARRIER"
                           " and mh.RULETARIFF          = ch.RULETARIFF"
                           " and mh.RULE                = ch.RULE"
                           " and mh.SEQNO               = ch.SEQNO"
                           " and mh.CREATORPSEUDOCITY   = ch.CREATORPSEUDOCITY"
                           " and mh.MARKUPTYPE          = ch.MARKUPTYPE"
                           " and mh.OWNERPSEUDOCITYTYPE = ch.OWNERPSEUDOCITYTYPE"
                           " and mh.OWNERPSEUDOCITY     = ch.OWNERPSEUDOCITY"
                           " and mh.SECONDARYSELLERID   = ch.SECONDARYSELLERID"
                           " and mh.ACCOUNTCODE         = ch.ACCOUNTCODE"
                           " and mh.VIEWERSHIPGROUPLABEL= ch.VIEWERSHIPGROUPLABEL"
                           " and mh.CREATEDATE          = ch.CREATEDATE"
                           ")");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    std::string hint2(" /*+ leading(m) */ ");
    partialStatement.Command(
        " union all"
        " ("
        "select " +
        hint2 + " m.VENDOR,m.CARRIER,m.RULETARIFF,m.RULE,m.SEQNO,m.CREATORPSEUDOCITY,"
                " m.MARKUPTYPE,m.OWNERPSEUDOCITYTYPE,m.OWNERPSEUDOCITY,m.CREATEDATE,"
                " EXPIREDATE,m.REQUESTDATE,m.SECONDARYSELLERID,m.ACCOUNTCODE,m.OWNERID,"
                " m.REDISTRIBUTETAG,UPDATETAG,SELLTAG,TKTTAG,VIEWNETIND,STATUS,"
                " c.ORDERNO,c.TVLEFFDATE,c.TVLDISCDATE,NEGFARECALCSEQ,DIRECTIONALITY,"
                " c.LOC1TYPE,c.LOC1,c.LOC2TYPE,c.LOC2,BUNDLEDIND,SELLINGFAREIND,"
                " c.SELLINGPERCENTNODEC,SELLINGPERCENT,NETSELLINGIND,SELLINGNODEC1,"
                " c.SELLINGFAREAMT1,SELLINGCUR1,SELLINGNODEC2,SELLINGFAREAMT2,"
                " c.SELLINGCUR2,PERCENTMINNODEC,PERCENTMIN,PERCENTMAXNODEC,PERCENTMAX,"
                " c.MARKUPFAREIND,MARKUPNODEC1,MARKUPMINAMT1,MARKUPMAXAMT1,"
                " MARKUPCUR1,c.MARKUPNODEC2,MARKUPMINAMT2,MARKUPMAXAMT2,MARKUPCUR2,"
                " FARECLASS,c.FARETYPE,SEASONTYPE,DOWTYPE,WHOLESALERNODEC1,"
                " WHOLESALERFAREAMT1,c.WHOLESALERCUR1,WHOLESALERNODEC2,"
                " WHOLESALERFAREAMT2,WHOLESALERCUR2,c.WHOLESALERFAREIND,"
                " WHOLESALERPERCENTNODEC,WHOLESALERPERCENT,PSGTYPE,c.NOSELLIND");
    partialStatement.From("=MARKUPCONTROL m /*! use index (PRIMARY) */, =MARKUPCALCULATE c");
    partialStatement.Where("m.VENDOR    = %9q"
                           " and m.CARRIER   = %10q"
                           " and m.RULETARIFF= %11n"
                           " and m.RULE      = %12q"
                           " and m.SEQNO     = %13n"
                           " and m.OWNERPSEUDOCITY  = %14q"
                           " and m.OWNERPSEUDOCITYTYPE in ('T','U') "
                           " and m.MARKUPTYPE in ('U','R') "
                           " and m.STATUS not in ('P','D') "
                           " and %15n <= m.EXPIREDATE"
                           " and %16n >= m.CREATEDATE"
                           " and m.VENDOR              = c.VENDOR"
                           " and m.CARRIER             = c.CARRIER"
                           " and m.RULETARIFF          = c.RULETARIFF"
                           " and m.RULE                = c.RULE"
                           " and m.SEQNO               = c.SEQNO"
                           " and m.CREATORPSEUDOCITY   = c.CREATORPSEUDOCITY"
                           " and m.MARKUPTYPE          = c.MARKUPTYPE"
                           " and m.OWNERPSEUDOCITYTYPE = c.OWNERPSEUDOCITYTYPE"
                           " and m.OWNERPSEUDOCITY     = c.OWNERPSEUDOCITY"
                           " and m.SECONDARYSELLERID   = c.SECONDARYSELLERID"
                           " and m.ACCOUNTCODE         = c.ACCOUNTCODE"
                           " and m.VIEWERSHIPGROUPLABEL= c.VIEWERSHIPGROUPLABEL"
                           " and m.CREATEDATE          = c.CREATEDATE"
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
}; // class QueryGetMarkupHistSQLStatement
} // tse
