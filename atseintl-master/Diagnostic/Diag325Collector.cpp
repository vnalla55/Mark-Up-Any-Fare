//----------------------------------------------------------------------------
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#include "Diagnostic/Diag325Collector.h"

#include "Common/FallbackUtil.h"
#include "Common/Money.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PaxType.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/BaseFareRule.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/FareByRuleItemInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "Rules/RuleUtil.h"

#include <iomanip>
#include <iostream>

using namespace std;

namespace tse
{

const char*
Diag325Collector::R3_PASS("R3 PASSES");
const char*
Diag325Collector::FAIL_PTC("PAXTYPE");
const char*
Diag325Collector::FAIL_TBL994("TBL994");
const char*
Diag325Collector::FAIL_UNAVLTAG("UNAVLTAG");
const char*
Diag325Collector::FAIL_PAX_STATUS("PAXSTATUS");
const char*
Diag325Collector::FAIL_FLT_SEG_CNT("FLTSEGCNT");
const char*
Diag325Collector::FAIL_WHOLLY_WITHIN_LOC("WHOLLYWITHINLOC");
const char*
Diag325Collector::NO_DISC("NO DISC");
const char*
Diag325Collector::FAIL_MILEAGE("MILEAGE");
const char*
Diag325Collector::FAIL_GLOBAL("RESULT GLOBAL");
const char*
Diag325Collector::FAIL_PCT("RESULT PRCCAT");
const char*
Diag325Collector::R2_PASS("R2 PASSES");
const char*
Diag325Collector::R2_NOT_FOUND("NO MATCHING RECORD 2 CAT 25");
const char*
Diag325Collector::R2_NOT_APPL("NOT APPLICABLE");
const char*
Diag325Collector::R2_FAIL_IF("FAIL IF CONDITION");
const char*
Diag325Collector::R2_FAIL_DIR("FAIL DIRECTIONALITY");
const char*
Diag325Collector::FAIL_SECURITY("CAT35 MSF");
const char*
Diag325Collector::FAIL_CAT35("CAT35 FARE");
const char*
Diag325Collector::FAIL_NON_CAT35("NON CAT35 FARE");

const char*
Diag325Collector::FAIL_PRIVATE_FARE("PRIVATE FARE");
const char*
Diag325Collector::FAIL_RULE_TARIFF("RULE TARIFF");
const char*
Diag325Collector::FAIL_RULE_NUMBER("RULE NUMBER");
const char*
Diag325Collector::FAIL_CARRIER_CODE("CARRIER CODE");
const char*
Diag325Collector::FAIL_OW_RT("OW/RT");
const char*
Diag325Collector::FAIL_GLOBAL_DIRECTION("GLOBAL DIRECTION");
const char*
Diag325Collector::FAIL_PASSENGER_TYPE_CODE("PASSENGER TYPE CODE");
const char*
Diag325Collector::FAIL_FARE_CLASS("FARE CLASS");
const char*
Diag325Collector::FAIL_FARE_TYPE_CODE("FARE TYPE CODE");
const char*
Diag325Collector::FAIL_SEASON_CODE("SEASON CODE");
const char*
Diag325Collector::FAIL_DAY_CODE("DAY CODE");
const char*
Diag325Collector::FAIL_PRICING_CATEGORY_CODE("PRICING CATEGORY CODE");
const char*
Diag325Collector::FAIL_MILEAGE_ROUTING("MILEAGE ROUTING");
const char*
Diag325Collector::FAIL_ROUTING_NUMBER("ROUTING NUMBER");
const char*
Diag325Collector::FAIL_FOOT_NOTE("FOOTNOTE");
const char*
Diag325Collector::FAIL_BOOKING_CODE("BOOKING CODE");
const char*
Diag325Collector::FAIL_MIN_MAX_FARE_RANGE("MIN/MAX FARE RANGE");

const char*
Diag325Collector::FAIL_FARE_BY_RULE_FARE("FARE BY RULE FARE");
const char*
Diag325Collector::FAIL_DISCOUNTED_FARE("DISCOUNTED FARE");
const char*
Diag325Collector::FAIL_VENDOR_CROSS_REF_CARRIER_PREF_FBR("VENDOR CROSS REF/CARRIER PREF FBR");
const char*
Diag325Collector::FAIL_RESULTING_DISPLAY_CATEGORY("RESULTING DISPLAY CATEGORY");
const char*
Diag325Collector::FAIL_VALID_FOR_FBR_BASE_FARE("NOT VALID FOR FBR BASE FARE");
const char*
Diag325Collector::FAIL_BASE_FARE_VENDOR_MATCH("ORIGINATOR/CREATOR NOT THE SAME");
const char*
Diag325Collector::FAIL_BASE_FARE_SECURITY("SECURITY");
const char*
Diag325Collector::FAIL_INVALID_INDUSTRY_FARE("INVALID INDUSTRY FARE");
const char*
Diag325Collector::FAIL_RESULTING_PRICING_CATEGORY("RESULTING PRICING CATEGORY");
const char*
Diag325Collector::FAIL_RESULTING_FARE_TYPE("RESULTING FARE TYPE");
const char*
Diag325Collector::FAIL_RESULTING_GLOBAL("RESULTING GLOBAL");
const char*
Diag325Collector::FAIL_BETWEEN_AND_CITIES("BETWEEN/AND CITIES");

void
Diag325Collector::printHeader()
{
  if (_active)
  {
    *this << "*****************  FARE BY RULE DIAGNOSTICS  ******************\n";
  }
}

void
Diag325Collector::writeHeader(const FareMarket& fareMarket, const PaxType& paxType)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc << "\n";
    dc << "***************************************************************";

    dc << "\n";
    dc << fareMarket.boardMultiCity();
    dc << "-";
    dc << fareMarket.governingCarrier();
    dc << "-";
    dc << fareMarket.offMultiCity();
    dc << "    PSGR-";
    dc << paxType.paxType();
    dc << '\n';
  }
}

Diag325Collector&
Diag325Collector::operator<<(const FareByRuleApp& fbrApp)
{

  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "---------------------------------------------------------------";
    dc << "\n";

    dc << "R8: ";
    dc << std::setw(4) << fbrApp.carrier();
    dc << std::setw(5) << fbrApp.primePaxType();
    dc << std::setw(7) << fbrApp.vendor();
    dc << std::setw(7) << fbrApp.ruleTariff();
    dc << std::setw(7) << fbrApp.ruleNo();
    dc << std::setw(21) << fbrApp.accountCode();
    dc << '\n';
  }

  return *this;
}

void
Diag325Collector::diag325Collector(const FareByRuleCtrlInfo* rule, const char* failCode)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    if (failCode == R2_NOT_FOUND)
    {
      dc << failCode;
    }
    else
    {
      dc << "R2: SEQ NBR ";
      dc << std::setw(8) << rule->sequenceNumber();

      if (failCode == R2_PASS)
      {
        if (rule->loc1().locType() != RuleConst::ANY_LOCATION_TYPE ||
            rule->loc2().locType() != RuleConst::ANY_LOCATION_TYPE ||
            rule->loc1zoneTblItemNo() != RuleConst::NOT_APPLICABLE_ZONE ||
            rule->loc1zoneTblItemNo() != RuleConst::NOT_APPLICABLE_ZONE)
        {
          dc << "\n";
          dc << "LOC1TYPE-";
          dc << std::setw(4) << rule->loc1().locType();
          dc << "LOC1-";
          dc << std::setw(6) << rule->loc1().loc();
          dc << "LOC1ZONE-";
          dc << std::setw(8) << rule->loc1zoneTblItemNo();
          dc << "\n";

          dc << "LOC2TYPE-";
          dc << std::setw(4) << rule->loc2().locType();
          dc << "LOC2-";
          dc << std::setw(6) << rule->loc2().loc();
          dc << "LOC2ZONE-";
          dc << std::setw(8) << rule->loc2zoneTblItemNo();
          dc << "\n";
        }
      }
      else
      {
        dc << "             ***" << failCode << "***";
      }
    }
    dc << '\n';
  }
}

void
Diag325Collector::diag325Collector(const CategoryRuleItemInfo* categoryRuleItemInfo,
                                   const char* failCode,
                                   const std::list<uint16_t>* const failedIfItins)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    if (failCode == R2_PASS)
    {
      dc << "R2S: ";
      switch (categoryRuleItemInfo->relationalInd())
      {
      case CategoryRuleItemInfo::THEN:
      {
        dc << "THEN ";
        break;
      }
      case CategoryRuleItemInfo::OR:
      {
        dc << "OR   ";
        break;
      }
      case CategoryRuleItemInfo::ELSE:
      {
        dc << "ELSE ";
        break;
      }
      default:
      {
        dc << "     ";
      }
      }
      dc << std::setw(10) << categoryRuleItemInfo->itemNo();
      dc << " DIR-";

      dc << std::setw(2) << categoryRuleItemInfo->directionality();
      dc << " I/O-";
      dc << std::setw(2) << categoryRuleItemInfo->inOutInd();
      dc << '\n';
    }
    else
    {
      dc << "R3: ";
      dc << std::setw(10) << categoryRuleItemInfo->itemNo();
      dc << "             ***" << failCode;
      if (failedIfItins && !failedIfItins->empty())
      {
        dc << " ITIN: ";
        for (std::list<uint16_t>::const_iterator it = failedIfItins->begin();
             it != failedIfItins->end();
             /*++it*/)
        {
          dc << *it;
          if (++it != failedIfItins->end())
            dc << ", ";
        }
      }
      dc << "***";
      dc << '\n';
    }
  }
}

void
Diag325Collector::diag325Collector(const FareByRuleItemInfo& fareByRuleItemInfo,
                                   PricingTrx& trx,
                                   const Itin& itin,
                                   const char* failCode)
{
  bool longDisplay(false);
  if (_active)
  {
    if ((trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "REC3") ||
        (trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "T989"))
    {
      longDisplay = true;
    }

    DiagCollector& dc(*this);
    dc.setf(std::ios::left, std::ios::adjustfield);

    bool isSpecifiedFareInd = false;
    if (fareByRuleItemInfo.fareInd() == 'S' || fareByRuleItemInfo.fareInd() == 'K' ||
        fareByRuleItemInfo.fareInd() == 'E' || fareByRuleItemInfo.fareInd() == 'F')
    {
      isSpecifiedFareInd = true;
    }

    if (isSpecifiedFareInd)
    {
      dc << "R3: ";
      dc << std::setw(10) << fareByRuleItemInfo.itemNo();

      dc << "SPECIFIED";
      if (failCode != R3_PASS)
      {
        dc << "             ***FAIL-" << failCode << "***";
      }
      dc << '\n';

      if (failCode == R3_PASS || longDisplay)
      {
        dc << "FIN-";
        dc << std::setw(2) << fareByRuleItemInfo.fareInd();

        dc << "AMT1- ";
        dc.setf(std::ios::right, std::ios::adjustfield);
        dc.setf(std::ios::fixed, std::ios::floatfield);
        dc.precision(2);
        dc << std::setw(10) << fareByRuleItemInfo.specifiedFareAmt1();

        dc.setf(std::ios::left, std::ios::adjustfield);
        dc << " " << std::setw(4) << fareByRuleItemInfo.specifiedCur1();

        dc << "AMT2- ";
        dc.setf(std::ios::right, std::ios::adjustfield);
        dc.setf(std::ios::fixed, std::ios::floatfield);
        dc.precision(2);
        dc << std::setw(10) << fareByRuleItemInfo.specifiedFareAmt2();
        dc.setf(std::ios::left, std::ios::adjustfield);
        dc << " " << std::setw(4) << fareByRuleItemInfo.specifiedCur2();

        dc << '\n';
      }
    }
    else
    {
      dc << "R3: ";
      dc << std::setw(10) << fareByRuleItemInfo.itemNo();
      dc << "TBL989-";
      dc << std::setw(10) << fareByRuleItemInfo.baseTableItemNo();
      if (failCode != R3_PASS)
      {
        dc << "     ***FAIL-" << failCode << "***";
      }
      dc << '\n';

      if (failCode == R3_PASS || longDisplay)
      {
        dc << "FIN-";
        dc << std::setw(2) << fareByRuleItemInfo.fareInd();

        dc << "PCT- ";
        dc.setf(std::ios::right, std::ios::adjustfield);
        dc.setf(std::ios::fixed, std::ios::floatfield);

        dc.precision(fareByRuleItemInfo.percentNoDec());
        dc << std::setw(4) << fareByRuleItemInfo.percent();

        dc << " AMT1- ";

        dc.setf(std::ios::right, std::ios::adjustfield);
        dc.setf(std::ios::fixed, std::ios::floatfield);
        dc.precision(2);
        dc << std::setw(10) << fareByRuleItemInfo.specifiedFareAmt1();
        dc.setf(std::ios::left, std::ios::adjustfield);
        dc << " " << std::setw(4) << fareByRuleItemInfo.specifiedCur1();

        dc << " AMT2- ";
        dc.setf(std::ios::right, std::ios::adjustfield);
        dc.setf(std::ios::fixed, std::ios::floatfield);
        dc.precision(2);
        dc << std::setw(10) << fareByRuleItemInfo.specifiedFareAmt2();
        dc.setf(std::ios::left, std::ios::adjustfield);
        dc << " " << std::setw(4) << fareByRuleItemInfo.specifiedCur2();

        dc << '\n';
      }
    }

    if (longDisplay)
    {
      dc << "PTC-";
      dc << std::setw(5) << fareByRuleItemInfo.paxType();
      dc << "APPL-";
      dc << std::setw(3) << fareByRuleItemInfo.negPsgstatusInd();
      dc << "STATUS-";
      dc << std::setw(3) << fareByRuleItemInfo.passengerInd();
      dc << "PAX LOCTYPE-";
      dc << std::setw(3) << fareByRuleItemInfo.psgLoc1().locType();
      dc << "LOC-";
      dc << std::setw(5) << fareByRuleItemInfo.psgLoc1().loc();
      dc << "\n";

      dc << "WHOLLY WITHIN LOCTYPE-";
      dc << std::setw(3) << fareByRuleItemInfo.whollyWithinLoc().locType();
      dc << "LOC-";
      dc << std::setw(5) << fareByRuleItemInfo.whollyWithinLoc().loc();
      dc << "FLT SEG-";
      dc << std::setw(3) << fareByRuleItemInfo.fltSegCnt();
      dc << "TBL994-";
      dc << std::setw(3) << fareByRuleItemInfo.overrideDateTblItemNo();
      dc << "\n";

      dc << "NO DISC-";
      dc << std::setw(3) << fareByRuleItemInfo.discountInd();
      dc << "SAME T/R-";
      dc << std::setw(3) << fareByRuleItemInfo.sameTariffRule();
      dc << "MIN MILEAGE-";
      dc << std::setw(7) << fareByRuleItemInfo.minMileage();
      dc << "MAX MILEAGE-";
      dc << std::setw(7) << fareByRuleItemInfo.maxMileage();
      dc << "\n";

      dc << "///////////////////////////////////////////////////////////////";
      dc << "\n";

      dc << "FARE COMPARISON: ";
      dc << '\n';

      dc << "CUR1-";
      dc << std::setw(4) << fareByRuleItemInfo.cur1();
      dc << " MIN-";
      dc.setf(std::ios::right, std::ios::adjustfield);
      dc.setf(std::ios::fixed, std::ios::floatfield);
      dc.precision(2);
      dc << std::setw(10) << fareByRuleItemInfo.minFareAmt1();
      dc << " MAX-";
      dc.setf(std::ios::right, std::ios::adjustfield);
      dc.setf(std::ios::fixed, std::ios::floatfield);
      dc.precision(2);
      dc << std::setw(10) << fareByRuleItemInfo.maxFareAmt1();
      dc.setf(std::ios::left, std::ios::adjustfield);
      dc << '\n';

      dc << "CUR2-";
      dc << std::setw(4) << fareByRuleItemInfo.cur2();
      dc << " MIN-";
      dc.setf(std::ios::right, std::ios::adjustfield);
      dc.setf(std::ios::fixed, std::ios::floatfield);
      dc.precision(2);
      dc << std::setw(10) << fareByRuleItemInfo.minFareAmt2();
      dc << " MAX-";
      dc.setf(std::ios::right, std::ios::adjustfield);
      dc.setf(std::ios::fixed, std::ios::floatfield);
      dc.precision(2);
      dc << std::setw(10) << fareByRuleItemInfo.maxFareAmt2();
      dc.setf(std::ios::left, std::ios::adjustfield);
      dc << '\n';

      dc << "TARIFF-";
      dc << std::setw(4) << fareByRuleItemInfo.ruleTariff();
      dc << " CXR-";
      dc << std::setw(4) << fareByRuleItemInfo.carrier();
      dc << " FC-";

      dc << std::setw(10) << fareByRuleItemInfo.baseFareClass();
      dc << " FT-";
      dc << std::setw(4) << fareByRuleItemInfo.baseFareType();
      dc << "\n";

      dc << "///////////////////////////////////////////////////////////////";
      dc << "\n";

      dc << "RESULTING FARE: ";
      dc << '\n';

      dc << "OWRT-";
      dc << std::setw(2) << fareByRuleItemInfo.resultowrt();
      dc << " GBL-";
      std::string gd;
      globalDirectionToStr(gd, fareByRuleItemInfo.resultglobalDir());
      dc << std::setw(3) << gd;
      dc << " RTG TARIFF-";
      dc << std::setw(4) << fareByRuleItemInfo.resultRoutingTariff();
      dc << " RTG-";
      dc << std::setw(5) << fareByRuleItemInfo.resultRouting();
      dc << " RTG VENDOR-";
      dc << std::setw(4) << fareByRuleItemInfo.resultRoutingVendor();
      dc << '\n';

      dc << "FC-";
      dc << std::setw(9) << fareByRuleItemInfo.resultFareClass1();
      dc << "FT-";
      dc << std::setw(4) << fareByRuleItemInfo.resultFareType1();
      dc << " ST-";
      dc << std::setw(3) << fareByRuleItemInfo.resultseasonType();
      dc << " DT-";
      dc << std::setw(3) << fareByRuleItemInfo.resultdowType();
      dc << " DCT-";
      dc << std::setw(3) << fareByRuleItemInfo.resultDisplaycatType();
      dc << " PCT-";
      dc << std::setw(3) << fareByRuleItemInfo.resultpricingcatType();
      dc << '\n';

      dc << "BC1-";
      dc << std::setw(2) << fareByRuleItemInfo.bookingCode1();
      dc << " BC2-";
      dc << std::setw(2) << fareByRuleItemInfo.bookingCode2();
      dc << " BC3-";
      dc << std::setw(2) << fareByRuleItemInfo.bookingCode3();
      dc << " BC4-";
      dc << std::setw(2) << fareByRuleItemInfo.bookingCode4();
      dc << " BC5-";
      dc << std::setw(2) << fareByRuleItemInfo.bookingCode5();
      dc << " BC6-";
      dc << std::setw(2) << fareByRuleItemInfo.bookingCode6();
      dc << " BC7-";
      dc << std::setw(2) << fareByRuleItemInfo.bookingCode7();
      dc << " BC8-";
      dc << std::setw(2) << fareByRuleItemInfo.bookingCode8();
      dc << '\n';
      dc << "BCTBL-";
      dc << std::setw(8) << fareByRuleItemInfo.bookingCodeTblItemNo();
      dc << " PRIME SEC-";
      dc << std::setw(2) << fareByRuleItemInfo.primeSector();
      dc << '\n';

      dc << "TC-";
      dc << std::setw(11) << fareByRuleItemInfo.tktCode();
      dc << " TCM-";
      dc << std::setw(2) << fareByRuleItemInfo.tktCodeModifier();
      dc << " TD-";
      dc << std::setw(11) << fareByRuleItemInfo.tktDesignator();
      dc << " TDM-";
      dc << std::setw(2) << fareByRuleItemInfo.tktDesignatorModifier();
      dc << '\n';

      dc << "///////////////////////////////////////////////////////////////";
      dc << '\n';

      if (!isSpecifiedFareInd)
      {
        dc << "CATEGORY OVERRIDE TAGS X-FBR B-BASE BLANK-BOTH:";
        dc << '\n';
        dc << " 1-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat1();
        dc << " 2-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat2();
        dc << " 3-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat3();
        dc << " 4-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat4();
        dc << " 5-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat5();
        dc << " 6-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat6();
        dc << " 7-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat7();
        dc << " 8-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat8();
        dc << " 9-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat9();
        dc << "10-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat10();
        dc << "11-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat11();
        dc << "12-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat12();
        dc << '\n';

        dc << "13-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat13();
        dc << "14-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat14();
        dc << "15-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat15();
        dc << "16-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat16();
        dc << "17-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat17();
        dc << "18-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat18();
        dc << "19-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat19();
        dc << "20-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat20();
        dc << "21-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat21();
        dc << "22-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat22();
        dc << "23-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat23();
        dc << "26-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat26();
        dc << '\n';

        dc << "27-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat27();
        dc << "28-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat28();
        dc << "29-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat29();
        dc << "31-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat31();
        dc << "35-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat35();
        dc << "50-";
        dc << std::setw(2) << fareByRuleItemInfo.ovrdcat50();
        dc << '\n';

        dc << "///////////////////////////////////////////////////////////////";
        dc << '\n';
      }
    }
  }
}

Diag325Collector&
Diag325Collector::operator << (const BaseFareRule& baseFareRule )
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);

    dc << "SEQ NBR - ";
    dc << std::setw(10) << baseFareRule.seqNo();
    dc << '\n';

    dc << "APPL-";
    dc << std::setw(4) << baseFareRule.baseFareAppl();
    dc << "TARIFF-";
    dc << std::setw(4) << baseFareRule.baseRuleTariff();
    dc << "RULE-";
    dc << std::setw(5) << baseFareRule.baseRuleNo();
    dc << "OWRT-";
    dc << std::setw(2) << baseFareRule.baseowrt();
    dc << "FC-";
    dc << std::setw(9) << baseFareRule.baseFareClass();
    dc << '\n';

    dc << "FT-";
    dc << std::setw(5) << baseFareRule.baseFareType();
    dc << "PTC-";
    dc << std::setw(5) << baseFareRule.basepsgType();
    dc << "CXR-";
    dc << std::setw(4) << baseFareRule.carrier();
    dc << "BTW-";
    dc << std::setw(5) << baseFareRule.market1();
    dc << "AND-";
    dc << std::setw(5) << baseFareRule.market2();
    dc << '\n';

    dc << "ST-";
    dc << std::setw(3) << baseFareRule.baseseasonType();
    dc << "DT-";
    dc << std::setw(3) << baseFareRule.basedowType();
    dc << "PCT-";
    dc << std::setw(3) << baseFareRule.basepricingcatType();
    dc << "GBL-";
    std::string gd;
    globalDirectionToStr(gd, baseFareRule.baseglobalDir());
    dc << std::setw(3) << gd;
    dc << "RTG-";
    dc << std::setw(5) << baseFareRule.baseRouting();
    dc << "FN1-";
    dc << std::setw(3) << baseFareRule.basefootNote1();
    dc << "FN2-";
    dc << std::setw(3) << baseFareRule.basefootNote2();
    dc << "BC1-";
    dc << std::setw(3) << baseFareRule.bookingCode1();
    dc << "BC2-";
    dc << std::setw(3) << baseFareRule.bookingCode2();
    dc << '\n';

    dc << "CUR1-";
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << std::setw(4) << baseFareRule.baseCur1();
    dc << " MIN-";
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc.precision(2);
    dc << std::setw(10) << baseFareRule.baseminFare1();
    dc << " MAX-";
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc.precision(2);
    dc << std::setw(10) << baseFareRule.baseMaxFare1();
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << '\n';

    dc << "CUR2-";
    dc << std::setw(4) << baseFareRule.baseCur2();
    dc << " MIN-";
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc.precision(2);
    dc << std::setw(10) << baseFareRule.baseminFare2();
    dc << " MAX-";
    dc.setf(std::ios::right, std::ios::adjustfield);
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc.precision(2);
    dc << std::setw(10) << baseFareRule.baseMaxFare2();
    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << '\n';

    dc << "///////////////////////////////////////////////////////////////";
    dc << '\n';
  }

  return *this;
}

Diag325Collector&
Diag325Collector::operator << (const PaxTypeFare& paxFare )
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);
    PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(_trx);
    if (pricingTrx)
    {
      if (paxFare.isSpecifiedFare())
      {
        dc << std::setw(9) << paxFare.fare()->fareClass();
        dc << std::setw(8) << Money(paxFare.fare()->fareAmount(), paxFare.fare()->currency());
      }
      else
      {
        dc << std::setw(9) << paxFare.fare()->fareClass();
        dc << std::setw(8) << Money(paxFare.fare()->fareAmount(), paxFare.fare()->currency());

        dc << " ";
        dc << std::setw(9) << paxFare.baseFare()->fareClass();
        dc << std::setw(3) << paxFare.baseFare()->fcaFareType();
        dc << " " << paxFare.baseFare()->fcaDisplayCatType() << " ";
        dc << std::setw(2)
           << (paxFare.baseFare()->fare()->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED ? "R" : "O");
        dc << std::setw(2);
        if (paxFare.baseFare()->directionality() == FROM)
          dc << "O";
        else if (paxFare.baseFare()->directionality() == TO)
          dc << "I";
        else
          dc << " ";

        dc << std::setw(4) << paxFare.baseFare()->fareTariff();
        dc << std::setw(5) << paxFare.baseFare()->fare()->ruleNumber();

        dc << std::setw(8) << Money(paxFare.baseFare()->fare()->fareAmount(),
                                    paxFare.baseFare()->fare()->currency());
      }
    }

    if (pricingTrx)
      if (pricingTrx->awardRequest())
      {
        dc.setf(std::ios::right);
        dc << std::setw(8) << " " << paxFare.mileage() << " MIL";
      }

    dc << '\n';
  }

  return *this;
}

std::string
Diag325Collector::formatFailBaseFareMessage(const PaxTypeFare& paxFare, const char* reason)
{
  std::ostringstream oss;

  if (_active)
  {
    oss.setf(std::ios::left, std::ios::adjustfield);
    oss << std::setw(9) << paxFare.fareClass();
    oss << std::setw(8) << Money(paxFare.fare()->fareAmount(), paxFare.fare()->currency());
    oss << "    " << reason << "\n";
  }

  return oss.str();
}

void
Diag325Collector::displayFailBaseFareList(std::vector<string>& failList)
{
  if (_active)
  {
    DiagCollector& dc(*this);

    dc.setf(std::ios::left, std::ios::adjustfield);
    dc << "FAILED BASE FARE LIST:   FAILED REASON:\n";

    for (const auto& elem : failList)
    {
      dc << elem;
    }
    dc << "///////////////////////////////////////////////////////////////\n";
  }
}

void
Diag325Collector::displayRemovedFares(const FareMarket& fm,
                                      const uint16_t& uniqueFareCount,
                                      const std::vector<PaxTypeFare*>& removedFaresVec)
{
  if (_active)
  {
    DiagCollector& dc(*this);
    dc << "\nREMOVED FARES FOR FM : " << fm.origin()->loc() << "-" << fm.governingCarrier() << "-"
       << fm.destination()->loc() << "\n";
    dc << "DUPLICATED FARE COUNT: " << removedFaresVec.size() << "\n";
    dc << "UNIQUE FARE COUNT    : " << uniqueFareCount << "\n";
    dc << "VEND TAR  CX RULE FARECLS  AMOUNT\n";
    dc << "====================================================\n";

    std::vector<PaxTypeFare*>::const_iterator currItr = removedFaresVec.begin();
    std::vector<PaxTypeFare*>::const_iterator endItr = removedFaresVec.end();

    for (; currItr != endItr; ++currItr)
    {
      const PaxTypeFare* currPtf = *currItr;
      dc << std::setw(5) << currPtf->vendor() << std::setw(5) << currPtf->fareTariff()
         << std::setw(3) << currPtf->carrier() << std::setw(5) << currPtf->ruleNumber()
         << std::setw(9) << currPtf->fareClass() << std::setw(10) << currPtf->originalFareAmount()
         << "\n";
    }
    dc << "\n";
  }
}
}
