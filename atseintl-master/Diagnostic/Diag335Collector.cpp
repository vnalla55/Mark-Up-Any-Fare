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

#include "Diagnostic/Diag335Collector.h"

#include "Common/Money.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxType.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/CategoryRuleItemInfo.h"
#include "DBAccess/MarkupCalculate.h"
#include "DBAccess/NegFareCalcInfo.h"
#include "DBAccess/NegFareRest.h"
#include "DBAccess/NegFareSecurityInfo.h"
#include "Rules/NegotiatedFareRuleUtil.h"
#include "Rules/RuleConst.h"

#include <iomanip>

namespace tse
{
const char*
Diag335Collector::R3_PASS("R3 PASSES\n");
const char*
Diag335Collector::FAIL_PTC("PAXTYPE\n");
const char*
Diag335Collector::FAIL_TBL994("TBL994\n");
const char*
Diag335Collector::FAIL_UNAVLTAG("UNAVLTAG\n");
const char*
Diag335Collector::FAIL_TEXTONLY("TEXTONLY\n");
const char*
Diag335Collector::FAIL_TKT_NETREMIT_SELLING("INVALID NET REMIT TKT DATA FOR SELLING FARE\n");
const char*
Diag335Collector::FAIL_COMM_NETREMIT_TYPE_L("INVALID COMMISSION DATA NET REMIT FOR FARE TYPE-L\n");
const char*
Diag335Collector::FAIL_COMM_NETREMIT_TYPE_C("INVALID COMMISSION DATA NET REMIT FOR FARE TYPE-C\n");
const char*
Diag335Collector::FAIL_COMM_NETREMIT_TYPE_T("INVALID COMMISSION DATA NET REMIT FOR FARE TYPE-T\n");
const char*
Diag335Collector::FAIL_TKT_NETREMIT_TYPE_C_T("INVALID NET REMIT TKT DATA FOR NET FARE\n");
const char*
Diag335Collector::FAIL_TKT_NETREMIT_TYPE_L_C_T("INVALID NET REMIT TKT DATA FOR NET FARE\n");
const char*
Diag335Collector::R2_NOT_FOUND("NO MATCHING RECORD 2 CAT 35\n");
const char*
Diag335Collector::R2_NOT_APPL("RECORD 2 CATEGORY 35 NOT APPLICABLE\n");
const char*
Diag335Collector::FAIL_CARRIER_RESRT("FAIL - CARRIER RESTRICTION\n");
const char*
Diag335Collector::FAIL_REC_SEG_1_2_DATA("FAIL - INVALID RECURRING SEGMENTS DATA\n");
const char*
Diag335Collector::FAIL_REC_SEG_BTW_AND("FAIL - INVALID RECURRING SEGMENTS BTW-AND DATA\n");
const char*
Diag335Collector::FAIL_REC_SEG_TKT_FARE_IND("FAIL - INVALID RECURRING SEGMENTS TKT FARE IND\n");
const char*
Diag335Collector::FAIL_REC_SEG_ITIN("FAIL - INVALID ITINERARY FOR RECURRING SEGMENTS DATA\n");
const char*
Diag335Collector::FAIL_LOC_OUT_SIDE_KOREA("USER LOCATION OUTSIDE KOREA\n");
const char*
Diag335Collector::FAIL_TKT_FARE_DATA_IND("UNSUPPORTED TICKETED FARE DATA INDICATOR\n");
const char*
Diag335Collector::R2_FAIL_EFF_DISC_DATE("RECORD 2 EFF/DISC DATE VALIDATION FAILED FOR ITIN ");
const char*
Diag335Collector::R2_FAIL_IF("FAIL IF CONDITION\n\n");

void
Diag335Collector::printHeader()
{
  if (_active)
  {
    *this << "***************  NET FARE MARK-UP DIAGNOSTICS  ****************\n";
  }
}

Diag335Collector&
Diag335Collector::operator<<(const FareMarket& fareMarket)
{
  if (_active)
  {
    *this << "\n***************************************************************\n";
    *this << fareMarket.boardMultiCity() << "-";
    *this << fareMarket.governingCarrier() << "-";
    *this << fareMarket.offMultiCity() << "\n";
  }
  return *this;
}

Diag335Collector&
Diag335Collector::operator << (const PaxTypeFare& paxTypeFare)
{
  if (_active)
  {
    std::ostringstream oss;
    oss << "---------------------------------------------------------------\n";
    oss.setf(std::ios::left, std::ios::adjustfield);
    oss << std::setw(9) << paxTypeFare.fareClass();
    oss << std::setw(5) << paxTypeFare.vendor();
    oss << std::setw(5) << paxTypeFare.ruleNumber();
    oss << std::setw(4) << paxTypeFare.fareTariff();
    oss << std::setw(3) << (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED ? "RT" : "OW");
    oss << std::setw(8) << Money(paxTypeFare.fareAmount(), paxTypeFare.currency());
    oss << " FT-" << std::setw(4) << paxTypeFare.fcaFareType();
    oss << "PTC-" << std::setw(4) << paxTypeFare.fcasPaxType();
    oss << "DCT-" << std::setw(2) << paxTypeFare.fcaDisplayCatType();
    oss << "\n";
    _itemString = oss.str();
  }
  return *this;
}

//----------------------------------------------------------------------------
// echo CategoryRuleInfo seq #
//----------------------------------------------------------------------------
DiagCollector&
Diag335Collector::operator<<(const CategoryRuleInfo& x)
{
  if (_active)
  {
    std::ostringstream oss;
    oss.setf(std::ios::left, std::ios::adjustfield);

    oss << "R2:SEQ NBR ";
    oss << std::setw(8) << x.sequenceNumber();

    displayLocKeys(x.loc1(), x.loc2());

    if (x.applInd() == RuleConst::STRING_DOES_NOT_APPLY)
    {
      oss << "        *** NOT APPLICABLE ***";
    }
    oss << "\n";
    _itemString.append(oss.str());
  }
  return *this;
}

//----------------------------------------------------------------------------
// echo loc1, loc2
//----------------------------------------------------------------------------
void
Diag335Collector::displayLocKeys(const LocKey& loc1, const LocKey& loc2)
{
  // if there is something to display
  if (loc1.locType() != RuleConst::ANY_LOCATION_TYPE ||
      loc2.locType() != RuleConst::ANY_LOCATION_TYPE)
  {
    std::ostringstream oss;
    // loc1
    oss << "\n";
    oss << "LOC1TYPE-";
    oss << std::setw(4) << loc1.locType();
    oss << "LOC1-";
    oss << std::setw(6) << loc1.loc();
    // loc2
    oss << "        LOC2TYPE-";
    oss << std::setw(4) << loc2.locType();
    oss << "LOC2-";
    oss << std::setw(6) << loc2.loc();
    oss << "\n";
    _itemString.append(oss.str());
  }
}

//----------------------------------------------------------------------------
// echo CategoryRuleItemInfo
//----------------------------------------------------------------------------
DiagCollector&
Diag335Collector::operator<<(const CategoryRuleItemInfo& x)
{
  if (_active)
  {
    displayRec2(x);
    _itemString.append("\n");
  }

  return *this;
}

void
Diag335Collector::displayRec2(const CategoryRuleItemInfo& x)
{
  _itemString.append("R2S: ");

  std::ostringstream oss;
  oss.setf(std::ios::left, std::ios::adjustfield);
  oss << std::setw(5) << getRelationString(x.relationalInd());
  oss << std::setw(4) << x.itemcat();
  oss << std::setw(10) << x.itemNo();
  oss << " DIR-";
  oss << std::setw(2) << x.directionality();
  oss << " I/O-";
  oss << std::setw(2) << x.inOutInd();
  _itemString.append(oss.str());
}

//----------------------------------------------------------------------------
// echo NegFareSecurityInfo
//----------------------------------------------------------------------------
DiagCollector&
Diag335Collector::operator<<(const NegFareSecurityInfo& x)
{
  if (_active)
  {
    std::ostringstream oss;
    oss.setf(std::ios::left, std::ios::adjustfield);
    oss << "T983:SEQ NBR " << std::setw(8) << x.seqNo();
    oss << "UPD-" << x.updateInd();
    oss << " REDIS-" << x.redistributeInd();
    oss << " SELL-" << x.sellInd();
    oss << " TKT-" << x.ticketInd();
    oss << " T980-" << x.secondarySellerId();
    oss << "\n";
    _itemString.append(oss.str());
  }
  return *this;
}

//----------------------------------------------------------------------------
// echo NegFareCalcInfo
//----------------------------------------------------------------------------
DiagCollector&
Diag335Collector::operator<<(const NegFareCalcInfo& x)
{

  if (_active)
  {
    std::ostringstream oss;
    oss.setf(std::ios::left, std::ios::adjustfield);
    oss.setf(std::ios::fixed, std::ios::floatfield);
    oss.precision(2);

    oss << "T979:SEQ NBR " << std::setw(8) << x.seqNo();
    oss << "N/S-" << std::setw(10) << ((x.netSellingInd() == 'S') ? "SELLING" : "NET");
    oss << "F/IND-" << x.fareInd() << "\n";

    _itemString.append(oss.str());

    // if fareInd is not CAMS &&
    // if fareInd is not PNTR
    if (!displaySellingAmounts(x.fareInd(),
                               x.sellingPercent(),
                               x.sellingFareAmt1(),
                               x.sellingCur1(),
                               x.sellingFareAmt2(),
                               x.sellingCur2()) &&
        !displayMinMaxAmounts(x.fareInd(),
                              x.calcPercentMin(),
                              x.calcPercentMax(),
                              x.calcMinFareAmt1(),
                              x.calcMaxFareAmt1(),
                              x.calcCur1(),
                              x.calcMinFareAmt2(),
                              x.calcMaxFareAmt2(),
                              x.calcCur2()))
    {
      // dispay error message
      _itemString.append("unknown fare indicator\n");
    }
  }

  return *this;
}

//----------------------------------------------------------------------------
// display details about selling amounts
// returns false if fareInd is not handled.
//----------------------------------------------------------------------------
bool
Diag335Collector::displaySellingAmounts(Indicator fareInd,
                                        Percent percent,
                                        double sellingAmount1,
                                        const tse::CurrencyCode& currency1,
                                        double sellingAmount2,
                                        const tse::CurrencyCode& currency2)
{
  std::ostringstream oss;
  oss.setf(std::ios::fixed, std::ios::floatfield);
  oss.precision(2);

  bool ret = true;
  switch (fareInd)
  {
  case 'C':
  case 'A':
  case 'M':
    // display percent
    oss.setf(std::ios::left, std::ios::adjustfield);
    oss << "PCT-" << std::setw(7) << percent;
  // intentional fall-thru
  case 'S':
    if (fareInd == 'C') // only percent
      break;

    // display selling amounts
    oss.setf(std::ios::right, std::ios::adjustfield);
    oss << "FARE1-" << std::setw(10) << Money(sellingAmount1, currency1);
    oss << " FARE2-" << std::setw(10) << Money(sellingAmount2, currency2);

    break;
  default:
    ret = false;
    break;
  }

  // If something was displayed
  if (ret)
    oss << "\n";
  _itemString.append(oss.str());
  return ret;
}

//----------------------------------------------------------------------------
// display details about min/max amounts
// returns false if fareInd is not handled.
//----------------------------------------------------------------------------
bool
Diag335Collector::displayMinMaxAmounts(Indicator fareInd,
                                       Percent minPercent,
                                       Percent maxPercent,
                                       double minAmount1,
                                       double maxAmount1,
                                       const tse::CurrencyCode& currency1,
                                       double minAmount2,
                                       double maxAmount2,
                                       const tse::CurrencyCode& currency2)
{
  std::ostringstream oss;
  oss.setf(std::ios::fixed, std::ios::floatfield);
  oss.precision(2);

  bool ret = true;
  switch (fareInd)
  {
  case 'P':
  case 'N':
  case 'T':
    // display min/max percent

    oss.setf(std::ios::left, std::ios::adjustfield);
    oss << "MIN PCT-" << std::setw(7) << minPercent;
    oss << " MAX PCT-" << std::setw(7) << maxPercent << "\n";
  // intentional fall-thru
  case 'R':
    if (fareInd == 'P') // only perceent
      break;

    // display min max amounts
    oss.setf(std::ios::right, std::ios::adjustfield);
    oss << "MIN FARE1-" << std::setw(10) << Money(minAmount1, currency1);
    oss << " MAX FARE1-" << std::setw(10) << Money(maxAmount1, currency1) << "\n";
    oss << "MIN FARE2-" << std::setw(10) << Money(minAmount2, currency2);
    oss << " MAX FARE2-" << std::setw(10) << Money(maxAmount2, currency2) << "\n";

    break;
  default:
    ret = false;
    break;
  }
  _itemString.append(oss.str());
  return ret;
}

//----------------------------------------------------------------------------
// echo MarkupCalculate
//----------------------------------------------------------------------------
DiagCollector&
Diag335Collector::operator<<(const MarkupCalculate& x)
{
  if (_active)
  {
    std::ostringstream oss;

    oss.setf(std::ios::left, std::ios::adjustfield);
    oss.setf(std::ios::fixed, std::ios::floatfield);
    oss.precision(2);

    oss << "T980:ORDER NBR " << std::setw(8) << x.orderNo();
    oss << "F/IND-" << x.sellingFareInd() << "\n";

    _itemString.append(oss.str());

    // display CAMS
    displaySellingAmounts(x.sellingFareInd(),
                          x.sellingPercent(),
                          x.sellingFareAmt1(),
                          x.sellingCur1(),
                          x.sellingFareAmt2(),
                          x.sellingCur2());

    std::ostringstream temp;
    temp << "MU/IND-" << x.markupFareInd() << "\n";

    _itemString.append(temp.str());
    //  oss << "MU/IND-" << x.markupFareInd() << "\n";

    // display PNTR
    displayMinMaxAmounts(x.markupFareInd(),
                         x.percentMin(),
                         x.percentMax(),
                         x.markupMinAmt1(),
                         x.markupMaxAmt1(),
                         x.markupCur1(),
                         x.markupMinAmt2(),
                         x.markupMaxAmt2(),
                         x.markupCur2());
  }
  return *this;
}

//----------------------------------------------------------------------------
// echo NegFareRest
//----------------------------------------------------------------------------
DiagCollector&
Diag335Collector::operator<<(const NegFareRest& x)
{
  if (_active)
  {
    std::ostringstream oss;
    oss.setf(std::ios::left, std::ios::adjustfield);
    oss << std::endl;
    oss << "R3:" << std::setw(8) << x.itemNo();
    oss << "PTC-" << std::setw(5) << x.psgType();
    oss << "T994-" << std::setw(8) << x.overrideDateTblItemNo();
    oss << "T983-" << std::setw(8) << x.negFareSecurityTblItemNo();
    oss << "T979-" << std::setw(8) << x.negFareCalcTblItemNo();
    oss << "\n";
    _itemString.append(oss.str());
  }
  return *this;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function int NegotiatedFareController::doNetRemitTkt
//
// Description:  To display a ticketing data for Record 3 Cat 35.
//
// @param  negFareRest   A reference to the NegFareRest object
// @param  failCode      A failed reason code. Value R3_PASS if passes.
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag335Collector::doNetRemitTkt(const NegFareRest& negFareRest,
                                bool isAxessUser,
                                bool isRecurringSegData)
{
  if (_active)
  {
    std::ostringstream oss;
    std::string gd;
    std::string segment;
    globalDirectionToStr(gd, negFareRest.globalDir1());
    const char* axessInd = isAxessUser ? "*X- " : "  - ";

    // header
    oss << " TICKETING FARE DATA: \n";

    // Segment 1
    oss << " SEGMENT 1 \n";
    oss << "  NET REMIT METHOD   - " << std::setw(1) << negFareRest.netRemitMethod() << "\n";
    displaySegment(negFareRest.tktFareDataInd1(),
                   negFareRest.owrt1(),
                   negFareRest.seasonType1(),
                   negFareRest.dowType1(),
                   gd,
                   negFareRest.carrier11(),
                   negFareRest.rule1(),
                   negFareRest.ruleTariff1(),
                   negFareRest.fareType1(),
                   negFareRest.fareClass1(),
                   negFareRest.betwCity1(),
                   negFareRest.andCity1(),
                   axessInd,
                   segment);
    oss << segment;
    segment.clear();
    if (isRecurringSegData)
    {
      segment.clear();
      // Segment 2
      oss << " SEGMENT 2 \n";
      displaySegment(negFareRest.tktFareDataInd2(),
                     negFareRest.owrt2(),
                     negFareRest.seasonType2(),
                     negFareRest.dowType2(),
                     gd,
                     negFareRest.carrier21(),
                     negFareRest.rule2(),
                     negFareRest.ruleTariff2(),
                     negFareRest.fareType2(),
                     negFareRest.fareClass2(),
                     negFareRest.betwCity2(),
                     negFareRest.andCity2(),
                     axessInd,
                     segment);
      oss << segment;
      segment.clear();
    }

    // Commisions
    oss << " COMMISSIONS DATA:\n";
    oss << "  COMM PERCENT       - ";
    if (negFareRest.commPercent() == RuleConst::PERCENT_NO_APPL)
      oss << "NOT APPL\n";
    else
      oss << std::setw(8) << negFareRest.commPercent() << '\n';

    oss << "  COMMISSIONS 1    " << axessInd;
    displayCommision(negFareRest.commAmt1(), negFareRest.cur1(), segment);
    oss << segment;
    segment.clear();
    oss << "  COMMISSIONS 2    " << axessInd;

    displayCommision(negFareRest.commAmt2(), negFareRest.cur2(), segment);
    oss << segment;
    // Tour codes
    PricingTrx* pricingTrx = dynamic_cast<PricingTrx*>(trx());
    if (pricingTrx)
    {
      oss << " TOUR BOX TYPE1      - " << negFareRest.tourBoxCodeType1() << '\n';
      oss << " TOUR BOX CODE1      - " << negFareRest.tourBoxCode1() << '\n';

      if (negFareRest.noSegs() == NegotiatedFareRuleUtil::TWO_SEGMENTS)
      {
        oss << " TOUR BOX TYPE2      - " << negFareRest.tourBoxCodeType2() << '\n';
        oss << " TOUR BOX CODE2      - " << negFareRest.tourBoxCode2() << '\n';
      }
    }

    // footer for axess
    if (isAxessUser)
    {
      oss << "          ** NOTE: *X- IS IGNORED FOR AXESS USER\n";
    }
    _itemString.append(oss.str());
  }
}

//----------------------------------------------------------------------------
// displays commision if applicable
//----------------------------------------------------------------------------
void
Diag335Collector::displayCommision(const MoneyAmount& amount,
                                   const tse::CurrencyCode& code,
                                   std::string& segment)
{
  std::ostringstream oss;
  oss << std::setw(8);

  if (!code.empty())
  {
    oss << Money(amount, code) << '\n';
  }
  else
  {
    oss << "NONE"
        << "\n";
  }
  segment.append(oss.str());
}

//----------------------------------------------------------------------------
// displays all parameters, which describes segment
//----------------------------------------------------------------------------
void
Diag335Collector::displaySegment(const Indicator& tktFareData,
                                 const Indicator& owrt,
                                 const Indicator& seasonType,
                                 const Indicator& dowType,
                                 const std::string gd,
                                 const CarrierCode& carrier,
                                 const RuleNumber& ruleNumber,
                                 const TariffNumber& ruleTariff,
                                 const FareType& fareType,
                                 const FareClassCode& fareClass,
                                 const LocCode& betwCity,
                                 const LocCode& andCity,
                                 const char* axessInd,
                                 std::string& segment)
{
  std::ostringstream oss;
  oss << "  TKT FARE INDICATOR - " << std::setw(1) << tktFareData << "     ";
  oss << "  OWRT INDICATOR     - " << std::setw(1) << owrt << "\n";

  oss << "  SEASON TYPE      " << axessInd << std::setw(1) << seasonType << "     ";
  oss << "  DOW INDICATOR    " << axessInd << std::setw(1) << dowType << "\n";

  oss << "  GLOBAL DIRECTION   - " << std::setw(2) << gd << "    ";
  oss << "  CARRIER CODE       - " << std::setw(2) << carrier << "\n";

  // rule info
  oss << "  RULE NUMBER        - " << std::setw(4) << ruleNumber << "  ";
  oss << "  RULE TARIFF        - " << std::setw(4) << ruleTariff << "\n";

  // fare info
  oss << "  FARE TYPE        " << axessInd << std::setw(4) << fareType << "  ";
  oss << "  FARE CLASS CODE  " << axessInd << std::setw(8) << fareClass << "\n";

  // cities
  oss << "  BTW CITY CODE    " << axessInd << std::setw(3) << betwCity << "   ";
  oss << "  AND CITY CODE    " << axessInd << std::setw(3) << andCity << "\n";

  segment.append(oss.str());
}

//----------------------------------------------------------------------------
// displays failCode and some details from negFarerest
//
// @param  negFareRest   A reference to the NegFareRest object
// @param  failCode      A failed reason code. Value R3_PASS if passes.
//----------------------------------------------------------------------------
void
Diag335Collector::displayFailCode(const NegFareRest& negFareRest, const char* failCode)
{
  if (_active)
  {
    std::ostringstream oss;
    if (failCode == Diag335Collector::R3_PASS)
    {
      // use existing operator
      *this << negFareRest;
    }
    else
    {
      // most important R3 info

      oss << "R3:" << std::setw(8) << negFareRest.itemNo();
      oss << "PTC-" << std::setw(5) << negFareRest.psgType();
      oss << "T994-" << std::setw(8) << negFareRest.overrideDateTblItemNo();

      // Carrier
      if (!negFareRest.carrier().empty())
      {
        oss << (negFareRest.tktAppl() == ' ' ? "CXR - " : "CXR NOT - ");
        oss << std::setw(2) << negFareRest.carrier();
      }

      // footer
      oss << "\n     *FAIL* " << failCode << "\n";
    }
    _itemString.append(oss.str());
  }
}

//----------------------------------------------------------------------------
// printHeader
//----------------------------------------------------------------------------
void
Diag335Collector::displayMessage(const std::string& msg, bool showDiag)
{
  if (_active)
  {
    std::ostringstream oss;
    oss << msg;

    _itemString.append(oss.str());
    if (showDiag)
    {
      *this << _itemString << "\n"; // Move Fare item to DIAG
    }
  }
}

void
Diag335Collector::displayRelation(const CategoryRuleItemInfo* rule, Record3ReturnTypes statusRule)
{
  if (_active)
  {
    displayRec2(*rule);

    std::ostringstream oss;

    oss << std::setw(8) << getStatusString(statusRule);

    oss << std::endl;
    _itemString.append(oss.str());
  }
}
}
