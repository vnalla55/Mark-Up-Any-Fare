// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetTaxRulesRecord.h"
#include "DBAccess/SQLStatement.h"
#include "DBAccess/TaxRulesRecord.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetTaxRulesRecordSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetTaxRulesRecordSQLStatement() {};

  virtual ~QueryGetTaxRulesRecordSQLStatement() {};

  enum ColumnIndexes
  {
    VENDOR = 0,
    NATION,
    TAXCODE,
    TAXREMITTANCEID,
    TAXTYPE,
    TAXPOINTTAG,
    PERCENTFLATTAG,
    TAXCARRIER,
    TICKETEDPOINTTAG,
    CALCORDER,
    SEQNO,
    CREATEDATE,
    VERSIONDATE,
    EXPIREDATE,
    LOCKDATE,
    EFFDATE,
    DISCDATE,
    HISTORICSALEEFFDATE,
    HISTORICSALEDISCDATE,
    HISTORICTRVLEFFDATE,
    HISTORICTRVLDISCDATE,
    TAXABLEUNITTAG1,
    TAXABLEUNITTAG2,
    TAXABLEUNITTAG3,
    TAXABLEUNITTAG4,
    TAXABLEUNITTAG5,
    TAXABLEUNITTAG6,
    TAXABLEUNITTAG7,
    TAXABLEUNITTAG8,
    TAXABLEUNITTAG9,
    TAXABLEUNITTAG10,
    TRAVELDATEAPPTAG,
    TVLFIRSTYEAR,
    TVLLASTYEAR,
    TVLFIRSTMONTH,
    TVLLASTMONTH,
    TVLFIRSTDAY,
    TVLLASTDAY,
    POSLOCTYPE,
    POSLOC,
    POSLOCZONETBLNO,
    SVCFEESSECURITYITEMNO,
    POTKTLOCTYPE,
    POTKTLOC,
    POTKTLOCZONETBLNO,
    PODELIVERYLOCTYPE,
    PODELIVERYLOC,
    PODELIVERYZONETBLNO,
    RTNTOORIG,
    JRNYIND,
    JRNYLOC1TYPE,
    JRNYLOC1,
    JRNYLOC1ZONETBLNO,
    JRNYLOC2TYPE,
    JRNYLOC2,
    JRNYLOC2ZONETBLNO,
    TRVLWHOLLYWITHINLOCTYPE,
    TRVLWHOLLYWITHINLOC,
    TRVLWHOLLYWITHINLOCZONETBLNO,
    JRNYINCLUDESLOCTYPE,
    JRNYINCLUDESLOC,
    JRNYINCLUDESLOCZONETBLNO,
    TAXPOINTLOC1INTDOMIND,
    TAXPOINTLOC1TRNSFRTYPE,
    TAXPOINTLOC1STOPOVERTAG,
    TAXPOINTLOC1TYPE,
    TAXPOINTLOC1,
    TAXPOINTLOC1ZONETBLNO,
    TAXPOINTLOC2INTLDOMIND,
    TAXPOINTLOC2COMPARE,
    TAXPOINTLOC2STOPOVERTAG,
    TAXPOINTLOC2TYPE,
    TAXPOINTLOC2,
    TAXPOINTLOC2ZONETBLNO,
    TAXPOINTLOC3GEOTYPE,
    TAXPOINTLOC3TYPE,
    TAXPOINTLOC3,
    TAXPOINTLOC3ZONETBLNO,
    STOPOVERTIMETAG,
    STOPOVERTIMEUNIT,
    CONNECTIONSTAG1,
    CONNECTIONSTAG2,
    CONNECTIONSTAG3,
    CONNECTIONSTAG4,
    CONNECTIONSTAG5,
    CONNECTIONSTAG6,
    CONNECTIONSTAG7,
    CURRENCYOFSALE,
    TAXCURRENCY,
    TAXCURDECIMALS,
    TAXAMT,
    TAXPERCENT,
    TAXROUNDUNIT,
    TAXROUNDDIR,
    MINTAX,
    MAXTAX,
    MINMAXCURRENCY,
    MINMAXDECIMALS,
    TKTVALAPPLQUALIFIER,
    TKTVALMIN,
    TKTVALMAX,
    TKTVALCURRENCY,
    TKTVALCURDECIMALS,
    CARRIERAPPLITEMNO1,
    CARRIERFLTITEMNO1,
    CARRIERFLTITEMNO2,
    PSGRTYPECODEITEMNO,
    SERVICEBAGGAGEAPPLTAG,
    SERVICEBAGGAGEITEMNO,
    SECTORDETAILAPPLTAG,
    SECTORDETAILITEMNO,
    TAXTEXTTBLITEMNO,
    TAXAPPLIMIT,
    NETREMITAPPLTAG,
    TAXAPPLIESTOTAGIND,
    ALTERNATERULEREFTAG,
    TAXPROCESSINGAPPLTAG,
    TAXMATCHINGAPPLTAG,
    OUTPUTTYPEIND,
    PAIDBY3RDPARTYTAG,
    RATDDATE,
    VATINCLUSIVEIND,
    EXEMPTTAG,
    NUMBEROFCOLUMNS
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select VENDOR,NATION,TAXCODE,TAXREMITTANCEID,TAXTYPE,TAXPOINTTAG,PERCENTFLATTAG,"
        "TAXCARRIER,TICKETEDPOINTTAG,CALCORDER,SEQNO,CREATEDATE,VERSIONDATE,EXPIREDATE,LOCKDATE,"
        "EFFDATE,"
        "DISCDATE,HISTORICSALEEFFDATE,HISTORICSALEDISCDATE,HISTORICTRVLEFFDATE,"
        "HISTORICTRVLDISCDATE,"
        "TAXABLEUNITTAG1,TAXABLEUNITTAG2,TAXABLEUNITTAG3,TAXABLEUNITTAG4,TAXABLEUNITTAG5,"
        "TAXABLEUNITTAG6,TAXABLEUNITTAG7,TAXABLEUNITTAG8,TAXABLEUNITTAG9,TAXABLEUNITTAG10,"
        "TRAVELDATEAPPTAG,TVLFIRSTYEAR,TVLLASTYEAR,TVLFIRSTMONTH,TVLLASTMONTH,TVLFIRSTDAY,"
        "TVLLASTDAY,"
        "POSLOCTYPE,POSLOC,POSLOCZONETBLNO,SVCFEESSECURITYITEMNO,POTKTLOCTYPE,POTKTLOC,"
        "POTKTLOCZONETBLNO,"
        "PODELIVERYLOCTYPE,PODELIVERYLOC,PODELIVERYZONETBLNO,"
        "RTNTOORIG,JRNYIND,JRNYLOC1TYPE,JRNYLOC1,JRNYLOC1ZONETBLNO,JRNYLOC2TYPE,JRNYLOC2,"
        "JRNYLOC2ZONETBLNO,TRVLWHOLLYWITHINLOCTYPE,TRVLWHOLLYWITHINLOC,"
        "TRVLWHOLLYWITHINLOCZONETBLNO,"
        "JRNYINCLUDESLOCTYPE,JRNYINCLUDESLOC,JRNYINCLUDESLOCZONETBLNO,"
        "TAXPOINTLOC1INTDOMIND,TAXPOINTLOC1TRNSFRTYPE,TAXPOINTLOC1STOPOVERTAG,TAXPOINTLOC1TYPE,"
        "TAXPOINTLOC1,TAXPOINTLOC1ZONETBLNO,"
        "TAXPOINTLOC2INTLDOMIND,TAXPOINTLOC2COMPARE,TAXPOINTLOC2STOPOVERTAG,TAXPOINTLOC2TYPE,"
        "TAXPOINTLOC2,TAXPOINTLOC2ZONETBLNO,"
        "TAXPOINTLOC3GEOTYPE,TAXPOINTLOC3TYPE,TAXPOINTLOC3,TAXPOINTLOC3ZONETBLNO,"
        "STOPOVERTIMETAG,STOPOVERTIMEUNIT,"
        "CONNECTIONSTAG1,CONNECTIONSTAG2,CONNECTIONSTAG3,CONNECTIONSTAG4,CONNECTIONSTAG5,"
        "CONNECTIONSTAG6,"
        "CONNECTIONSTAG7,CURRENCYOFSALE,TAXCURRENCY,TAXCURDECIMALS,TAXAMT,TAXPERCENT,TAXROUNDUNIT,"
        "TAXROUNDDIR,"
        "MINTAX,MAXTAX,MINMAXCURRENCY,MINMAXDECIMALS,TKTVALAPPLQUALIFIER,TKTVALMIN,TKTVALMAX,"
        "TKTVALCURRENCY,"
        "TKTVALCURDECIMALS,CARRIERAPPLITEMNO1,CARRIERFLTITEMNO1,CARRIERFLTITEMNO2,"
        "PSGRTYPECODEITEMNO,"
        "SERVICEBAGGAGEAPPLTAG,SERVICEBAGGAGEITEMNO,SECTORDETAILAPPLTAG,SECTORDETAILITEMNO,"
        "TAXTEXTTBLITEMNO,"
        "TAXAPPLIMIT,NETREMITAPPLTAG,TAXAPPLIESTOTAGIND,ALTERNATERULEREFTAG,TAXPROCESSINGAPPLTAG,"
        "TAXMATCHINGAPPLTAG,OUTPUTTYPEIND,PAIDBY3RDPARTYTAG,RATDDATE,VATINCLUSIVEIND,EXEMPTTAG");

    this->From("=TAXRULESRECORD");
    this->Where("NATION = %1q"
                " and TAXPOINTTAG = %2q"
                " and %cd <= EXPIREDATE");
    this->OrderBy("NATION, TAXCODE, TAXTYPE, SEQNO, CREATEDATE");

    adjustBaseSQL();
    this->ConstructSQL();
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static DateTime readNullableDate(Row* row, int columnIndex)
  {
    if (row->isNull(columnIndex))
      return DateTime::emptyDate();
    else
      return row->getDate(columnIndex);
  }

  static TaxRulesRecord* mapRowToRulesRecord(Row* row)
  {
    TaxRulesRecord* rulesRecord = new TaxRulesRecord;

    rulesRecord->vendor() = row->getString(VENDOR);
    rulesRecord->nation() = row->getString(NATION);
    rulesRecord->taxCode() = row->getString(TAXCODE);
    rulesRecord->taxRemittanceId() = row->getChar(TAXREMITTANCEID);
    rulesRecord->taxType() = row->getString(TAXTYPE);
    rulesRecord->taxPointTag() = row->getChar(TAXPOINTTAG);
    rulesRecord->percentFlatTag() = row->getChar(PERCENTFLATTAG);
    rulesRecord->taxCarrier() = row->getString(TAXCARRIER);
    rulesRecord->ticketedPointTag() = row->getChar(TICKETEDPOINTTAG);
    rulesRecord->calcOrder() = static_cast<uint16_t>(row->getChar(CALCORDER) - '0');
    rulesRecord->seqNo() = row->getLong(SEQNO);

    rulesRecord->createDate() = row->getDate(CREATEDATE);
    rulesRecord->versionDate() = row->getDate(VERSIONDATE).get64BitRep();
    rulesRecord->expireDate() = row->getDate(EXPIREDATE);
    rulesRecord->lockDate() = row->getDate(LOCKDATE);
    rulesRecord->effDate() = row->getDate(EFFDATE);
    rulesRecord->discDate() = row->getDate(DISCDATE);

    rulesRecord->historicSaleEffDate() = readNullableDate(row, HISTORICSALEEFFDATE);
    rulesRecord->historicSaleDiscDate() = readNullableDate(row, HISTORICSALEDISCDATE);
    rulesRecord->historicTrvlEffDate() = readNullableDate(row, HISTORICTRVLEFFDATE);
    rulesRecord->historicTrvlDiscDate() = readNullableDate(row, HISTORICTRVLDISCDATE);

    rulesRecord->taxableUnitTag1() = row->getChar(TAXABLEUNITTAG1);
    rulesRecord->taxableUnitTag2() = row->getChar(TAXABLEUNITTAG2);
    rulesRecord->taxableUnitTag3() = row->getChar(TAXABLEUNITTAG3);
    rulesRecord->taxableUnitTag4() = row->getChar(TAXABLEUNITTAG4);
    rulesRecord->taxableUnitTag5() = row->getChar(TAXABLEUNITTAG5);
    rulesRecord->taxableUnitTag6() = row->getChar(TAXABLEUNITTAG6);
    rulesRecord->taxableUnitTag7() = row->getChar(TAXABLEUNITTAG7);
    rulesRecord->taxableUnitTag8() = row->getChar(TAXABLEUNITTAG8);
    rulesRecord->taxableUnitTag9() = row->getChar(TAXABLEUNITTAG9);
    rulesRecord->taxableUnitTag10() = row->getChar(TAXABLEUNITTAG10);

    rulesRecord->travelDateAppTag() = row->getChar(TRAVELDATEAPPTAG);
    rulesRecord->tvlFirstYear() = static_cast<uint16_t>(row->getInt(TVLFIRSTYEAR));
    rulesRecord->tvlLastYear() = static_cast<uint16_t>(row->getInt(TVLLASTYEAR));
    rulesRecord->tvlFirstMonth() = static_cast<uint16_t>(row->getInt(TVLFIRSTMONTH));
    rulesRecord->tvlLastMonth() = static_cast<uint16_t>(row->getInt(TVLLASTMONTH));
    rulesRecord->tvlFirstDay() = static_cast<uint16_t>(row->getInt(TVLFIRSTDAY));
    rulesRecord->tvlLastDay() = static_cast<uint16_t>(row->getInt(TVLLASTDAY));

    rulesRecord->posLocType() = row->getChar(POSLOCTYPE);
    rulesRecord->posLoc() = row->getString(POSLOC);
    rulesRecord->posLocZoneTblNo() = row->getInt(POSLOCZONETBLNO);
    rulesRecord->svcFeesSecurityItemNo() = row->getInt(SVCFEESSECURITYITEMNO);

    rulesRecord->poTktLocType() = row->getChar(POTKTLOCTYPE);
    rulesRecord->poTktLoc() = row->getString(POTKTLOC);
    rulesRecord->poTktLocZoneTblNo() = row->getInt(POTKTLOCZONETBLNO);
    rulesRecord->poDeliveryLocType() = row->getChar(PODELIVERYLOCTYPE);
    rulesRecord->poDeliveryLoc() = row->getString(PODELIVERYLOC);
    rulesRecord->poDeliveryZoneTblNo() = row->getInt(PODELIVERYZONETBLNO);

    rulesRecord->rtnToOrig() = row->getChar(RTNTOORIG);

    rulesRecord->jrnyInd() = row->getChar(JRNYIND);
    rulesRecord->jrnyLoc1Type() = row->getChar(JRNYLOC1TYPE);
    rulesRecord->jrnyLoc1() = row->getString(JRNYLOC1);
    rulesRecord->jrnyLoc1ZoneTblNo() = row->getInt(JRNYLOC1ZONETBLNO);

    rulesRecord->jrnyLoc2Type() = row->getChar(JRNYLOC2TYPE);
    rulesRecord->jrnyLoc2() = row->getString(JRNYLOC2);
    rulesRecord->jrnyLoc2ZoneTblNo() = row->getInt(JRNYLOC2ZONETBLNO);

    rulesRecord->trvlWhollyWithinLocType() = row->getChar(TRVLWHOLLYWITHINLOCTYPE);
    rulesRecord->trvlWhollyWithinLoc() = row->getString(TRVLWHOLLYWITHINLOC);
    rulesRecord->trvlWhollyWithinLocZoneTblNo() = row->getInt(TRVLWHOLLYWITHINLOCZONETBLNO);

    rulesRecord->jrnyIncludesLocType() = row->getChar(JRNYINCLUDESLOCTYPE);
    rulesRecord->jrnyIncludesLoc() = row->getString(JRNYINCLUDESLOC);
    rulesRecord->jrnyIncludesLocZoneTblNo() = row->getInt(JRNYINCLUDESLOCZONETBLNO);

    rulesRecord->taxPointLoc1IntDomInd() = row->getChar(TAXPOINTLOC1INTDOMIND);
    rulesRecord->taxPointLoc1TrnsfrType() = row->getChar(TAXPOINTLOC1TRNSFRTYPE);
    rulesRecord->taxPointLoc1StopoverTag() = row->getChar(TAXPOINTLOC1STOPOVERTAG);
    rulesRecord->taxPointLoc1Type() = row->getChar(TAXPOINTLOC1TYPE);
    rulesRecord->taxPointLoc1() = row->getString(TAXPOINTLOC1);
    rulesRecord->taxPointLoc1ZoneTblNo() = row->getInt(TAXPOINTLOC1ZONETBLNO);

    rulesRecord->taxPointLoc2IntlDomInd() = row->getChar(TAXPOINTLOC2INTLDOMIND);
    rulesRecord->taxPointLoc2Compare() = row->getChar(TAXPOINTLOC2COMPARE);
    rulesRecord->taxPointLoc2StopoverTag() = row->getChar(TAXPOINTLOC2STOPOVERTAG);
    rulesRecord->taxPointLoc2Type() = row->getChar(TAXPOINTLOC2TYPE);
    rulesRecord->taxPointLoc2() = row->getString(TAXPOINTLOC2);
    rulesRecord->taxPointLoc2ZoneTblNo() = row->getInt(TAXPOINTLOC2ZONETBLNO);

    rulesRecord->taxPointLoc3GeoType() = row->getChar(TAXPOINTLOC3GEOTYPE);
    rulesRecord->taxPointLoc3Type() = row->getChar(TAXPOINTLOC3TYPE);
    rulesRecord->taxPointLoc3() = row->getString(TAXPOINTLOC3);
    rulesRecord->taxPointLoc3ZoneTblNo() = row->getInt(TAXPOINTLOC3ZONETBLNO);

    rulesRecord->stopoverTimeTag() = row->getString(STOPOVERTIMETAG);
    rulesRecord->stopoverTimeUnit() = row->getChar(STOPOVERTIMEUNIT);

    rulesRecord->connectionsTag1() = row->getChar(CONNECTIONSTAG1);
    rulesRecord->connectionsTag2() = row->getChar(CONNECTIONSTAG2);
    rulesRecord->connectionsTag3() = row->getChar(CONNECTIONSTAG3);
    rulesRecord->connectionsTag4() = row->getChar(CONNECTIONSTAG4);
    rulesRecord->connectionsTag5() = row->getChar(CONNECTIONSTAG5);
    rulesRecord->connectionsTag6() = row->getChar(CONNECTIONSTAG6);
    rulesRecord->connectionsTag7() = row->getChar(CONNECTIONSTAG7);

    rulesRecord->currencyOfSale() = row->getString(CURRENCYOFSALE);
    rulesRecord->taxCurrency() = row->getString(TAXCURRENCY);
    rulesRecord->taxCurDecimals() = row->getInt(TAXCURDECIMALS);
    rulesRecord->taxAmt() = static_cast<uint32_t>(row->getLong(TAXAMT));
    rulesRecord->taxPercent() = row->getInt(TAXPERCENT);
    rulesRecord->taxRoundUnit() = row->getChar(TAXROUNDUNIT);
    rulesRecord->taxRoundDir() = row->getChar(TAXROUNDDIR);

    rulesRecord->minTax() = static_cast<uint32_t>(row->getLong(MINTAX));
    rulesRecord->maxTax() = static_cast<uint32_t>(row->getLong(MAXTAX));
    rulesRecord->minMaxCurrency() = row->getString(MINMAXCURRENCY);
    rulesRecord->minMaxDecimals() = row->getInt(MINMAXDECIMALS);

    rulesRecord->tktValApplQualifier() = row->getChar(TKTVALAPPLQUALIFIER);
    rulesRecord->tktValMin() = static_cast<uint32_t>(row->getLong(TKTVALMIN));
    rulesRecord->tktValMax() = static_cast<uint32_t>(row->getLong(TKTVALMAX));
    rulesRecord->tktValCurrency() = row->getString(TKTVALCURRENCY);
    rulesRecord->tktValCurDecimals() = row->getInt(TKTVALCURDECIMALS);

    rulesRecord->carrierApplItemNo1() = row->getInt(CARRIERAPPLITEMNO1);
    rulesRecord->carrierFltItemNo1() = row->getInt(CARRIERFLTITEMNO1);
    rulesRecord->carrierFltItemNo2() = row->getInt(CARRIERFLTITEMNO2);
    rulesRecord->psgrTypeCodeItemNo() = row->getInt(PSGRTYPECODEITEMNO);
    rulesRecord->serviceBaggageApplTag() = row->getChar(SERVICEBAGGAGEAPPLTAG);
    rulesRecord->serviceBaggageItemNo() = row->getInt(SERVICEBAGGAGEITEMNO);
    rulesRecord->sectorDetailApplTag() = row->getChar(SECTORDETAILAPPLTAG);
    rulesRecord->sectorDetailItemNo() = row->getInt(SECTORDETAILITEMNO);
    rulesRecord->taxTextTblItemNo() = row->getInt(TAXTEXTTBLITEMNO);

    rulesRecord->taxAppLimit() = row->getChar(TAXAPPLIMIT);
    rulesRecord->netRemitApplTag() = row->getChar(NETREMITAPPLTAG);
    rulesRecord->taxAppliesToTagInd() = row->getChar(TAXAPPLIESTOTAGIND);

    rulesRecord->alternateRuleRefTag() = row->getInt(ALTERNATERULEREFTAG);
    rulesRecord->taxProcessingApplTag() = row->getString(TAXPROCESSINGAPPLTAG);
    rulesRecord->taxMatchingApplTag() = row->getString(TAXMATCHINGAPPLTAG);

    rulesRecord->outputTypeInd() = row->getChar(OUTPUTTYPEIND);
    rulesRecord->paidBy3rdPartyTag() = row->getChar(PAIDBY3RDPARTYTAG);
    rulesRecord->ratdDate() = row->getInt(RATDDATE);
    rulesRecord->vatInclusiveInd() = row->getChar(VATINCLUSIVEIND);

    rulesRecord->exemptTag() = row->getChar(EXEMPTTAG);

    return rulesRecord;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
}; // class QueryGetTaxRulesRecordSQLStatement

template <class QUERYCLASS>
class QueryGetTaxRulesRecordHistoricalSQLStatement
    : public QueryGetTaxRulesRecordSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("NATION = %1q"
                " and TAXPOINTTAG = %2q"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
    this->OrderBy("NATION, TAXCODE, TAXTYPE, SEQNO, CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class QueryGetTaxRulesRecordHistoricalSQLStatement

template <class QUERYCLASS>
class QueryGetTaxRulesRecordByCodeSQLStatement
    : public QueryGetTaxRulesRecordSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("TAXCODE = %1q"
                " and %cd <= EXPIREDATE");
    this->OrderBy("NATION, SEQNO, CREATEDATE");
  }

}; // class QueryGetTaxRulesRecordByCodeSQLStatement

template <class QUERYCLASS>
class QueryGetTaxRulesRecordByCodeHistoricalSQLStatement
    : public QueryGetTaxRulesRecordSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("TAXCODE = %1q"
                " and %2n <= EXPIREDATE"
                " and %3n >= CREATEDATE");
    this->OrderBy("NATION, SEQNO, CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class QueryGetTaxRulesRecordByCodeHistoricalSQLStatement

// DEPRECATED
// Remove with ATPCO_TAX_X1byCodeDAORefactor fallback removal
template <class QUERYCLASS>
class QueryGetTaxRulesRecordByCodeAndTypeSQLStatement
    : public QueryGetTaxRulesRecordSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("TAXCODE = %1q"
                " and TAXTYPE = %2q"
                " and %cd <= EXPIREDATE");
    this->OrderBy("NATION, SEQNO, CREATEDATE");
  }

}; // class

// DEPRECATED
// Remove with ATPCO_TAX_X1byCodeDAORefactor fallback removal
template <class QUERYCLASS>
class QueryGetTaxRulesRecordByCodeAndTypeHistoricalSQLStatement
    : public QueryGetTaxRulesRecordSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("TAXCODE = %1q"
                " and TAXTYPE = %2q"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
    this->OrderBy("NATION, SEQNO, CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
}; // class

template <class QUERYCLASS>
class QueryGetAllTaxRulesRecordSQLStatement : public QueryGetTaxRulesRecordSQLStatement<QUERYCLASS>
{

private:
  void adjustBaseSQL() override
  {
    this->Where("%cd <= EXPIREDATE");
    this->OrderBy("NATION, TAXCODE, TAXTYPE, SEQNO, CREATEDATE");
  }
}; // QueryGetAllTaxRulesRecordSQLStatement

} // tse

