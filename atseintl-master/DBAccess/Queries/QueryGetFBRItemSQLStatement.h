//----------------------------------------------------------------------------
//          File:           QueryGetFBRItemSQLStatement.h
//          Description:    QueryGetFBRItemSQLStatement
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

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetFareTypeMatrixs.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetFBRItemSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetFBRItemSQLStatement() {};
  virtual ~QueryGetFBRItemSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    TEXTTBLITEMNO,
    BASETABLEITEMNO,
    MINMILEAGE,
    MAXMILEAGE,
    RESULTROUTINGTARIFF,
    RESULTMPM,
    BOOKINGCODETBLITEMNO,
    SPECIFIEDFAREAMT1,
    SPECIFIEDFAREAMT2,
    MINFAREAMT1,
    MAXFAREAMT1,
    MINFAREAMT2,
    MAXFAREAMT2,
    MINAGE,
    MAXAGE,
    MINNOPSG,
    MAXNOPSG,
    FLTSEGCNT,
    SPECIFIEDNODEC1,
    SPECIFIEDNODEC2,
    NODEC1,
    NODEC2,
    RULETARIFF,
    PERCENTNODEC,
    PERCENT,
    INHIBIT,
    UNAVAILTAG,
    NEGPSGSTATUSIND,
    PASSENGERIND,
    PSGLOC1TYPE,
    PSGID,
    WHOLLYWITHINLOCTYPE,
    LOC1TYPE,
    DISCOUNTIND,
    FAREIND,
    RESULTOWRT,
    RESULTSEASONTYPE,
    RESULTDOWTYPE,
    RESULTDISPLAYCATTYPE,
    RESULTPRICINGCATTYPE,
    TKTCODEMODIFIER,
    TKTDESIGNATORMODIFIER,
    OVRDCAT1,
    OVRDCAT2,
    OVRDCAT3,
    OVRDCAT4,
    OVRDCAT5,
    OVRDCAT6,
    OVRDCAT7,
    OVRDCAT8,
    OVRDCAT9,
    OVRDCAT10,
    OVRDCAT11,
    OVRDCAT12,
    OVRDCAT13,
    OVRDCAT14,
    OVRDCAT15,
    OVRDCAT16,
    OVRDCAT17,
    OVRDCAT18,
    OVRDCAT19,
    OVRDCAT20,
    OVRDCAT21,
    OVRDCAT22,
    OVRDCAT23,
    OVRDCAT24,
    OVRDCAT26,
    OVRDCAT27,
    OVRDCAT28,
    OVRDCAT29,
    OVRDCAT30,
    OVRDCAT31,
    OVRDCAT32,
    OVRDCAT33,
    OVRDCAT34,
    OVRDCAT35,
    OVRDCAT36,
    OVRDCAT37,
    OVRDCAT38,
    OVRDCAT39,
    OVRDCAT40,
    OVRDCAT41,
    OVRDCAT42,
    OVRDCAT43,
    OVRDCAT44,
    OVRDCAT45,
    OVRDCAT46,
    OVRDCAT47,
    OVRDCAT48,
    OVRDCAT49,
    OVRDCAT50,
    HIGHESTIND,
    PSGLOC1,
    WHOLLYWITHINLOC,
    TSI,
    LOC1,
    SPECIFIEDCUR1,
    SPECIFIEDCUR2,
    CUR1,
    CUR2,
    CARRIER,
    BASEFARECLASS,
    BASEFARETYPE,
    RESULTGLOBALDIR,
    RESULTROUTING,
    RESULTFARECLASS1,
    RESULTFARETYPE1,
    BOOKINGCODE1,
    BOOKINGCODE2,
    BOOKINGCODE3,
    BOOKINGCODE4,
    BOOKINGCODE5,
    BOOKINGCODE6,
    BOOKINGCODE7,
    BOOKINGCODE8,
    TKTCODE,
    TKTDESIGNATOR,
    PSGTYPE,
    OVERRIDEDATEITEMNO,
    SAMETARIFFRULE,
    PRIMESECTOR,
    RESULTROUTINGVENDOR,
    NUMBEROFCOLUMNS
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {

    this->Command(
        "select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,TEXTTBLITEMNO,BASETABLEITEMNO,"
        " MINMILEAGE,MAXMILEAGE,RESULTROUTINGTARIFF,RESULTMPM,BOOKINGCODETBLITEMNO,"
        " SPECIFIEDFAREAMT1,SPECIFIEDFAREAMT2,MINFAREAMT1,MAXFAREAMT1,MINFAREAMT2,"
        " MAXFAREAMT2,MINAGE,MAXAGE,MINNOPSG,MAXNOPSG,FLTSEGCNT,SPECIFIEDNODEC1,"
        " SPECIFIEDNODEC2,NODEC1,NODEC2,RULETARIFF,PERCENTNODEC,PERCENT,INHIBIT,"
        " UNAVAILTAG,NEGPSGSTATUSIND,PASSENGERIND,PSGLOC1TYPE,PSGID,WHOLLYWITHINLOCTYPE,"
        " LOC1TYPE,DISCOUNTIND,FAREIND,RESULTOWRT,RESULTSEASONTYPE,RESULTDOWTYPE,"
        " RESULTDISPLAYCATTYPE,RESULTPRICINGCATTYPE,TKTCODEMODIFIER,TKTDESIGNATORMODIFIER,"
        " OVRDCAT1,OVRDCAT2,OVRDCAT3,OVRDCAT4,OVRDCAT5,OVRDCAT6,OVRDCAT7,OVRDCAT8,OVRDCAT9,"
        " OVRDCAT10,OVRDCAT11,OVRDCAT12,OVRDCAT13,OVRDCAT14,OVRDCAT15,OVRDCAT16,OVRDCAT17,"
        " OVRDCAT18,OVRDCAT19,OVRDCAT20,OVRDCAT21,OVRDCAT22,OVRDCAT23,OVRDCAT24,OVRDCAT26,"
        " OVRDCAT27,OVRDCAT28,OVRDCAT29,OVRDCAT30,OVRDCAT31,OVRDCAT32,OVRDCAT33,OVRDCAT34,"
        " OVRDCAT35,OVRDCAT36,OVRDCAT37,OVRDCAT38,OVRDCAT39,OVRDCAT40,OVRDCAT41,OVRDCAT42,"
        " OVRDCAT43,OVRDCAT44,OVRDCAT45,OVRDCAT46,OVRDCAT47,OVRDCAT48,OVRDCAT49,OVRDCAT50,"
        " HIGHESTIND,PSGLOC1,WHOLLYWITHINLOC,TSI,LOC1,SPECIFIEDCUR1,SPECIFIEDCUR2,CUR1,"
        " CUR2,CARRIER,BASEFARECLASS,BASEFARETYPE,RESULTGLOBALDIR,RESULTROUTING,"
        " RESULTFARECLASS1,RESULTFARETYPE1,BOOKINGCODE1,BOOKINGCODE2,BOOKINGCODE3,"
        " BOOKINGCODE4,BOOKINGCODE5,BOOKINGCODE6,BOOKINGCODE7,BOOKINGCODE8,"
        " TKTCODE,TKTDESIGNATOR,PSGTYPE,OVERRIDEDATEITEMNO,SAMETARIFFRULE,PRIMESECTOR,"
        " RESULTROUTINGVENDOR");
    this->From("=FAREBYRULE");
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, CREATEDATE");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  };

  static tse::FareByRuleItemInfo* mapRowToFareByRuleItemInfo(Row* row)
  {
    tse::FareByRuleItemInfo* fbr = new tse::FareByRuleItemInfo;

    fbr->vendor() = row->getString(VENDOR);
    fbr->itemNo() = row->getInt(ITEMNO);
    fbr->createDate() = row->getDate(CREATEDATE);
    fbr->expireDate() = row->getDate(EXPIREDATE);
    fbr->textTblItemNo() = row->getInt(TEXTTBLITEMNO);
    fbr->baseTableItemNo() = row->getInt(BASETABLEITEMNO);
    fbr->minMileage() = row->getInt(MINMILEAGE);
    fbr->maxMileage() = row->getInt(MAXMILEAGE);
    fbr->resultRoutingTariff() = row->getInt(RESULTROUTINGTARIFF);
    fbr->resultmpm() = row->getInt(RESULTMPM);
    fbr->bookingCodeTblItemNo() = row->getInt(BOOKINGCODETBLITEMNO);

    fbr->specifiedNoDec1() = row->getInt(SPECIFIEDNODEC1);
    fbr->specifiedNoDec2() = row->getInt(SPECIFIEDNODEC2);
    fbr->specifiedFareAmt1() =
        QUERYCLASS::adjustDecimal(row->getInt(SPECIFIEDFAREAMT1), fbr->specifiedNoDec1());
    fbr->specifiedFareAmt2() =
        QUERYCLASS::adjustDecimal(row->getInt(SPECIFIEDFAREAMT2), fbr->specifiedNoDec2());

    fbr->noDec1() = row->getInt(NODEC1);
    fbr->noDec2() = row->getInt(NODEC2);
    fbr->minFareAmt1() = QUERYCLASS::adjustDecimal(row->getInt(MINFAREAMT1), fbr->noDec1());
    fbr->maxFareAmt1() = QUERYCLASS::adjustDecimal(row->getInt(MAXFAREAMT1), fbr->noDec1());
    fbr->minFareAmt2() = QUERYCLASS::adjustDecimal(row->getInt(MINFAREAMT2), fbr->noDec2());
    fbr->maxFareAmt2() = QUERYCLASS::adjustDecimal(row->getInt(MAXFAREAMT2), fbr->noDec2());

    fbr->minAge() = row->getInt(MINAGE);
    fbr->maxAge() = row->getInt(MAXAGE);
    fbr->minNoPsg() = row->getInt(MINNOPSG);
    fbr->maxNoPsg() = row->getInt(MAXNOPSG);
    fbr->fltSegCnt() = row->getInt(FLTSEGCNT);
    fbr->ruleTariff() = row->getInt(RULETARIFF);

    fbr->percentNoDec() = row->getInt(PERCENTNODEC);
    fbr->percent() = QUERYCLASS::adjustDecimal(row->getInt(PERCENT), fbr->percentNoDec());

    fbr->inhibit() = row->getChar(INHIBIT);
    fbr->unavailtag() = row->getChar(UNAVAILTAG);
    fbr->negPsgstatusInd() = row->getChar(NEGPSGSTATUSIND);
    fbr->passengerInd() = row->getChar(PASSENGERIND);
    fbr->psgid() = row->getChar(PSGID);
    fbr->discountInd() = row->getChar(DISCOUNTIND);
    fbr->fareInd() = row->getChar(FAREIND);
    fbr->resultowrt() = row->getChar(RESULTOWRT);
    fbr->resultseasonType() = row->getChar(RESULTSEASONTYPE);
    fbr->resultdowType() = row->getChar(RESULTDOWTYPE);
    fbr->resultDisplaycatType() = row->getChar(RESULTDISPLAYCATTYPE);
    fbr->resultpricingcatType() = row->getChar(RESULTPRICINGCATTYPE);
    fbr->tktCodeModifier() = row->getChar(TKTCODEMODIFIER);
    fbr->tktDesignatorModifier() = row->getChar(TKTDESIGNATORMODIFIER);
    fbr->ovrdcat1() = row->getChar(OVRDCAT1);
    fbr->ovrdcat2() = row->getChar(OVRDCAT2);
    fbr->ovrdcat3() = row->getChar(OVRDCAT3);
    fbr->ovrdcat4() = row->getChar(OVRDCAT4);
    fbr->ovrdcat5() = row->getChar(OVRDCAT5);
    fbr->ovrdcat6() = row->getChar(OVRDCAT6);
    fbr->ovrdcat7() = row->getChar(OVRDCAT7);
    fbr->ovrdcat8() = row->getChar(OVRDCAT8);
    fbr->ovrdcat9() = row->getChar(OVRDCAT9);
    fbr->ovrdcat10() = row->getChar(OVRDCAT10);
    fbr->ovrdcat11() = row->getChar(OVRDCAT11);
    fbr->ovrdcat12() = row->getChar(OVRDCAT12);
    fbr->ovrdcat13() = row->getChar(OVRDCAT13);
    fbr->ovrdcat14() = row->getChar(OVRDCAT14);
    fbr->ovrdcat15() = row->getChar(OVRDCAT15);
    fbr->ovrdcat16() = row->getChar(OVRDCAT16);
    fbr->ovrdcat17() = row->getChar(OVRDCAT17);
    fbr->ovrdcat18() = row->getChar(OVRDCAT18);
    fbr->ovrdcat19() = row->getChar(OVRDCAT19);
    fbr->ovrdcat20() = row->getChar(OVRDCAT20);
    fbr->ovrdcat21() = row->getChar(OVRDCAT21);
    fbr->ovrdcat22() = row->getChar(OVRDCAT22);
    fbr->ovrdcat23() = row->getChar(OVRDCAT23);
    fbr->ovrdcat24() = row->getChar(OVRDCAT24);
    fbr->ovrdcat26() = row->getChar(OVRDCAT26);
    fbr->ovrdcat27() = row->getChar(OVRDCAT27);
    fbr->ovrdcat28() = row->getChar(OVRDCAT28);
    fbr->ovrdcat29() = row->getChar(OVRDCAT29);
    fbr->ovrdcat30() = row->getChar(OVRDCAT30);
    fbr->ovrdcat31() = row->getChar(OVRDCAT31);
    fbr->ovrdcat32() = row->getChar(OVRDCAT32);
    fbr->ovrdcat33() = row->getChar(OVRDCAT33);
    fbr->ovrdcat34() = row->getChar(OVRDCAT34);
    fbr->ovrdcat35() = row->getChar(OVRDCAT35);
    fbr->ovrdcat36() = row->getChar(OVRDCAT36);
    fbr->ovrdcat37() = row->getChar(OVRDCAT37);
    fbr->ovrdcat38() = row->getChar(OVRDCAT38);
    fbr->ovrdcat39() = row->getChar(OVRDCAT39);
    fbr->ovrdcat40() = row->getChar(OVRDCAT40);
    fbr->ovrdcat41() = row->getChar(OVRDCAT41);
    fbr->ovrdcat42() = row->getChar(OVRDCAT42);
    fbr->ovrdcat43() = row->getChar(OVRDCAT43);
    fbr->ovrdcat44() = row->getChar(OVRDCAT44);
    fbr->ovrdcat45() = row->getChar(OVRDCAT45);
    fbr->ovrdcat46() = row->getChar(OVRDCAT46);
    fbr->ovrdcat47() = row->getChar(OVRDCAT47);
    fbr->ovrdcat48() = row->getChar(OVRDCAT48);
    fbr->ovrdcat49() = row->getChar(OVRDCAT49);
    fbr->ovrdcat50() = row->getChar(OVRDCAT50);
    fbr->highestInd() = row->getChar(HIGHESTIND);
    LocKey& psgLoc1 = fbr->psgLoc1();
    psgLoc1.loc() = row->getString(PSGLOC1);
    psgLoc1.locType() = row->getChar(PSGLOC1TYPE);

    LocKey& whollyWithinLoc = fbr->whollyWithinLoc();
    whollyWithinLoc.loc() = row->getString(WHOLLYWITHINLOC);
    whollyWithinLoc.locType() = row->getChar(WHOLLYWITHINLOCTYPE);

    fbr->tsi() = row->getInt(TSI);

    LocKey& loc1 = fbr->loc1();
    loc1.loc() = row->getString(LOC1);
    loc1.locType() = row->getChar(LOC1TYPE);

    fbr->specifiedCur1() = row->getString(SPECIFIEDCUR1);
    fbr->specifiedCur2() = row->getString(SPECIFIEDCUR2);
    fbr->cur1() = row->getString(CUR1);
    fbr->cur2() = row->getString(CUR2);
    fbr->carrier() = row->getString(CARRIER);
    fbr->baseFareClass() = row->getString(BASEFARECLASS);
    fbr->baseFareType() = row->getString(BASEFARETYPE);

    std::string gd = row->getString(RESULTGLOBALDIR);
    strToGlobalDirection(fbr->resultglobalDir(), gd);

    fbr->resultRouting() = row->getString(RESULTROUTING);
    fbr->resultFareClass1() = row->getString(RESULTFARECLASS1);
    fbr->resultFareType1() = row->getString(RESULTFARETYPE1);
    fbr->bookingCode1() = row->getString(BOOKINGCODE1);
    fbr->bookingCode2() = row->getString(BOOKINGCODE2);
    fbr->bookingCode3() = row->getString(BOOKINGCODE3);
    fbr->bookingCode4() = row->getString(BOOKINGCODE4);
    fbr->bookingCode5() = row->getString(BOOKINGCODE5);
    fbr->bookingCode6() = row->getString(BOOKINGCODE6);
    fbr->bookingCode7() = row->getString(BOOKINGCODE7);
    fbr->bookingCode8() = row->getString(BOOKINGCODE8);
    fbr->tktCode() = row->getString(TKTCODE);
    fbr->tktDesignator() = row->getString(TKTDESIGNATOR);
    fbr->paxType() = row->getString(PSGTYPE);
    fbr->overrideDateTblItemNo() = row->getInt(OVERRIDEDATEITEMNO);
    fbr->sameTariffRule() = row->getChar(SAMETARIFFRULE);
    fbr->primeSector() = row->getChar(PRIMESECTOR);
    if (!row->isNull(RESULTROUTINGVENDOR))
      fbr->resultRoutingVendor() = row->getString(RESULTROUTINGVENDOR);

    return fbr;
  };

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {};
}; // class QueryGetFBRItemSQLStatement

////////////////////////////////////////////////////////////////////////
// Template to adjust the SQL for QueryGetFBRItemHistorical
////////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetFBRItemHistoricalSQLStatement : public QueryGetFBRItemSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command(
        " select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,TEXTTBLITEMNO,BASETABLEITEMNO,"
        " MINMILEAGE,MAXMILEAGE,RESULTROUTINGTARIFF,RESULTMPM,BOOKINGCODETBLITEMNO,"
        " SPECIFIEDFAREAMT1,SPECIFIEDFAREAMT2,MINFAREAMT1,MAXFAREAMT1,MINFAREAMT2,"
        " MAXFAREAMT2,MINAGE,MAXAGE,MINNOPSG,MAXNOPSG,FLTSEGCNT,SPECIFIEDNODEC1,"
        " SPECIFIEDNODEC2,NODEC1,NODEC2,RULETARIFF,PERCENTNODEC,PERCENT,INHIBIT,"
        " UNAVAILTAG,NEGPSGSTATUSIND,PASSENGERIND,PSGLOC1TYPE,PSGID,WHOLLYWITHINLOCTYPE,"
        " LOC1TYPE,DISCOUNTIND,FAREIND,RESULTOWRT,RESULTSEASONTYPE,RESULTDOWTYPE,"
        " RESULTDISPLAYCATTYPE,RESULTPRICINGCATTYPE,TKTCODEMODIFIER,TKTDESIGNATORMODIFIER,"
        " OVRDCAT1,OVRDCAT2,OVRDCAT3,OVRDCAT4,OVRDCAT5,OVRDCAT6,OVRDCAT7,OVRDCAT8,OVRDCAT9,"
        " OVRDCAT10,OVRDCAT11,OVRDCAT12,OVRDCAT13,OVRDCAT14,OVRDCAT15,OVRDCAT16,OVRDCAT17,"
        " OVRDCAT18,OVRDCAT19,OVRDCAT20,OVRDCAT21,OVRDCAT22,OVRDCAT23,OVRDCAT24,OVRDCAT26,"
        " OVRDCAT27,OVRDCAT28,OVRDCAT29,OVRDCAT30,OVRDCAT31,OVRDCAT32,OVRDCAT33,OVRDCAT34,"
        " OVRDCAT35,OVRDCAT36,OVRDCAT37,OVRDCAT38,OVRDCAT39,OVRDCAT40,OVRDCAT41,OVRDCAT42,"
        " OVRDCAT43,OVRDCAT44,OVRDCAT45,OVRDCAT46,OVRDCAT47,OVRDCAT48,OVRDCAT49,OVRDCAT50,"
        " HIGHESTIND,PSGLOC1,WHOLLYWITHINLOC,TSI,LOC1,SPECIFIEDCUR1,SPECIFIEDCUR2,CUR1,"
        " CUR2,CARRIER,BASEFARECLASS,BASEFARETYPE,RESULTGLOBALDIR,RESULTROUTING,"
        " RESULTFARECLASS1,RESULTFARETYPE1,BOOKINGCODE1,BOOKINGCODE2,BOOKINGCODE3,"
        " BOOKINGCODE4,BOOKINGCODE5,BOOKINGCODE6,BOOKINGCODE7,BOOKINGCODE8,"
        " TKTCODE,TKTDESIGNATOR,PSGTYPE,OVERRIDEDATEITEMNO,SAMETARIFFRULE,PRIMESECTOR,"
        " RESULTROUTINGVENDOR");
    partialStatement.From("=FAREBYRULEH");
    partialStatement.Where("VENDOR = %1q"
                           "  and ITEMNO = %2n"
                           "  and VALIDITYIND = 'Y'"
                           "  and %3n <= EXPIREDATE"
                           "  and %4n >= CREATEDATE");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    partialStatement.Command(
        " union all"
        " select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,TEXTTBLITEMNO,BASETABLEITEMNO,"
        " MINMILEAGE,MAXMILEAGE,RESULTROUTINGTARIFF,RESULTMPM,BOOKINGCODETBLITEMNO,"
        " SPECIFIEDFAREAMT1,SPECIFIEDFAREAMT2,MINFAREAMT1,MAXFAREAMT1,MINFAREAMT2,"
        " MAXFAREAMT2,MINAGE,MAXAGE,MINNOPSG,MAXNOPSG,FLTSEGCNT,SPECIFIEDNODEC1,"
        " SPECIFIEDNODEC2,NODEC1,NODEC2,RULETARIFF,PERCENTNODEC,PERCENT,INHIBIT,"
        " UNAVAILTAG,NEGPSGSTATUSIND,PASSENGERIND,PSGLOC1TYPE,PSGID,WHOLLYWITHINLOCTYPE,"
        " LOC1TYPE,DISCOUNTIND,FAREIND,RESULTOWRT,RESULTSEASONTYPE,RESULTDOWTYPE,"
        " RESULTDISPLAYCATTYPE,RESULTPRICINGCATTYPE,TKTCODEMODIFIER,TKTDESIGNATORMODIFIER,"
        " OVRDCAT1,OVRDCAT2,OVRDCAT3,OVRDCAT4,OVRDCAT5,OVRDCAT6,OVRDCAT7,OVRDCAT8,OVRDCAT9,"
        " OVRDCAT10,OVRDCAT11,OVRDCAT12,OVRDCAT13,OVRDCAT14,OVRDCAT15,OVRDCAT16,OVRDCAT17,"
        " OVRDCAT18,OVRDCAT19,OVRDCAT20,OVRDCAT21,OVRDCAT22,OVRDCAT23,OVRDCAT24,OVRDCAT26,"
        " OVRDCAT27,OVRDCAT28,OVRDCAT29,OVRDCAT30,OVRDCAT31,OVRDCAT32,OVRDCAT33,OVRDCAT34,"
        " OVRDCAT35,OVRDCAT36,OVRDCAT37,OVRDCAT38,OVRDCAT39,OVRDCAT40,OVRDCAT41,OVRDCAT42,"
        " OVRDCAT43,OVRDCAT44,OVRDCAT45,OVRDCAT46,OVRDCAT47,OVRDCAT48,OVRDCAT49,OVRDCAT50,"
        " HIGHESTIND,PSGLOC1,WHOLLYWITHINLOC,TSI,LOC1,SPECIFIEDCUR1,SPECIFIEDCUR2,CUR1,"
        " CUR2,CARRIER,BASEFARECLASS,BASEFARETYPE,RESULTGLOBALDIR,RESULTROUTING,"
        " RESULTFARECLASS1,RESULTFARETYPE1,BOOKINGCODE1,BOOKINGCODE2,BOOKINGCODE3,"
        " BOOKINGCODE4,BOOKINGCODE5,BOOKINGCODE6,BOOKINGCODE7,BOOKINGCODE8,"
        " TKTCODE,TKTDESIGNATOR,PSGTYPE,OVERRIDEDATEITEMNO,SAMETARIFFRULE,PRIMESECTOR,"
        " RESULTROUTINGVENDOR");
    partialStatement.From("=FAREBYRULE");
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
      this->OrderBy("VENDOR, ITEMNO, CREATEDATE");
  }

  //  override this version to replace parts of the compound statement
  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}

}; // class QueryGetFBRItemHistoricalSQLStatement
} // tse
