//-----------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly
// prohibited.
//-----------------------------------------------------------------------------

#pragma once

#include "DBAccess/DBResultSet.h"
#include "DBAccess/Queries/QueryGetVoluntaryRefundsInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{

class Row;

template <class QUERYCLASS>
class QueryGetVoluntaryRefundsInfoSQLStatement : public DBAccess::SQLStatement
{
public:
  enum ColumnIndexes
  {
    VENDOR = 0,
    ITEMNO,
    CREATEDATE,
    EXPIREDATE,
    VALIDITYIND,
    INHIBIT,
    PSGTYPE,
    WAIVERTBLITEMNO,
    TKTVALIDITYIND,
    TKTVALIDITYPERIOD,
    TKTVALIDITYUNIT,
    DEPOFJOURNEYIND,
    PUIND,
    FARECOMPONENTIND,
    ADVCANCELFROMTO,
    ADVCANCELLASTTOD,
    ADVCANCELPERIOD,
    ADVCANCELUNIT,
    TKTTIMELIMIT,
    ADVRESVALIDATION,
    FULLYFLOWN,
    ORIGSCHEDFLT,
    ORIGSCHEDFLTPERIOD,
    ORIGSCHEDFLTUNIT,
    CANCELLATIONIND,
    FAREBREAKPOINTS,
    REPRICEIND,
    RULETARIFF,
    RULETARIFFIND,
    RULE,
    FARECLASSIND,
    FARECLASS,
    FARETYPETBLITEMNO,
    SAMEFAREIND,
    NMLSPECIALIND,
    OWRT,
    FAREAMOUNTIND,
    BOOKINGCODEIND,
    PENALTYAMT1,
    PENALTYCUR1,
    PENALTYNODEC1,
    PENALTYAMT2,
    PENALTYCUR2,
    PENALTYNODEC2,
    PENALTYPERCENT,
    PENALTYPERCENTNODEC,
    HIGHLOWIND,
    MINIMUMAMT,
    MINIMUMAMTCUR,
    MINIMUMAMTNODEC,
    REISSUEFEEIND,
    CALCOPTION,
    DISCOUNTTAG1,
    DISCOUNTTAG2,
    DISCOUNTTAG3,
    DISCOUNTTAG4,
    FORMOFREFUND,
    TAXNONREFUNDABLEIND,
    CUSTOMER1PERIOD,
    CUSTOMER1UNIT,
    CUSTOMER1RESTKT,
    CARRIERAPPLTBLITEMNO,
    OVERRIDEDATETBLITEMNO,
    TEXTTBLITEMNO,
    UNAVAILTAG
  };

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command("select VENDOR,ITEMNO,CREATEDATE,EXPIREDATE,VALIDITYIND,INHIBIT,PSGTYPE,"
                  "WAIVERTBLITEMNO,TKTVALIDITYIND,TKTVALIDITYPERIOD,TKTVALIDITYUNIT,"
                  "DEPOFJOURNEYIND,PUIND,FARECOMPONENTIND,ADVCANCELFROMTO,ADVCANCELLASTTOD,"
                  "ADVCANCELPERIOD,ADVCANCELUNIT,TKTTIMELIMIT,ADVRESVALIDATION,"
                  "FULLYFLOWN,ORIGSCHEDFLT,ORIGSCHEDFLTPERIOD,ORIGSCHEDFLTUNIT,"
                  "CANCELLATIONIND,FAREBREAKPOINTS,REPRICEIND,RULETARIFF,RULETARIFFIND,"
                  "RULE,FARECLASSIND,FARECLASS,FARETYPETBLITEMNO,SAMEFAREIND,NMLSPECIALIND,"
                  "OWRT,FAREAMOUNTIND,BOOKINGCODEIND,PENALTYAMT1,PENALTYCUR1,PENALTYNODEC1,"
                  "PENALTYAMT2,PENALTYCUR2,PENALTYNODEC2,PENALTYPERCENT,PENALTYPERCENTNODEC,"
                  "HIGHLOWIND,MINIMUMAMT,MINIMUMAMTCUR,MINIMUMAMTNODEC,REISSUEFEEIND,"
                  "CALCOPTION,DISCOUNTTAG1,DISCOUNTTAG2,DISCOUNTTAG3,DISCOUNTTAG4,FORMOFREFUND,"
                  "TAXNONREFUNDABLEIND,CUSTOMER1PERIOD,CUSTOMER1UNIT,CUSTOMER1RESTKT,"
                  "CARRIERAPPLTBLITEMNO,OVERRIDEDATETBLITEMNO,TEXTTBLITEMNO,UNAVAILTAG ");
    this->From(" =VOLUNTARYREFUNDS");
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %cd <= EXPIREDATE");
    if (DataManager::forceSortOrder())
      this->OrderBy("VENDOR, ITEMNO, CREATEDATE");

    adjustBaseSQL();

    ConstructSQL();

    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static VoluntaryRefundsInfo* map(Row* row)
  {
    VoluntaryRefundsInfo* vr = new VoluntaryRefundsInfo;
    vr->vendor() = row->getString(VENDOR);
    vr->itemNo() = row->getInt(ITEMNO);
    vr->createDate() = row->getDate(CREATEDATE);
    vr->expireDate() = row->getDate(EXPIREDATE);
    vr->validityInd() = row->getChar(VALIDITYIND);
    vr->inhibit() = row->getChar(INHIBIT);
    vr->psgType() = row->getString(PSGTYPE);
    vr->waiverTblItemNo() = row->getInt(WAIVERTBLITEMNO);
    vr->tktValidityInd() = row->getChar(TKTVALIDITYIND);
    vr->tktValidityPeriod() = row->getString(TKTVALIDITYPERIOD);
    vr->tktValidityUnit() = row->getString(TKTVALIDITYUNIT);
    vr->depOfJourneyInd() = row->getChar(DEPOFJOURNEYIND);
    vr->puInd() = row->getChar(PUIND);
    vr->fareComponentInd() = row->getChar(FARECOMPONENTIND);
    vr->advCancelFromTo() = row->getChar(ADVCANCELFROMTO);
    vr->advCancelLastTod() = row->getInt(ADVCANCELLASTTOD);
    vr->advCancelPeriod() = row->getString(ADVCANCELPERIOD);
    vr->advCancelUnit() = row->getString(ADVCANCELUNIT);
    vr->tktTimeLimit() = row->getChar(TKTTIMELIMIT);
    vr->advResValidation() = row->getChar(ADVRESVALIDATION);
    vr->fullyFlown() = row->getChar(FULLYFLOWN);
    vr->origSchedFlt() = row->getChar(ORIGSCHEDFLT);
    vr->origSchedFltPeriod() = row->getString(ORIGSCHEDFLTPERIOD);
    vr->origSchedFltUnit() = row->getString(ORIGSCHEDFLTUNIT);
    vr->cancellationInd() = row->getChar(CANCELLATIONIND);
    vr->fareBreakpoints() = row->getChar(FAREBREAKPOINTS);
    vr->repriceInd() = row->getChar(REPRICEIND);
    vr->ruleTariff() = row->getInt(RULETARIFF);
    vr->ruleTariffInd() = row->getChar(RULETARIFFIND);
    vr->rule() = row->getString(RULE);
    vr->fareClassInd() = row->getChar(FARECLASSIND);
    vr->fareClass() = row->getString(FARECLASS);
    vr->fareTypeTblItemNo() = row->getInt(FARETYPETBLITEMNO);
    vr->sameFareInd() = row->getChar(SAMEFAREIND);
    vr->nmlSpecialInd() = row->getChar(NMLSPECIALIND);
    vr->owrt() = row->getChar(OWRT);
    vr->fareAmountInd() = row->getChar(FAREAMOUNTIND);
    vr->bookingCodeInd() = row->getChar(BOOKINGCODEIND);
    vr->penalty1NoDec() = row->getInt(PENALTYNODEC1);
    vr->penalty1Amt() = QUERYCLASS::adjustDecimal(row->getInt(PENALTYAMT1), vr->penalty1NoDec());
    vr->penalty1Cur() = row->getString(PENALTYCUR1);
    vr->penalty2NoDec() = row->getInt(PENALTYNODEC2);
    vr->penalty2Amt() = QUERYCLASS::adjustDecimal(row->getInt(PENALTYAMT2), vr->penalty2NoDec());
    vr->penalty2Cur() = row->getString(PENALTYCUR2);
    int noDec = row->getInt(PENALTYPERCENTNODEC);
    vr->penaltyPercentNoDec() = noDec > 0 ? 4 : noDec;
    vr->penaltyPercent() =
        QUERYCLASS::adjustDecimal(row->getInt(PENALTYPERCENT), vr->penaltyPercentNoDec());
    vr->highLowInd() = row->getChar(HIGHLOWIND);
    vr->minimumAmtNoDec() = row->getInt(MINIMUMAMTNODEC);
    vr->minimumAmt() = QUERYCLASS::adjustDecimal(row->getInt(MINIMUMAMT), vr->minimumAmtNoDec());
    vr->minimumAmtCur() = row->getString(MINIMUMAMTCUR);
    vr->reissueFeeInd() = row->getChar(REISSUEFEEIND);
    vr->calcOption() = row->getChar(CALCOPTION);
    vr->discountTag1() = row->getChar(DISCOUNTTAG1);
    vr->discountTag2() = row->getChar(DISCOUNTTAG2);
    vr->discountTag3() = row->getChar(DISCOUNTTAG3);
    vr->discountTag4() = row->getChar(DISCOUNTTAG4);
    vr->formOfRefund() = row->getChar(FORMOFREFUND);
    vr->taxNonrefundableInd() = row->getChar(TAXNONREFUNDABLEIND);
    vr->customer1Period() = row->getString(CUSTOMER1PERIOD);
    vr->customer1Unit() = row->getString(CUSTOMER1UNIT);
    vr->customer1ResTkt() = row->getChar(CUSTOMER1RESTKT);
    vr->carrierApplTblItemNo() = row->getInt(CARRIERAPPLTBLITEMNO);
    vr->overrideDateTblItemNo() = row->getInt(OVERRIDEDATETBLITEMNO);
    vr->unavailTag() = row->getChar(UNAVAILTAG);

    return vr;
  }

  virtual ~QueryGetVoluntaryRefundsInfoSQLStatement() {}

private:
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetVoluntaryRefundsInfoHistoricalSQLStatement
    : public QueryGetVoluntaryRefundsInfoSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("VENDOR = %1q"
                " and ITEMNO = %2n"
                " and VALIDITYIND = 'Y'"
                " and %3n <= EXPIREDATE"
                " and %4n >= CREATEDATE");
  }
};
}

