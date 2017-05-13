//-------------------------------------------------------------------------------
// Copyright 2009, Sabre Inc.  All rights reserved.
// This software/documentation is the confidential and proprietary product of
// Sabre Inc.   Any unauthorized use, reproduction, or transfer of this
// software/documentation,  in any medium, or incorporation of this
// software/documentation into any system or publication, is strictly prohibited.
//-------------------------------------------------------------------------------
#pragma once

#include "Common/Global.h"
#include "DBAccess/OptionalServicesInfo.h"
#include "DBAccess/SQLStatement.h"

namespace tse
{
class Row;

template <class QUERYCLASS>
class QueryGetOptionalServicesSQLStatement : public DBAccess::SQLStatement
{
public:
  QueryGetOptionalServicesSQLStatement() {}
  virtual ~QueryGetOptionalServicesSQLStatement() {}

  enum ColumnIndexes
  {
    VENDOR = 0,
    CARRIER,
    SERVICETYPECODE,
    SERVICESUBTYPECODE,
    FLTTKTMERCHIND,
    SEQNO,
    CREATEDATE,
    EXPIREDATE,
    PUBLICPRIVATEIND,
    EFFDATE,
    DISCDATE,
    TICKETEFFDATE,
    TICKETDISCDATE,
    PSGTYPE,
    MINAGE,
    MAXAGE,
    FIRSTOCCURRENCE,
    LASTOCCURRENCE,
    FREQUENTFLYERSTATUS,
    SVCFEESACCOUNTCODETBLITEMNO,
    SVCFEESTKTDESIGTBLITEMNO,
    TOURCODE,
    SVCFEESSECURITYTBLITEMNO,
    SECTORPORTIONIND,
    FROMTOWITHININD,
    LOC1TYPE,
    LOC1,
    LOC1ZONETBLITEMNO,
    LOC2TYPE,
    LOC2,
    LOC2ZONETBLITEMNO,
    VIALOCTYPE,
    VIALOC,
    VIAZONETBLITEMNO,
    STOPCNXDESTIND,
    STOPOVERTIME,
    STOPOVERUNIT,
    CABIN,
    SVCFEESRESBKGDESIGTBLITEMNO,
    UPGRADECABIN,
    UPGRDSVCFEESRESBKGDESIGTBLITEM,
    SVCFEESCXRRESULTINGFCLTBLITEMN,
    INFORULETARIFF,
    INFORULE,
    RULETARIFFIND,
    FAREIND,
    STARTTIME,
    STOPTIME,
    TIMEAPPLICATION,
    DOW,
    TAXCARRIERFLTTBLITEMNO,
    EQUIPMENTCODE,
    ADVPURCHPERIOD,
    ADVPURCHUNIT,
    ADVPURCHTKTISSUE,
    REFUNDREISSUEIND,
    COMMISSIONIND,
    INTERLINEIND,
    FORMOFFEEREFUNDIND,
    NOTAVAILNOCHARGEIND,
    COLLECTSUBTRACTIND,
    NETSELLIND,
    ANDORIND,
    SVCFEESCURRENCYTBLITEMNO,
    FREQFLYERMILEAGEFEE,
    FREQFLYERMILEAGEAPPL,
    TAXINCLIND,
    AVAILABILITYIND,
    RULEBUSTERFCL,
    TAXTEXTTBLITEMNO,
    SEGCOUNT,
    RESULTSVCFEESTKTDESIGTBLITEMNO,
    SEGNO,
    CATEGORY,
    SEGRULETARIFF,
    SEGRULE,
    TVLSTARTYEAR,
    TVLSTARTMONTH,
    TVLSTARTDAY,
    TVLSTOPYEAR,
    TVLSTOPMONTH,
    TVLSTOPDAY,
    FREEBAGGAGEPCS,
    BAGGAGEOCCURRENCEFIRSTPC,
    BAGGAGEOCCURRENCELASTPC,
    BAGGAGEWEIGHT,
    BAGGAGEWEIGHTUNIT,
    TAXEXEMPTIND,
    BAGGAGETRAVELAPPL
  }; // enum

  const std::string& RegisterColumnsAndBaseSQL(const char* queryName) override
  {
    this->Command(
        "select o.VENDOR, o.CARRIER, o.SERVICETYPECODE, o.SERVICESUBTYPECODE, o.FLTTKTMERCHIND,"
        "       o.SEQNO, o.CREATEDATE, EXPIREDATE, PUBLICPRIVATEIND, EFFDATE, DISCDATE,"
        "       TICKETEFFDATE, TICKETDISCDATE, PSGTYPE, MINAGE, MAXAGE, FIRSTOCCURRENCE,"
        "       LASTOCCURRENCE, FREQUENTFLYERSTATUS, SVCFEESACCOUNTCODETBLITEMNO,"
        "       SVCFEESTKTDESIGTBLITEMNO, TOURCODE, SVCFEESSECURITYTBLITEMNO,"
        "       SECTORPORTIONIND, FROMTOWITHININD, LOC1TYPE, LOC1, LOC1ZONETBLITEMNO,"
        "       LOC2TYPE, LOC2, LOC2ZONETBLITEMNO, VIALOCTYPE, VIALOC, VIAZONETBLITEMNO,"
        "       STOPCNXDESTIND, STOPOVERTIME, STOPOVERUNIT, CABIN, SVCFEESRESBKGDESIGTBLITEMNO,"
        "       UPGRADECABIN, UPGRDSVCFEESRESBKGDESIGTBLITEM, SVCFEESCXRRESULTINGFCLTBLITEMN,"
        "       o.RULETARIFF INFORULETARIFF, o.RULE INFORULE, RULETARIFFIND, FAREIND,"
        "       STARTTIME, STOPTIME, TIMEAPPLICATION, DOW, TAXCARRIERFLTTBLITEMNO, EQUIPMENTCODE,"
        "       ADVPURCHPERIOD, ADVPURCHUNIT, ADVPURCHTKTISSUE, REFUNDREISSUEIND, COMMISSIONIND,"
        "       INTERLINEIND, FORMOFFEEREFUNDIND, NOTAVAILNOCHARGEIND, COLLECTSUBTRACTIND,"
        "       NETSELLIND, ANDORIND, SVCFEESCURRENCYTBLITEMNO, FREQFLYERMILEAGEFEE,"
        "       FREQFLYERMILEAGEAPPL, TAXINCLIND, AVAILABILITYIND, RULEBUSTERFCL,"
        "       TAXTEXTTBLITEMNO, SEGCOUNT, RESULTSVCFEESTKTDESIGTBLITEMNO, SEGNO,"
        "       CATEGORY, s.RULETARIFF SEGRULETARIFF, s.RULE SEGRULE,"
        "       TVLSTARTYEAR, TVLSTARTMONTH, TVLSTARTDAY, TVLSTOPYEAR, TVLSTOPMONTH, TVLSTOPDAY,"
        "       FREEBAGGAGEPCS, BAGGAGEOCCURRENCEFIRSTPC, BAGGAGEOCCURRENCELASTPC, BAGGAGEWEIGHT,"
        "       BAGGAGEWEIGHTUNIT, TAXEXEMPTIND, BAGGAGETRAVELAPPL");

    //------------------------------------------------------------------------
    // Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------
    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(7);
    joinFields.push_back("VENDOR");
    joinFields.push_back("CARRIER");
    joinFields.push_back("SERVICETYPECODE");
    joinFields.push_back("SERVICESUBTYPECODE");
    joinFields.push_back("FLTTKTMERCHIND");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=OPTIONALSERVICES", "o", "LEFT OUTER JOIN", "=OPTIONALSERVICESSEG", "s", joinFields, from);
    this->From(from);
    //------------------------------------------------------------------------
    // End Oracle Conversion Code Block
    //------------------------------------------------------------------------

    this->Where("o.VENDOR = %1q "
                " and o.CARRIER= %2q "
                " and o.FLTTKTMERCHIND = %3q "
                " and o.VALIDITYIND = 'Y'"
                " and %cd <= o.EXPIREDATE");
    this->OrderBy("SEQNO");

    // callback to allow for replacement of SQL clauses by a derived class/template
    adjustBaseSQL();

    this->ConstructSQL();

    // Register SQL statement prior to parameter substitutions
    QUERYCLASS::registerBaseSQL(queryName, *this);

    return *this;
  }

  static OptionalServicesInfo*
  mapRowToOptionalServicesInfo(Row* row, OptionalServicesInfo* prevService)
  {
    VendorCode vendor = row->getString(VENDOR);
    CarrierCode carrier = row->getString(CARRIER);
    ServiceTypeCode type = row->getString(SERVICETYPECODE);
    ServiceSubTypeCode subType = row->getString(SERVICESUBTYPECODE);
    Indicator fltTktMerchInd = row->getChar(FLTTKTMERCHIND);
    long seqNo = row->getLong(SEQNO);
    DateTime createDate = row->getDate(CREATEDATE);

    OptionalServicesInfo* service = nullptr;

    if (prevService && prevService->vendor() == vendor && prevService->carrier() == carrier &&
        prevService->serviceTypeCode() == type && prevService->serviceSubTypeCode() == subType &&
        prevService->fltTktMerchInd() == fltTktMerchInd && prevService->seqNo() == seqNo &&
        prevService->createDate() == createDate)
    {
      service = prevService;
    }
    else
    {
      service = new OptionalServicesInfo;

      service->vendor() = row->getString(VENDOR);
      service->carrier() = row->getString(CARRIER);
      service->serviceTypeCode() = row->getString(SERVICETYPECODE);
      service->serviceSubTypeCode() = row->getString(SERVICESUBTYPECODE);
      service->fltTktMerchInd() = row->getChar(FLTTKTMERCHIND);
      service->seqNo() = row->getLong(SEQNO);
      service->createDate() = row->getDate(CREATEDATE);

      if (LIKELY(!row->isNull(EXPIREDATE)))
        service->expireDate() = row->getDate(EXPIREDATE);
      if (LIKELY(!row->isNull(PUBLICPRIVATEIND)))
        service->publicPrivateInd() = row->getChar(PUBLICPRIVATEIND);
      if (LIKELY(!row->isNull(EFFDATE)))
        service->effDate() = row->getDate(EFFDATE);
      if (LIKELY(!row->isNull(DISCDATE)))
        service->discDate() = row->getDate(DISCDATE);
      if (!row->isNull(TICKETEFFDATE))
        service->ticketEffDate() = row->getDate(TICKETEFFDATE);
      if (!row->isNull(TICKETDISCDATE))
        service->ticketDiscDate() = row->getDate(TICKETDISCDATE);
      if (!row->isNull(PSGTYPE))
        service->psgType() = row->getString(PSGTYPE);
      if (LIKELY(!row->isNull(MINAGE)))
        service->minAge() = row->getInt(MINAGE);
      if (LIKELY(!row->isNull(MAXAGE)))
        service->maxAge() = row->getInt(MAXAGE);
      if (LIKELY(!row->isNull(FIRSTOCCURRENCE)))
        service->firstOccurence() = row->getInt(FIRSTOCCURRENCE);
      if (LIKELY(!row->isNull(LASTOCCURRENCE)))
        service->lastOccurence() = row->getInt(LASTOCCURRENCE);
      if (LIKELY(!row->isNull(FREQUENTFLYERSTATUS)))
        service->frequentFlyerStatus() = row->getInt(FREQUENTFLYERSTATUS);
      if (LIKELY(!row->isNull(SVCFEESACCOUNTCODETBLITEMNO)))
        service->serviceFeesAccountCodeTblItemNo() = row->getInt(SVCFEESACCOUNTCODETBLITEMNO);
      if (LIKELY(!row->isNull(SVCFEESTKTDESIGTBLITEMNO)))
        service->serviceFeesTktDesigTblItemNo() = row->getInt(SVCFEESTKTDESIGTBLITEMNO);
      if (!row->isNull(TOURCODE))
        service->tourCode() = row->getString(TOURCODE);
      if (LIKELY(!row->isNull(SVCFEESSECURITYTBLITEMNO)))
        service->serviceFeesSecurityTblItemNo() = row->getInt(SVCFEESSECURITYTBLITEMNO);
      if (LIKELY(!row->isNull(SECTORPORTIONIND)))
        service->sectorPortionInd() = row->getChar(SECTORPORTIONIND);
      if (LIKELY(!row->isNull(FROMTOWITHININD)))
        service->fromToWithinInd() = row->getChar(FROMTOWITHININD);
      if (LIKELY(!row->isNull(LOC1TYPE)))
        service->loc1().locType() = row->getChar(LOC1TYPE);
      if (!row->isNull(LOC1))
        service->loc1().loc() = row->getString(LOC1);
      if (!row->isNull(LOC1ZONETBLITEMNO))
        service->loc1ZoneTblItemNo() = row->getString(LOC1ZONETBLITEMNO);
      if (LIKELY(!row->isNull(LOC2TYPE)))
        service->loc2().locType() = row->getChar(LOC2TYPE);
      if (!row->isNull(LOC2))
        service->loc2().loc() = row->getString(LOC2);
      if (!row->isNull(LOC2ZONETBLITEMNO))
        service->loc2ZoneTblItemNo() = row->getString(LOC2ZONETBLITEMNO);
      if (LIKELY(!row->isNull(VIALOCTYPE)))
        service->viaLoc().locType() = row->getChar(VIALOCTYPE);
      if (!row->isNull(VIALOC))
        service->viaLoc().loc() = row->getString(VIALOC);
      if (!row->isNull(VIAZONETBLITEMNO))
        service->viaLocZoneTblItemNo() = row->getString(VIAZONETBLITEMNO);
      if (LIKELY(!row->isNull(STOPCNXDESTIND)))
        service->stopCnxDestInd() = row->getChar(STOPCNXDESTIND);
      if (!row->isNull(STOPOVERTIME))
        service->stopoverTime() = row->getString(STOPOVERTIME);
      if (LIKELY(!row->isNull(STOPOVERUNIT)))
        service->stopoverUnit() = row->getChar(STOPOVERUNIT);
      if (LIKELY(!row->isNull(CABIN)))
        service->cabin() = row->getChar(CABIN);
      if (LIKELY(!row->isNull(SVCFEESRESBKGDESIGTBLITEMNO)))
        service->serviceFeesResBkgDesigTblItemNo() = row->getInt(SVCFEESRESBKGDESIGTBLITEMNO);
      if (LIKELY(!row->isNull(UPGRADECABIN)))
        service->upgradeCabin() = row->getChar(UPGRADECABIN);
      if (LIKELY(!row->isNull(UPGRDSVCFEESRESBKGDESIGTBLITEM)))
        service->upgrdServiceFeesResBkgDesigTblItemNo() =
            row->getInt(UPGRDSVCFEESRESBKGDESIGTBLITEM);
      if (LIKELY(!row->isNull(SVCFEESCXRRESULTINGFCLTBLITEMN)))
        service->serviceFeesCxrResultingFclTblItemNo() =
            row->getInt(SVCFEESCXRRESULTINGFCLTBLITEMN);
      if (LIKELY(!row->isNull(INFORULETARIFF)))
        service->ruleTariff() = row->getInt(INFORULETARIFF);
      if (!row->isNull(INFORULE))
        service->rule() = row->getString(INFORULE);
      if (!row->isNull(RULETARIFFIND))
        service->ruleTariffInd() = row->getString(RULETARIFFIND);
      if (LIKELY(!row->isNull(FAREIND)))
        service->fareInd() = row->getChar(FAREIND);
      if (LIKELY(!row->isNull(STARTTIME)))
        service->startTime() = row->getInt(STARTTIME);
      if (LIKELY(!row->isNull(STOPTIME)))
        service->stopTime() = row->getInt(STOPTIME);
      if (LIKELY(!row->isNull(TIMEAPPLICATION)))
        service->timeApplication() = row->getChar(TIMEAPPLICATION);
      if (!row->isNull(DOW))
        service->dayOfWeek() = row->getString(DOW);
      if (!row->isNull(TAXCARRIERFLTTBLITEMNO))
        service->carrierFltTblItemNo() = row->getInt(TAXCARRIERFLTTBLITEMNO);
      if (!row->isNull(EQUIPMENTCODE))
        service->equipmentCode() = row->getString(EQUIPMENTCODE);
      if (!row->isNull(ADVPURCHPERIOD))
        service->advPurchPeriod() = row->getString(ADVPURCHPERIOD);
      if (!row->isNull(ADVPURCHUNIT))
        service->advPurchUnit() = row->getString(ADVPURCHUNIT);
      if (LIKELY(!row->isNull(ADVPURCHTKTISSUE)))
        service->advPurchTktIssue() = row->getChar(ADVPURCHTKTISSUE);
      if (LIKELY(!row->isNull(REFUNDREISSUEIND)))
        service->refundReissueInd() = row->getChar(REFUNDREISSUEIND);
      if (LIKELY(!row->isNull(COMMISSIONIND)))
        service->commissionInd() = row->getChar(COMMISSIONIND);
      if (LIKELY(!row->isNull(INTERLINEIND)))
        service->interlineInd() = row->getChar(INTERLINEIND);
      if (LIKELY(!row->isNull(FORMOFFEEREFUNDIND)))
        service->formOfFeeRefundInd() = row->getChar(FORMOFFEEREFUNDIND);
      if (LIKELY(!row->isNull(NOTAVAILNOCHARGEIND)))
        service->notAvailNoChargeInd() = row->getChar(NOTAVAILNOCHARGEIND);
      if (LIKELY(!row->isNull(COLLECTSUBTRACTIND)))
        service->collectSubtractInd() = row->getChar(COLLECTSUBTRACTIND);
      if (LIKELY(!row->isNull(NETSELLIND)))
        service->netSellingInd() = row->getChar(NETSELLIND);
      if (LIKELY(!row->isNull(ANDORIND)))
        service->andOrInd() = row->getChar(ANDORIND);
      if (!row->isNull(SVCFEESCURRENCYTBLITEMNO))
        service->serviceFeesCurrencyTblItemNo() = row->getInt(SVCFEESCURRENCYTBLITEMNO);
      if (LIKELY(!row->isNull(FREQFLYERMILEAGEFEE)))
        service->applicationFee() = row->getInt(FREQFLYERMILEAGEFEE);
      if (LIKELY(!row->isNull(FREQFLYERMILEAGEAPPL)))
        service->frequentFlyerMileageAppl() = row->getChar(FREQFLYERMILEAGEAPPL);
      if (LIKELY(!row->isNull(TAXINCLIND)))
        service->taxInclInd() = row->getChar(TAXINCLIND);
      if (LIKELY(!row->isNull(AVAILABILITYIND)))
        service->availabilityInd() = row->getChar(AVAILABILITYIND);
      if (!row->isNull(RULEBUSTERFCL))
        service->ruleBusterFcl() = row->getString(RULEBUSTERFCL);
      if (LIKELY(!row->isNull(TAXTEXTTBLITEMNO)))
        service->taxTblItemNo() = row->getInt(TAXTEXTTBLITEMNO);
      if (LIKELY(!row->isNull(SEGCOUNT)))
        service->segCount() = row->getInt(SEGCOUNT);
      if (LIKELY(!row->isNull(RESULTSVCFEESTKTDESIGTBLITEMNO)))
        service->resultServiceFeesTktDesigTblItemNo() = row->getInt(RESULTSVCFEESTKTDESIGTBLITEMNO);
      if (!row->isNull(TVLSTARTYEAR))
      {
        if (isdigit(row->getString(TVLSTARTYEAR)[0]))
          service->tvlStartYear() = row->getInt(TVLSTARTYEAR) + 2000;
      }
      if (!row->isNull(TVLSTARTMONTH))
        service->tvlStartMonth() = row->getInt(TVLSTARTMONTH);
      if (!row->isNull(TVLSTARTDAY))
        service->tvlStartDay() = row->getInt(TVLSTARTDAY);
      if (!row->isNull(TVLSTOPYEAR))
      {
        if (isdigit(row->getString(TVLSTOPYEAR)[0]))
          service->tvlStopYear() = row->getInt(TVLSTOPYEAR) + 2000;
      }
      if (!row->isNull(TVLSTOPMONTH))
        service->tvlStopMonth() = row->getInt(TVLSTOPMONTH);
      if (!row->isNull(TVLSTOPDAY))
        service->tvlStopDay() = row->getInt(TVLSTOPDAY);
      if (!row->isNull(FREEBAGGAGEPCS))
        service->freeBaggagePcs() = row->getInt(FREEBAGGAGEPCS);
      if (!row->isNull(BAGGAGEOCCURRENCEFIRSTPC))
        service->baggageOccurrenceFirstPc() = row->getInt(BAGGAGEOCCURRENCEFIRSTPC);
      if (!row->isNull(BAGGAGEOCCURRENCELASTPC))
        service->baggageOccurrenceLastPc() = row->getInt(BAGGAGEOCCURRENCELASTPC);
      if (!row->isNull(BAGGAGEWEIGHT))
        service->baggageWeight() = row->getInt(BAGGAGEWEIGHT);
      if (LIKELY(!row->isNull(BAGGAGEWEIGHTUNIT)))
        service->baggageWeightUnit() = row->getChar(BAGGAGEWEIGHTUNIT);
      if (LIKELY(!row->isNull(TAXEXEMPTIND)))
        service->taxExemptInd() = row->getChar(TAXEXEMPTIND);
      if (LIKELY(!row->isNull(BAGGAGETRAVELAPPL)))
        service->baggageTravelApplication() = row->getChar(BAGGAGETRAVELAPPL);
    }

    if (!row->isNull(SEGNO))
    {
      OptionalServicesSeg* seg = new OptionalServicesSeg;
      seg->segNo() = row->getInt(SEGNO);

      if (!row->isNull(CATEGORY))
        seg->category() = row->getInt(CATEGORY);
      if (!row->isNull(SEGRULETARIFF))
        seg->ruleTariff() = row->getInt(SEGRULETARIFF);
      if (!row->isNull(SEGRULE))
        seg->rule() = row->getString(SEGRULE);

      service->segs().push_back(seg);
    }

    return service;
  }

private:
  // Override this function to replace the FROM, WHERE, etc prior to SQL construction
  virtual void adjustBaseSQL() {}
};

template <class QUERYCLASS>
class QueryGetOptionalServicesMktSQLStatement
    : public QueryGetOptionalServicesSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    this->Where("o.VENDOR = %1q  "
                " and o.CARRIER= %2q "
                " and o.VALIDITYIND = 'Y' "
                " and o.LOC1TYPE = %3q and o.LOC1 = %4q "
                " and o.LOC2TYPE = %5q and o.LOC2 = %6q "
                " and %cd <= o.EXPIREDATE");
  }
};

////////////////////////////////////////////////////////////////////////
//   Template used to replace Where clause
///////////////////////////////////////////////////////////////////////
template <class QUERYCLASS>
class QueryGetOptionalServicesHistoricalSQLStatement
    : public QueryGetOptionalServicesSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL() override
  {
    DBAccess::SQLStatement partialStatement;
    DBAccess::CompoundSQLStatement compoundStatement;

    partialStatement.Command(
        "select oh.VENDOR, oh.CARRIER, oh.SERVICETYPECODE, oh.SERVICESUBTYPECODE, "
        "oh.FLTTKTMERCHIND,"
        "       oh.SEQNO, oh.CREATEDATE, EXPIREDATE, PUBLICPRIVATEIND, EFFDATE, DISCDATE,"
        "       TICKETEFFDATE, TICKETDISCDATE, PSGTYPE, MINAGE, MAXAGE, FIRSTOCCURRENCE,"
        "       LASTOCCURRENCE, FREQUENTFLYERSTATUS, SVCFEESACCOUNTCODETBLITEMNO,"
        "       SVCFEESTKTDESIGTBLITEMNO, TOURCODE, SVCFEESSECURITYTBLITEMNO,"
        "       SECTORPORTIONIND, FROMTOWITHININD, LOC1TYPE, LOC1, LOC1ZONETBLITEMNO,"
        "       LOC2TYPE, LOC2, LOC2ZONETBLITEMNO, VIALOCTYPE, VIALOC, VIAZONETBLITEMNO,"
        "       STOPCNXDESTIND, STOPOVERTIME, STOPOVERUNIT, CABIN, SVCFEESRESBKGDESIGTBLITEMNO,"
        "       UPGRADECABIN, UPGRDSVCFEESRESBKGDESIGTBLITEM, SVCFEESCXRRESULTINGFCLTBLITEMN,"
        "       oh.RULETARIFF INFORULETARIFF, oh.RULE INFORULE, RULETARIFFIND, FAREIND,"
        "       STARTTIME, STOPTIME, TIMEAPPLICATION, DOW, TAXCARRIERFLTTBLITEMNO, EQUIPMENTCODE,"
        "       ADVPURCHPERIOD, ADVPURCHUNIT, ADVPURCHTKTISSUE, REFUNDREISSUEIND, COMMISSIONIND,"
        "       INTERLINEIND, FORMOFFEEREFUNDIND, NOTAVAILNOCHARGEIND, COLLECTSUBTRACTIND,"
        "       NETSELLIND, ANDORIND, SVCFEESCURRENCYTBLITEMNO, FREQFLYERMILEAGEFEE,"
        "       FREQFLYERMILEAGEAPPL, TAXINCLIND, AVAILABILITYIND, RULEBUSTERFCL,"
        "       TAXTEXTTBLITEMNO, SEGCOUNT, RESULTSVCFEESTKTDESIGTBLITEMNO, SEGNO,"
        "       CATEGORY, sh.RULETARIFF SEGRULETARIFF, sh.RULE SEGRULE,"
        "       TVLSTARTYEAR, TVLSTARTMONTH, TVLSTARTDAY, TVLSTOPYEAR, TVLSTOPMONTH, TVLSTOPDAY,"
        "       FREEBAGGAGEPCS, BAGGAGEOCCURRENCEFIRSTPC, BAGGAGEOCCURRENCELASTPC,"
        "       BAGGAGEWEIGHT, BAGGAGEWEIGHTUNIT, TAXEXEMPTIND, BAGGAGETRAVELAPPL");

    //------------------------------------------------------------------------
    // Oracle Conversion Project Text Follows
    //------------------------------------------------------------------------
    std::string fromh;
    std::vector<std::string> joinFieloh;
    joinFieloh.reserve(7);
    joinFieloh.push_back("VENDOR");
    joinFieloh.push_back("CARRIER");
    joinFieloh.push_back("SERVICETYPECODE");
    joinFieloh.push_back("SERVICESUBTYPECODE");
    joinFieloh.push_back("FLTTKTMERCHIND");
    joinFieloh.push_back("SEQNO");
    joinFieloh.push_back("CREATEDATE");
    this->generateJoinString("=OPTIONALSERVICESH",
                             "oh",
                             "LEFT OUTER JOIN",
                             "=OPTIONALSERVICESSEGH",
                             "sh",
                             joinFieloh,
                             fromh);
    partialStatement.From(fromh);
    partialStatement.Where("oh.VENDOR = %1q  "
                           " and oh.CARRIER= %2q "
                           " and oh.FLTTKTMERCHIND = %3q "
                           " and oh.VALIDITYIND = 'Y'"
                           " and %4n <= oh.EXPIREDATE"
                           " and %5n >= oh.CREATEDATE");
    adjustBaseSQL(0, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();

    partialStatement.Command(
        " union all"
        "  select o.VENDOR, o.CARRIER, o.SERVICETYPECODE, o.SERVICESUBTYPECODE, o.FLTTKTMERCHIND,"
        "       o.SEQNO, o.CREATEDATE, EXPIREDATE, PUBLICPRIVATEIND, EFFDATE, DISCDATE,"
        "       TICKETEFFDATE, TICKETDISCDATE, PSGTYPE, MINAGE, MAXAGE, FIRSTOCCURRENCE,"
        "       LASTOCCURRENCE, FREQUENTFLYERSTATUS, SVCFEESACCOUNTCODETBLITEMNO,"
        "       SVCFEESTKTDESIGTBLITEMNO, TOURCODE, SVCFEESSECURITYTBLITEMNO,"
        "       SECTORPORTIONIND, FROMTOWITHININD, LOC1TYPE, LOC1, LOC1ZONETBLITEMNO,"
        "       LOC2TYPE, LOC2, LOC2ZONETBLITEMNO, VIALOCTYPE, VIALOC, VIAZONETBLITEMNO,"
        "       STOPCNXDESTIND, STOPOVERTIME, STOPOVERUNIT, CABIN, SVCFEESRESBKGDESIGTBLITEMNO,"
        "       UPGRADECABIN, UPGRDSVCFEESRESBKGDESIGTBLITEM, SVCFEESCXRRESULTINGFCLTBLITEMN,"
        "       o.RULETARIFF INFORULETARIFF, o.RULE INFORULE, RULETARIFFIND, FAREIND,"
        "       STARTTIME, STOPTIME, TIMEAPPLICATION, DOW, TAXCARRIERFLTTBLITEMNO, EQUIPMENTCODE,"
        "       ADVPURCHPERIOD, ADVPURCHUNIT, ADVPURCHTKTISSUE, REFUNDREISSUEIND, COMMISSIONIND,"
        "       INTERLINEIND, FORMOFFEEREFUNDIND, NOTAVAILNOCHARGEIND, COLLECTSUBTRACTIND,"
        "       NETSELLIND, ANDORIND, SVCFEESCURRENCYTBLITEMNO, FREQFLYERMILEAGEFEE,"
        "       FREQFLYERMILEAGEAPPL, TAXINCLIND, AVAILABILITYIND, RULEBUSTERFCL,"
        "       TAXTEXTTBLITEMNO, SEGCOUNT, RESULTSVCFEESTKTDESIGTBLITEMNO, SEGNO,"
        "       CATEGORY, s.RULETARIFF SEGRULETARIFF, s.RULE SEGRULE,"
        "       TVLSTARTYEAR, TVLSTARTMONTH, TVLSTARTDAY, TVLSTOPYEAR, TVLSTOPMONTH, TVLSTOPDAY,"
        "       FREEBAGGAGEPCS, BAGGAGEOCCURRENCEFIRSTPC, BAGGAGEOCCURRENCELASTPC,"
        "       BAGGAGEWEIGHT, BAGGAGEWEIGHTUNIT, TAXEXEMPTIND, BAGGAGETRAVELAPPL");

    std::string from;
    std::vector<std::string> joinFields;
    joinFields.reserve(7);
    joinFields.push_back("VENDOR");
    joinFields.push_back("CARRIER");
    joinFields.push_back("SERVICETYPECODE");
    joinFields.push_back("SERVICESUBTYPECODE");
    joinFields.push_back("FLTTKTMERCHIND");
    joinFields.push_back("SEQNO");
    joinFields.push_back("CREATEDATE");
    this->generateJoinString(
        "=OPTIONALSERVICES", "o", "LEFT OUTER JOIN", "=OPTIONALSERVICESSEG", "s", joinFields, from);
    partialStatement.From(from);
    partialStatement.Where("o.VENDOR = %5q  "
                           " and o.CARRIER= %6q "
                           " and o.FLTTKTMERCHIND = %7q "
                           " and o.VALIDITYIND = 'Y'"
                           " and %8n <= o.EXPIREDATE"
                           " and %9n >= o.CREATEDATE");

    adjustBaseSQL(1, partialStatement);
    compoundStatement.push_back(partialStatement);
    partialStatement.Reset();
    this->Command(compoundStatement.ConstructSQL());
    this->From("");
    this->Where("");
    this->OrderBy("SEQNO, CREATEDATE");
  }

  virtual void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) {}
};

template <class QUERYCLASS>
class QueryGetOptionalServicesMktHistoricalSQLStatement
    : public QueryGetOptionalServicesHistoricalSQLStatement<QUERYCLASS>
{
private:
  void adjustBaseSQL(int step, DBAccess::SQLStatement& partialStatement) override
  {
    if (step == 0)
    {
      partialStatement.Where("oh.VENDOR = %1q  "
                             " and oh.CARRIER= %2q "
                             " and oh.LOC1TYPE = %3q and oh.LOC1 = %4q "
                             " and oh.LOC2TYPE = %5q and oh.LOC2 = %6q "
                             " and oh.VALIDITYIND = 'Y'"
                             " and %7n <= oh.EXPIREDATE"
                             " and %8n >= oh.CREATEDATE");
    }
    else if (step == 1)
    {
      partialStatement.Where("o.VENDOR = %9q  "
                             " and o.CARRIER= %10q "
                             " and o.LOC1TYPE = %11q and o.LOC1 = %12q "
                             " and o.LOC2TYPE = %13q and o.LOC2 = %14q "
                             " and o.VALIDITYIND = 'Y'"
                             " and %15n <= o.EXPIREDATE"
                             " and %16n >= o.CREATEDATE");
    }
  }
};
} // tse
