//----------------------------------------------------------------------------
//          File:           QueryGetTaxCodeSQLStatement.h
//          Description:    QueryGetTaxCodeSQLStatement
//          Created:        10/7/2007
//          Authors:         Mike Lillis
//
//          Updates:
//
//      2007, Sabre Inc.  All rights reserved.  This software/documentation is the confidential
//     and proprietary product of Sabre Inc. Any unauthorized use, reproduction,
//     or transfer of this software/documentation, in any medium, or incorporation of this
//     software/documentation into any system or publication, is strictly prohibited
//----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxCode.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

template <class QUERYCLASS>
class QueryGetTaxCodeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxCodeSQLStatement() {}
  virtual ~QueryGetTaxCodeSQLStatement() {}

  enum ColumnIndexes
  {
    TAXCODE = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    LOCKDATE,
    NEWSEQNO,
    MEMONO,
    SPECIALPROCESSNO,
    TAXNODEC,
    TAXCDROUNDUNITNODEC,
    FARERANGENODEC,
    DISCPERCENTNODEC,
    DISCPERCENT,
    TAXAMT,
    MINTAX,
    MAXTAX,
    PLUSUPAMT,
    LOWRANGE,
    HIGHRANGE,
    RANGEINCREMENT,
    TAXCDROUNDUNIT,
    EFFDATE,
    DISCDATE,
    FIRSTTVLDATE,
    LASTTVLDATE,
    CREATORBUSINESSUNIT,
    CREATORID,
    LOC1EXCLIND,
    LOC1TYPE,
    LOC1,
    LOC2EXCLIND,
    LOC2TYPE,
    LOC2,
    TAXTYPE,
    NATION,
    TAXCDROUNDRULE,
    TAXFULLFAREIND,
    TAXEQUIVAMTIND,
    TAXEXCESSBAGIND,
    TVLDATEASORIGININD,
    DISPLAYONLYIND,
    FEEIND,
    INTERLINABLETAXIND,
    SHOWSEPARATEIND,
    POSEXCLIND,
    POSLOCTYPE,
    POSLOC,
    POIEXCLIND,
    POILOCTYPE,
    POILOC,
    SELLCUREXCLIND,
    SELLCUR,
    OCCURRENCE,
    FREETKTEXEMPT,
    IDTVLEXEMPT,
    RANGETYPE,
    RANGEIND,
    NEXTSTOPOVERRESTR,
    SPCLTAXROUNDING,
    TAXCUR,
    TAXCURNODEC,
    FARECLASSEXCLIND,
    TKTDSGEXCLIND,
    VALCXREXCLIND,
    EXEMPEQUIPEXCLIND,
    PSGREXCLIND,
    EXEMPCXREXCLIND,
    FARETYPEEXCLIND,
    MULTIOCCCONVRNDIND,
    ORIGINLOCTYPE,
    ORIGINLOC,
    ORIGINLOCEXCLIND,
    VERSIONINHERITEDIND,
    VERSIONDISPLAYIND,
    LOC1APPL,
    LOC2APPL,
    TRIPTYPE,
    TRAVELTYPE,
    ITINERARYTYPE,
    TAXONTAXEXCL,
    TAXRESTRICTIONLOCNO,
    SPECCONFIGNAME,
    FORMOFPAYMENT,
    EXCEPTIND,
    CARRIER,
    DIRECTIONALIND,
    LOC1TYPECAB,
    LOC1CAB,
    LOC2TYPECAB,
    LOC2CAB,
    CLASSOFSERVICE,
    FLIGHT1,
    FLIGHT2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select t.TAXCODE,t.VERSIONDATE,t.SEQNO,t.CREATEDATE,EXPIREDATE,LOCKDATE,"
                  "NEWSEQNO,MEMONO,SPECIALPROCESSNO,TAXNODEC,TAXCDROUNDUNITNODEC,"
                  "FARERANGENODEC,DISCPERCENTNODEC,DISCPERCENT,TAXAMT,MINTAX,MAXTAX,"
                  "PLUSUPAMT,LOWRANGE,HIGHRANGE,RANGEINCREMENT,TAXCDROUNDUNIT,"
                  "EFFDATE,DISCDATE,FIRSTTVLDATE,LASTTVLDATE,CREATORBUSINESSUNIT,"
                  "CREATORID,LOC1EXCLIND,t.LOC1TYPE,t.LOC1,LOC2EXCLIND,t.LOC2TYPE,"
                  "t.LOC2,TAXTYPE,NATION,TAXCDROUNDRULE,TAXFULLFAREIND,TAXEQUIVAMTIND,"
                  "TAXEXCESSBAGIND,TVLDATEASORIGININD,DISPLAYONLYIND,FEEIND,"
                  "INTERLINABLETAXIND,SHOWSEPARATEIND,POSEXCLIND,POSLOCTYPE,POSLOC,"
                  "POIEXCLIND,POILOCTYPE,POILOC,SELLCUREXCLIND,SELLCUR,OCCURRENCE,"
                  "FREETKTEXEMPT,IDTVLEXEMPT,RANGETYPE,RANGEIND,NEXTSTOPOVERRESTR,"
                  "SPCLTAXROUNDING,TAXCUR,TAXCURNODEC,FARECLASSEXCLIND,TKTDSGEXCLIND,"
                  "VALCXREXCLIND,EXEMPEQUIPEXCLIND,PSGREXCLIND,EXEMPCXREXCLIND,"
                  "FARETYPEEXCLIND,MULTIOCCCONVRNDIND,ORIGINLOCTYPE,ORIGINLOC,"
                  "ORIGINLOCEXCLIND,VERSIONINHERITEDIND,VERSIONDISPLAYIND,LOC1APPL,"
                  "LOC2APPL,TRIPTYPE,TRAVELTYPE,ITINERARYTYPE,TAXONTAXEXCL, TAXRESTRICTIONLOCNO,"
                  "SPECCONFIGNAME, FORMOFPAYMENT,c.EXCEPTIND,CARRIER,DIRECTIONALIND,"
                  "c.LOC1TYPE LOC1TYPECAB,c.LOC1 LOC1CAB,c.LOC2TYPE LOC2TYPECAB,"
                  "c.LOC2 LOC2CAB,CLASSOFSERVICE,c.FLIGHT1,c.FLIGHT2");

    //		        this->From("=TAXCODE t LEFT OUTER JOIN =TAXCODECABIN c"
    //		                  " USING (TAXCODE,VERSIONDATE,SEQNO,CREATEDATE)");
    //------------------------------------------------------------------------
    // *Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(4);
    joinFields.push_back("TAXCODE");
    joinFields.push_back("VERSIONDATE");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=TAXCODE", "t", "LEFT OUTER JOIN", "=TAXCODECABIN", "c", joinFields, from);
    this->From(from);

    //------------------------------------------------------------------------
    // *End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where(" t.TAXCODE = %1q"
                " and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("t.TAXCODE,t.VERSIONDATE,t.SEQNO,t.CREATEDATE,c.ORDERNO");
    else
      this->OrderBy("1,2,3,4");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TaxCodeReg* mapRowToTaxCodeReg(Row* row, TaxCodeReg* tcrPrev)
  {
    TaxCode taxCode = row->getString(TAXCODE);
    uint64_t versionDate = row->getDate(VERSIONDATE).get64BitRep();
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    tse::TaxCodeReg* tcr;

    // If parent not changed, just have a new child row
    if (tcrPrev != nullptr && taxCode == tcrPrev->taxCode() && versionDate == tcrPrev->versionDate() &&
        seqNo == tcrPrev->seqNo() && createDate == tcrPrev->createDate())
    {
      tcr = tcrPrev;
    }
    else
    { // Create new parent
      tcr = new tse::TaxCodeReg;
      tcr->taxCode() = taxCode;
      tcr->versionDate() = versionDate;
      tcr->seqNo() = seqNo;
      tcr->createDate() = createDate;

      tcr->expireDate() = row->getDate(EXPIREDATE);
      tcr->lockDate() = row->getDate(LOCKDATE);

      tcr->specialProcessNo() = row->getInt(SPECIALPROCESSNO);

      tcr->discPercentNodec() = row->getInt(DISCPERCENTNODEC);
      tcr->discPercent() =
          QUERYCLASS::adjustDecimal(row->getInt(DISCPERCENT), tcr->discPercentNodec());

      tcr->taxNodec() = row->getInt(TAXNODEC);
      tcr->taxAmt() = QUERYCLASS::adjustDecimal(row->getInt(TAXAMT), tcr->taxNodec());

      tcr->taxCurNodec() = row->getInt(TAXCURNODEC);
      tcr->minTax() = QUERYCLASS::adjustDecimal(row->getInt(MINTAX), tcr->taxCurNodec());
      tcr->maxTax() = QUERYCLASS::adjustDecimal(row->getInt(MAXTAX), tcr->taxCurNodec());
      tcr->plusupAmt() = QUERYCLASS::adjustDecimal(row->getInt(PLUSUPAMT), tcr->taxCurNodec());

      tcr->taxcdRoundUnitNodec() = row->getInt(TAXCDROUNDUNITNODEC);
      tcr->taxcdRoundUnit() =
          QUERYCLASS::adjustDecimal(row->getInt(TAXCDROUNDUNIT), tcr->taxcdRoundUnitNodec());

      tcr->rangeType() = row->getChar(RANGETYPE);
      long lowRange = row->getInt(LOWRANGE);
      long highRange = row->getLong(HIGHRANGE);
      long rangeincrement = row->getInt(RANGEINCREMENT);

      tcr->taxAmtInt().amt() = row->getInt(TAXAMT);
      tcr->taxAmtInt().nodec() = tcr->taxNodec();

      tcr->minTaxInt().amt() = row->getInt(MINTAX);
      tcr->minTaxInt().nodec() = tcr->taxCurNodec();

      tcr->maxTaxInt().amt() = row->getInt(MAXTAX);
      tcr->maxTaxInt().nodec() = tcr->taxCurNodec();

      tcr->taxcdRoundUnitInt().amt() = row->getInt(TAXCDROUNDUNIT);
      tcr->taxcdRoundUnitInt().nodec() = tcr->taxcdRoundUnitNodec();

      tcr->discPercentInt().amt() = row->getInt(DISCPERCENT);
      tcr->discPercentInt().nodec() = tcr->discPercentNodec();

      if (tcr->rangeType() == 'F')
      {
        tcr->fareRangeNodec() = row->getInt(FARERANGENODEC);
        tcr->lowRange() = QUERYCLASS::adjustDecimal(lowRange, tcr->fareRangeNodec());
        tcr->highRange() = QUERYCLASS::adjustDecimal(highRange, tcr->fareRangeNodec());
        tcr->rangeincrement() = QUERYCLASS::adjustDecimal(rangeincrement, tcr->fareRangeNodec());

        tcr->lowRangeInt().amt() = row->getInt(LOWRANGE);
        tcr->lowRangeInt().nodec() = tcr->fareRangeNodec();

        tcr->highRangeInt().amt() = row->getLong(HIGHRANGE);
        tcr->highRangeInt().nodec() = tcr->fareRangeNodec();
      }
      else
      {
        tcr->lowRange() = lowRange;
        tcr->highRange() = highRange;
        tcr->rangeincrement() = rangeincrement;

        tcr->lowRangeInt().amt() = row->getInt(LOWRANGE);
        tcr->highRangeInt().amt() = row->getLong(HIGHRANGE);
      }

      tcr->effDate() = row->getDate(EFFDATE);
      tcr->discDate() = row->getDate(DISCDATE);
      tcr->firstTvlDate() = row->getDate(FIRSTTVLDATE);
      tcr->lastTvlDate() = row->getDate(LASTTVLDATE);

      tcr->loc1ExclInd() = row->getChar(LOC1EXCLIND);
      tcr->loc1Type() = row->getChar(LOC1TYPE);
      tcr->loc1() = row->getString(LOC1);
      tcr->loc2ExclInd() = row->getChar(LOC2EXCLIND);
      tcr->loc2Type() = row->getChar(LOC2TYPE);
      tcr->loc2() = row->getString(LOC2);
      tcr->taxType() = row->getChar(TAXTYPE);
      tcr->nation() = row->getString(NATION);

      std::string roundRule = row->getString(TAXCDROUNDRULE);
      tcr->taxcdRoundRule() = EMPTY;
      if (roundRule == "D")
        tcr->taxcdRoundRule() = DOWN;
      else if (roundRule == "N")
        tcr->taxcdRoundRule() = NEAREST;
      else if (roundRule == "U")
        tcr->taxcdRoundRule() = UP;
      else if (roundRule == "O")
        tcr->taxcdRoundRule() = NONE;

      tcr->taxfullFareInd() = row->getChar(TAXFULLFAREIND);
      tcr->taxequivAmtInd() = row->getChar(TAXEQUIVAMTIND);
      tcr->taxexcessbagInd() = row->getChar(TAXEXCESSBAGIND);
      tcr->tvlDateasoriginInd() = row->getChar(TVLDATEASORIGININD);
      tcr->displayonlyInd() = row->getChar(DISPLAYONLYIND);
      tcr->feeInd() = row->getChar(FEEIND);
      tcr->interlinableTaxInd() = row->getChar(INTERLINABLETAXIND);
      tcr->showseparateInd() = row->getChar(SHOWSEPARATEIND);
      tcr->posExclInd() = row->getChar(POSEXCLIND);
      tcr->posLocType() = LocType(row->getString(POSLOCTYPE)[0]);
      tcr->posLoc() = row->getString(POSLOC);
      tcr->poiExclInd() = row->getChar(POIEXCLIND);
      tcr->poiLocType() = LocType(row->getString(POILOCTYPE)[0]);
      tcr->poiLoc() = row->getString(POILOC);
      tcr->sellCurExclInd() = row->getChar(SELLCUREXCLIND);
      tcr->sellCur() = row->getString(SELLCUR);
      tcr->occurrence() = row->getChar(OCCURRENCE);
      tcr->freeTktexempt() = row->getChar(FREETKTEXEMPT);
      tcr->idTvlexempt() = row->getChar(IDTVLEXEMPT);
      tcr->rangeInd() = row->getChar(RANGEIND);
      tcr->nextstopoverrestr() = row->getChar(NEXTSTOPOVERRESTR);
      tcr->spclTaxRounding() = row->getChar(SPCLTAXROUNDING);
      tcr->taxCur() = row->getString(TAXCUR);
      tcr->fareclassExclInd() = row->getChar(FARECLASSEXCLIND);
      tcr->tktdsgExclInd() = row->getChar(TKTDSGEXCLIND);
      tcr->valcxrExclInd() = row->getChar(VALCXREXCLIND);
      tcr->exempequipExclInd() = row->getChar(EXEMPEQUIPEXCLIND);
      tcr->psgrExclInd() = row->getChar(PSGREXCLIND);
      tcr->exempcxrExclInd() = row->getChar(EXEMPCXREXCLIND);
      tcr->fareTypeExclInd() = row->getChar(FARETYPEEXCLIND);
      tcr->multioccconvrndInd() = row->getChar(MULTIOCCCONVRNDIND);
      tcr->originLocType() = LocType(row->getString(ORIGINLOCTYPE)[0]);
      tcr->originLoc() = row->getString(ORIGINLOC);
      tcr->originLocExclInd() = row->getChar(ORIGINLOCEXCLIND);
      tcr->versioninheritedInd() = row->getChar(VERSIONINHERITEDIND);
      tcr->versiondisplayInd() = row->getChar(VERSIONDISPLAYIND);
      tcr->loc1Appl() = row->getChar(LOC1APPL);
      tcr->loc2Appl() = row->getChar(LOC2APPL);
      tcr->tripType() = row->getChar(TRIPTYPE);
      tcr->travelType() = row->getChar(TRAVELTYPE);
      tcr->itineraryType() = row->getChar(ITINERARYTYPE);
      tcr->formOfPayment() = row->getChar(FORMOFPAYMENT);
      tcr->taxOnTaxExcl() = row->getChar(TAXONTAXEXCL);
      if (row->isNull(TAXRESTRICTIONLOCNO))
      {
        tcr->taxRestrictionLocNo() = "";
      }
      else
      {
        tcr->taxRestrictionLocNo() = row->getString(TAXRESTRICTIONLOCNO);
      }

      if (row->isNull(SPECCONFIGNAME))
      {
        tcr->specConfigName() = "";
      }
      else
      {
        tcr->specConfigName() = row->getString(SPECCONFIGNAME);
      }
    }

    if (!row->isNull(EXCEPTIND))
    { // Add new Cabin & return
      TaxCodeCabin* tcc = new TaxCodeCabin;
      tcc->exceptInd() = row->getChar(EXCEPTIND);
      tcc->carrier() = row->getString(CARRIER);

      std::string direct = row->getString(DIRECTIONALIND);
      if (direct == "F")
        tcc->directionalInd() = FROM;
      else if (direct == "W")
        tcc->directionalInd() = WITHIN;
      else if (direct == "O")
        tcc->directionalInd() = ORIGIN;
      else if (direct == "X")
        tcc->directionalInd() = TERMINATE;
      else if (direct.empty() || direct == " " || direct == "B")
        tcc->directionalInd() = BETWEEN;

      LocKey* loc = &tcc->loc1();
      loc->locType() = row->getChar(LOC1TYPECAB);
      loc->loc() = row->getString(LOC1CAB);

      loc = &tcc->loc2();
      loc->locType() = row->getChar(LOC2TYPECAB);
      loc->loc() = row->getString(LOC2CAB);

      tcc->classOfService() = row->getString(CLASSOFSERVICE);

      tcc->flight1() = QUERYCLASS::checkFlightWildCard(row->getString(FLIGHT1));
      tcc->flight2() = QUERYCLASS::checkFlightWildCard(row->getString(FLIGHT2));

      tcr->cabins().push_back(tcc);
    }

    return tcr;
  } // mapRowToTaxCodeReg()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
//---------------------------------------------------------------------------------------
// TAXCODESEQTEXT

template <class QUERYCLASS>
class QueryGetTaxCodeGenTextSeqSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxCodeGenTextSeqSQLStatement() {}
  virtual ~QueryGetTaxCodeGenTextSeqSQLStatement() {}

  enum ColumnIndexes
  {
    TAXCODE = 0,
    SEQNO,
    MESSAGEDISPLAYCAT,
    MESSAGEORDERNO,
    CREATEDATE,
    EFFDATE,
    EXPIREDATE,
    ITEMNO,
    KEYWORD1,
    KEYWORD2,
    TEXT,
    VERSIONDATE,
    NUMBEROFCOLUMNS
  }; // enum*/

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select a.TAXCODE,a.SEQNO,a.MESSAGEDISPLAYCAT,a.MESSAGEORDERNO,a.CREATEDATE,"
                  "d.EFFDATE,d.EXPIREDATE,a.ITEMNO,KEYWORD1,KEYWORD2,TEXT,a.VERSIONDATE ");
    this->From("=TAXCODESEQTEXT a, =FREETEXT b, =FREETEXTSEG c, =TAXCODE d");
    this->Where("a.itemno=b.itemno"
                " and a.messagetype='TAX' and b.messagetype='TAX' and c.messagetype='TAX'"
                " and a.itemno=c.itemno"
                " and a.CREATEDATE=d.CREATEDATE"
                " and d.SEQNO=a.SEQNO"
                " and a.taxcode=d.taxcode"
                " and a.versiondate=d.versiondate"
                " and a.TAXCODE = %1q"
                " and %cd <= d.EXPIREDATE");
    this->OrderBy("a.TAXCODE, d.SEQNO, a.MESSAGEDISPLAYCAT, a.ITEMNO, c.SEQNO, a.MESSAGEORDERNO,"
                  " a.CREATEDATE, d.EFFDATE");

    adjustBaseSQL();
    this->ConstructSQL();
    QUERYCLASS::registerBaseSQL(queryName, *this);
    return *this;
  };

  static tse::TaxCodeGenText* mapRowToTaxCodeGenText(Row* row, TaxCodeGenText* taxTPrev)
  {
    char mdc = row->getChar(MESSAGEDISPLAYCAT);
    int msgOrdNo = row->getInt(MESSAGEORDERNO);
    DateTime crDt = row->getDate(CREATEDATE);
    int itNo = row->getInt(ITEMNO);

    tse::TaxCodeGenText* tct;

    if (taxTPrev != nullptr && mdc == taxTPrev->messageDisplayCat() &&
        msgOrdNo == taxTPrev->messageOrderNo() && crDt == taxTPrev->createDate() &&
        itNo == taxTPrev->itemNo())
    {
      tct = taxTPrev;
    }
    else
    {
      tct = new tse::TaxCodeGenText;
      tct->messageDisplayCat() = mdc;
      tct->messageOrderNo() = msgOrdNo;
      tct->createDate() = crDt;
      tct->itemNo() = itNo;
      tct->taxCode() = row->getString(TAXCODE);
      tct->effDate() = row->getDate(EFFDATE);
      tct->expireDate() = row->getDate(EXPIREDATE);
      tct->keyWord1() = row->getString(KEYWORD1);
      tct->keyWord2() = row->getString(KEYWORD2);
    }

    // isNull returns true for single space in oracle but in mysql it does not
    // This is causing elimination of empty string in oracle. Text is not null column.
    // if (!row->isNull(TEXT) )
    //{
    std::string tm = row->getString(TEXT);
    tct->txtMsgs().push_back(tm);
    //}
    return tct;
  }

  static long mapRowToSeqNo(Row* row) { return row->getLong(SEQNO); }

  static TaxCode mapRowToTaxCode(Row* row) { return row->getString(TAXCODE); }

  static uint64_t mapRowToVersionDate(Row* row) { return row->getDate(VERSIONDATE).get64BitRep(); }

  static DateTime mapRowToCreateDate(Row* row) { return row->getDate(CREATEDATE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

// TAXCODESEQTEXT end
//---------------------------------------------------------------------------------------

template <class QUERYCLASS>
class QueryGetTaxCodeGenTextSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxCodeGenTextSQLStatement() {}
  virtual ~QueryGetTaxCodeGenTextSQLStatement() {}

  enum ColumnIndexes
  {
    TAXCODE = 0,
    MESSAGEDISPLAYCAT,
    MESSAGEORDERNO,
    EFFDATE,
    DISCDATE,
    EXPIREDATE,
    CREATEDATE,
    ITEMNO,
    KEYWORD1,
    KEYWORD2,
    TEXT,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TAXCODE,MESSAGEDISPLAYCAT,MESSAGEORDERNO,EFFDATE,DISCDATE,"
                  "EXPIREDATE,a.CREATEDATE,a.ITEMNO,KEYWORD1,KEYWORD2,TEXT ");
    this->From("=TAXCODEGENTEXT a, =FREETEXT b, =FREETEXTSEG c");
    this->Where("a.itemno=b.itemno"
                " and b.messagetype='TAX'"
                " and b.messagetype=c.messagetype"
                " and b.itemno=c.itemno"
                " and TAXCODE = %1q"
                " and %cd <= EXPIREDATE");
    this->OrderBy("TAXCODE,MESSAGEDISPLAYCAT,MESSAGEORDERNO,CREATEDATE,"
                  "ITEMNO,SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static tse::TaxCodeGenText* mapRowToTaxCodeGenText(Row* row, TaxCodeGenText* taxTPrev)
  {
    char mdc = row->getChar(MESSAGEDISPLAYCAT);
    int msgOrdNo = row->getInt(MESSAGEORDERNO);
    DateTime crDt = row->getDate(CREATEDATE);
    int itNo = row->getInt(ITEMNO);

    tse::TaxCodeGenText* tct;

    // Parent not changed, just have a new child row
    if (taxTPrev != nullptr && mdc == taxTPrev->messageDisplayCat() &&
        msgOrdNo == taxTPrev->messageOrderNo() && crDt == taxTPrev->createDate() &&
        itNo == taxTPrev->itemNo())
    {
      tct = taxTPrev;
    }
    else
    { // Create new parent
      tct = new tse::TaxCodeGenText;
      tct->messageDisplayCat() = mdc;
      tct->messageOrderNo() = msgOrdNo;
      tct->createDate() = crDt;
      tct->itemNo() = itNo;

      tct->taxCode() = row->getString(TAXCODE);

      tct->effDate() = row->getDate(EFFDATE);
      tct->discDate() = row->getDate(DISCDATE);
      tct->expireDate() = row->getDate(EXPIREDATE);

      tct->keyWord1() = row->getString(KEYWORD1);
      tct->keyWord2() = row->getString(KEYWORD2);
    }

    // isNull returns true for single space in oracle but in mysql it does not
    // This is causing elimination of empty string in oracle. Text is not null column.
    // if (!row->isNull(TEXT) )
    //{
    std::string tm = row->getString(TEXT);
    tct->txtMsgs().push_back(tm);
    //}

    return tct;
  } // mapRowToTaxCodeGenText()

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetTaxRestrValCxrSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxRestrValCxrSQLStatement() {}
  virtual ~QueryGetTaxRestrValCxrSQLStatement() {}

  enum ColumnIndexes
  {
    TAXCODE = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    CARRIER,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TAXCODE,VERSIONDATE,SEQNO,CREATEDATE,CARRIER");
    this->From("=TAXRESTRVALCXR");
    this->Where("TAXCODE = %1q");
    this->OrderBy("1,2,3,4");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static TaxCode mapRowToTaxCode(Row* row) { return row->getString(TAXCODE); }

  static uint64_t mapRowToVersionDate(Row* row) { return row->getDate(VERSIONDATE).get64BitRep(); }

  static long mapRowToSeqNo(Row* row) { return row->getLong(SEQNO); }

  static DateTime mapRowToCreateDate(Row* row) { return row->getDate(CREATEDATE); }

  static CarrierCode mapRowToCarrier(Row* row) { return row->getString(CARRIER); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetTaxRestrPsgrSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxRestrPsgrSQLStatement() {}
  virtual ~QueryGetTaxRestrPsgrSQLStatement() {}

  enum ColumnIndexes
  {
    TAXCODE = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    PSGTYPE,
    SHOWPSG,
    FAREZEROONLY,
    MINAGE,
    MAXAGE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TAXCODE,VERSIONDATE,SEQNO,CREATEDATE,PSGTYPE,SHOWPSG,"
                  "FAREZEROONLY,MINAGE,MAXAGE");
    this->From("=TAXRESTRPSGR");
    this->Where("TAXCODE = %1q");
    this->OrderBy("1,2,3,4");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static TaxCode mapRowToTaxCode(Row* row) { return row->getString(TAXCODE); }

  static uint64_t mapRowToVersionDate(Row* row) { return row->getDate(VERSIONDATE).get64BitRep(); }

  static long mapRowToSeqNo(Row* row) { return row->getLong(SEQNO); }

  static DateTime mapRowToCreateDate(Row* row) { return row->getDate(CREATEDATE); }

  static void mapRowToTaxRestrictionPsg(Row* row, TaxRestrictionPsg& restrictionPsg)
  {
    restrictionPsg.psgType() = row->getString(PSGTYPE);
    restrictionPsg.showPsg() = row->getChar(SHOWPSG);
    restrictionPsg.fareZeroOnly() = row->getChar(FAREZEROONLY);
    restrictionPsg.minAge() = row->getInt(MINAGE);
    restrictionPsg.maxAge() = row->getInt(MAXAGE);
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

class Row;
template <class QUERYCLASS>
class QueryGetTaxRestrFareTypeSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxRestrFareTypeSQLStatement() {}
  virtual ~QueryGetTaxRestrFareTypeSQLStatement() {}

  enum ColumnIndexes
  {
    TAXCODE = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    FARETYPE,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TAXCODE,VERSIONDATE,SEQNO,CREATEDATE,FARETYPE");
    this->From("=TAXRESTRFARETYPE");
    this->Where("TAXCODE = %1q");
    this->OrderBy("1,2,3,4");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static TaxCode mapRowToTaxCode(Row* row) { return row->getString(TAXCODE); }

  static uint64_t mapRowToVersionDate(Row* row) { return row->getDate(VERSIONDATE).get64BitRep(); }

  static long mapRowToSeqNo(Row* row) { return row->getLong(SEQNO); }

  static DateTime mapRowToCreateDate(Row* row) { return row->getDate(CREATEDATE); }

  static const char* mapRowToFareType(Row* row) { return row->getString(FARETYPE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetTaxRestrFareClassSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxRestrFareClassSQLStatement() {}
  virtual ~QueryGetTaxRestrFareClassSQLStatement() {}

  enum ColumnIndexes
  {
    TAXCODE = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    FARECLASS,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TAXCODE,VERSIONDATE,SEQNO,CREATEDATE,FARECLASS");
    this->From("=TAXRESTRFARECLASS");
    this->Where("TAXCODE = %1q");
    this->OrderBy("1,2,3,4");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static TaxCode mapRowToTaxCode(Row* row) { return row->getString(TAXCODE); }

  static uint64_t mapRowToVersionDate(Row* row) { return row->getDate(VERSIONDATE).get64BitRep(); }

  static long mapRowToSeqNo(Row* row) { return row->getLong(SEQNO); }

  static DateTime mapRowToCreateDate(Row* row) { return row->getDate(CREATEDATE); }

  static const char* mapRowToFareClass(Row* row) { return row->getString(FARECLASS); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetTaxExempEquipSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxExempEquipSQLStatement() {}
  virtual ~QueryGetTaxExempEquipSQLStatement() {}

  enum ColumnIndexes
  {
    TAXCODE = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    EQUIPCODE,
    NUMBEROFCOLUMNS
  }; // enum
  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TAXCODE,VERSIONDATE,SEQNO,CREATEDATE,EQUIPCODE");
    this->From("=TAXEXEMPEQUIP");
    this->Where("TAXCODE = %1q");
    this->OrderBy("1,2,3,4");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static TaxCode mapRowToTaxCode(Row* row) { return row->getString(TAXCODE); }

  static uint64_t mapRowToVersionDate(Row* row) { return row->getDate(VERSIONDATE).get64BitRep(); }

  static long mapRowToSeqNo(Row* row) { return row->getLong(SEQNO); }

  static DateTime mapRowToCreateDate(Row* row) { return row->getDate(CREATEDATE); }

  static EquipmentType mapRowToEquipCode(Row* row) { return row->getString(EQUIPCODE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetTaxExempCxrSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxExempCxrSQLStatement() {}
  virtual ~QueryGetTaxExempCxrSQLStatement() {}

  enum ColumnIndexes
  {
    TAXCODE = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    CARRIER,
    AIRPORT1,
    AIRPORT2,
    DIRECTION,
    FLT1,
    FLT2,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TAXCODE,VERSIONDATE,SEQNO,CREATEDATE,CARRIER,"
                  "AIRPORT1,AIRPORT2,DIRECTION,FLT1,FLT2");
    this->From("=TAXEXEMPCXR");
    this->Where("TAXCODE = %1q");
    this->OrderBy("1,2,3,4");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void mapRowToTaxExemptionCarrier(Row* row, TaxExemptionCarrier& exemptionCarrier)
  {
    exemptionCarrier.airport1() = row->getString(AIRPORT1);
    exemptionCarrier.airport2() = row->getString(AIRPORT2);
    exemptionCarrier.direction() = row->getChar(DIRECTION);
    exemptionCarrier.flight1() = QUERYCLASS::checkFlightWildCard(row->getString(FLT1));
    exemptionCarrier.flight2() = QUERYCLASS::checkFlightWildCard(row->getString(FLT2));
    exemptionCarrier.carrier() = row->getString(CARRIER);
  }

  static TaxCode mapRowToTaxCode(Row* row) { return row->getString(TAXCODE); }

  static uint64_t mapRowToVersionDate(Row* row) { return row->getDate(VERSIONDATE).get64BitRep(); }

  static long mapRowToSeqNo(Row* row) { return row->getLong(SEQNO); }

  static DateTime mapRowToCreateDate(Row* row) { return row->getDate(CREATEDATE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetTaxOnTaxSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxOnTaxSQLStatement() {}
  virtual ~QueryGetTaxOnTaxSQLStatement() {}

  enum ColumnIndexes
  {
    TAXCODE = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    TAXONTAXCODE,
    NUMBEROFCOLUMNS
  }; // enum
  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TAXCODE,VERSIONDATE,SEQNO,CREATEDATE,TAXONTAXCODE");
    this->From("=TAXONTAX");
    this->Where("TAXCODE = %1q");
    if (DataManager::forceSortOrder())
      this->OrderBy("1,2,3,4,5");
    else
      this->OrderBy("1,2,3,4");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static TaxCode mapRowToTaxCode(Row* row) { return row->getString(TAXCODE); }

  static uint64_t mapRowToVersionDate(Row* row) { return row->getDate(VERSIONDATE).get64BitRep(); }

  static long mapRowToSeqNo(Row* row) { return row->getLong(SEQNO); }

  static DateTime mapRowToCreateDate(Row* row) { return row->getDate(CREATEDATE); }

  static const char* mapRowToTaxOnTaxCode(Row* row) { return row->getString(TAXONTAXCODE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetTaxRestrTransitSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxRestrTransitSQLStatement() {}
  virtual ~QueryGetTaxRestrTransitSQLStatement() {}

  enum ColumnIndexes
  {
    TAXCODE = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    ORDERNO,
    TRANSITHOURS,
    TRANSITMINUTES,
    SAMEDAYIND,
    NEXTDAYIND,
    SAMEFLIGHT,
    TRANSITTAXONLY,
    TRANSITDOMDOM,
    TRANSITDOMINTL,
    TRANSITINTLDOM,
    TRANSITINTLINTL,
    TRANSITSURFDOM,
    TRANSITSURFINTL,
    TRANSITOFFLINECXR,
    FLIGHTARRIVALHOURS,
    FLIGHTARRIVALMINUTES,
    FLIGHTDEPARTHOURS,
    FLIGHTDEPARTMINUTES,
    VIALOCTYPE,
    VIALOC,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TAXCODE,VERSIONDATE,SEQNO,CREATEDATE,ORDERNO,TRANSITHOURS,TRANSITMINUTES,"
                  "SAMEDAYIND,NEXTDAYIND,SAMEFLIGHT,TRANSITTAXONLY,TRANSITDOMDOM,TRANSITDOMINTL,"
                  "TRANSITINTLDOM,TRANSITINTLINTL,TRANSITSURFDOM,TRANSITSURFINTL,TRANSITOFFLINECXR,"
                  "FLIGHTARRIVALHOURS,FLIGHTARRIVALMINUTES,FLIGHTDEPARTHOURS,FLIGHTDEPARTMINUTES,"
                  "VIALOCTYPE,VIALOC");
    this->From("=TAXRESTRTRANSIT");
    this->Where("TAXCODE = %1q");
    if (DataManager::forceSortOrder())
      this->OrderBy("1,2,3,4,5");
    else
      this->OrderBy("1,2,3,4");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static void mapRowToTaxRestrictionTransit(Row* row, TaxRestrictionTransit& rt)
  {
    rt.orderNo() = row->getInt(ORDERNO);
    rt.transitHours() = row->getInt(TRANSITHOURS);
    rt.transitMinutes() = row->getInt(TRANSITMINUTES);
    rt.sameDayInd() = row->getChar(SAMEDAYIND);
    rt.nextDayInd() = row->getChar(NEXTDAYIND);
    rt.sameFlight() = row->getChar(SAMEFLIGHT);
    rt.transitTaxonly() = row->getChar(TRANSITTAXONLY);
    rt.transitDomDom() = row->getChar(TRANSITDOMDOM);
    rt.transitDomIntl() = row->getChar(TRANSITDOMINTL);
    rt.transitIntlDom() = row->getChar(TRANSITINTLDOM);
    rt.transitIntlIntl() = row->getChar(TRANSITINTLINTL);
    rt.transitSurfDom() = row->getChar(TRANSITSURFDOM);
    rt.transitSurfIntl() = row->getChar(TRANSITSURFINTL);
    rt.transitOfflineCxr() = row->getChar(TRANSITOFFLINECXR);
    rt.flightArrivalHours() = row->getInt(FLIGHTARRIVALHOURS);
    rt.flightArrivalMinutes() = row->getInt(FLIGHTARRIVALMINUTES);
    rt.flightDepartHours() = row->getInt(FLIGHTDEPARTHOURS);
    rt.flightArrivalMinutes() = row->getInt(FLIGHTDEPARTMINUTES);
    rt.viaLocType() = LocType(row->getString(VIALOCTYPE)[0]);
    rt.viaLoc() = row->getString(VIALOC);
  }

  static TaxCode mapRowToTaxCode(Row* row) { return row->getString(TAXCODE); }

  static uint64_t mapRowToVersionDate(Row* row) { return row->getDate(VERSIONDATE).get64BitRep(); }

  static long mapRowToSeqNo(Row* row) { return row->getLong(SEQNO); }

  static DateTime mapRowToCreateDate(Row* row) { return row->getDate(CREATEDATE); }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};
template <class QUERYCLASS>
class QueryGetTaxRestrTktDsgSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxRestrTktDsgSQLStatement() {}
  virtual ~QueryGetTaxRestrTktDsgSQLStatement() {}

  enum ColumnIndexes
  {
    TAXCODE = 0,
    VERSIONDATE,
    SEQNO,
    CREATEDATE,
    TKTDESIGNATOR,
    CARRIER,
    NUMBEROFCOLUMNS
  }; // enum
  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select TAXCODE,VERSIONDATE,SEQNO,CREATEDATE,TKTDESIGNATOR,CARRIER");
    this->From("=TAXRESTRTKTDSG");
    this->Where("TAXCODE = %1q");
    if (DataManager::forceSortOrder())
      this->OrderBy("1,2,3,4,5,6");
    else
      this->OrderBy("1,2,3,4");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static TaxCode mapRowToTaxCode(Row* row) { return row->getString(TAXCODE); }

  static uint64_t mapRowToVersionDate(Row* row) { return row->getDate(VERSIONDATE).get64BitRep(); }

  static long mapRowToSeqNo(Row* row) { return row->getLong(SEQNO); }

  static DateTime mapRowToCreateDate(Row* row) { return row->getDate(CREATEDATE); }

  static void mapRowToTaxRestrictionTktDesignator(Row* row, TaxRestrictionTktDesignator* td)
  {
    td->tktDesignator() = row->getString(TKTDESIGNATOR);
    td->carrier() = row->getString(CARRIER);
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

////////////////////////////////////////////////////////////////////////
//
//   Template used to get replace Where clause and add an OrderBy
//
///////////////////////////////////////////////////////////////////////

template <class QUERYCLASS>
class QueryGetAllTaxCodeRegsSQLStatement : public QueryGetTaxCodeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override { this->Where("%cd <= EXPIREDATE"); }
};

template <class QUERYCLASS>
class QueryGetTaxCodeHistoricalSQLStatement : public QueryGetTaxCodeSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" t.TAXCODE = %1q"
                " and %2n <= t.EXPIREDATE"
                " and (%3n >= t.CREATEDATE"
                "  or %4n >= t.EFFDATE)");
  }
};

template <class QUERYCLASS>
class QueryGetTaxCodeGenTextSeqHistoricalSQLStatement
    : public QueryGetTaxCodeGenTextSeqSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("a.itemno=b.itemno"
                " and a.messagetype='TAX' and b.messagetype='TAX' and c.messagetype='TAX'"
                " and a.itemno=c.itemno"
                " and a.CREATEDATE=d.CREATEDATE"
                " and d.SEQNO=a.SEQNO"
                " and a.taxcode=d.taxcode"
                " and a.versiondate=d.versiondate"
                " and d.TAXCODE = %1q"
                " and %2n <= d.EXPIREDATE"
                " and (%3n >= a.CREATEDATE"
                " or %4n >= d.EFFDATE)");
  }
};

template <class QUERYCLASS>
class QueryGetTaxCodeGenTextHistoricalSQLStatement
    : public QueryGetTaxCodeGenTextSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("a.itemno=b.itemno"
                " and b.messagetype='TAX'"
                " and b.messagetype=c.messagetype"
                " and b.itemno=c.itemno"
                " and TAXCODE = %1q"
                " and %2n <= a.EXPIREDATE"
                " and (%3n >= a.CREATEDATE"
                "  or %4n >= a.EFFDATE)");
  }
};

template <class QUERYCLASS>
class QueryGetTaxRestrValCxrHistoricalSQLStatement
    : public QueryGetTaxRestrValCxrSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" TAXCODE = %1q"
                " and %2n >= CREATEDATE");
  }
};

template <class QUERYCLASS>
class QueryGetTaxRestrPsgrHistoricalSQLStatement
    : public QueryGetTaxRestrPsgrSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" TAXCODE = %1q"
                " and %2n >= CREATEDATE");
  }
};

template <class QUERYCLASS>
class QueryGetTaxRestrFareTypeHistoricalSQLStatement
    : public QueryGetTaxRestrFareTypeSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" TAXCODE = %1q"
                " and %2n >= CREATEDATE");
  }
};

template <class QUERYCLASS>
class QueryGetTaxRestrFareClassHistoricalSQLStatement
    : public QueryGetTaxRestrFareClassSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" TAXCODE = %1q"
                " and %2n >= CREATEDATE");
  }
};

template <class QUERYCLASS>
class QueryGetTaxExempEquipHistoricalSQLStatement
    : public QueryGetTaxExempEquipSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" TAXCODE = %1q"
                " and %2n >= CREATEDATE");
  }
};

template <class QUERYCLASS>
class QueryGetTaxExempCxrHistoricalSQLStatement : public QueryGetTaxExempCxrSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" TAXCODE = %1q"
                " and %2n >= CREATEDATE");
  }
};

template <class QUERYCLASS>
class QueryGetTaxOnTaxHistoricalSQLStatement : public QueryGetTaxOnTaxSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" TAXCODE = %1q"
                " and %2n >= CREATEDATE");
  }
};

template <class QUERYCLASS>
class QueryGetTaxRestrTransitHistoricalSQLStatement
    : public QueryGetTaxRestrTransitSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" TAXCODE = %1q"
                " and %2n >= CREATEDATE");
  }
};

template <class QUERYCLASS>
class QueryGetTaxRestrTktDsgHistoricalSQLStatement
    : public QueryGetTaxRestrTktDsgSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where(" TAXCODE = %1q"
                " and %2n >= CREATEDATE");
  }
};

template <class QUERYCLASS>
class QueryGetAllTaxCodeRegsHistoricalSQLStatement : public QueryGetTaxCodeSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where(" %1n <= EXPIREDATE"
                " and (%2n >= t.CREATEDATE"
                "  or %3n >= EFFDATE)");
  }
};

} // tse

