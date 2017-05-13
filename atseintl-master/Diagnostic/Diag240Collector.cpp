//----------------------------------------------------------------------------
//  File:        Diag240Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 240 Fare Focus Rule
//
//  Updates:
//          06/24/14 - fareFocus rule
//
//  Copyright Sabre 2009
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

#include "Diagnostic/Diag240Collector.h"

#include "Common/FareMarketUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseBoostStringTypes.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DataModel/ExchangePricingTrx.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexExchangeTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/FareFocusRuleInfo.h"


#include <iomanip>
#include <iostream>

using namespace std;

namespace tse
{

FIXEDFALLBACK_DECL(fallbackFFRaddFocusCodeInFFR);

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag240Collector::printFareMarketHeader
//
// Description:  This method will print the FareMarket Header info
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag240Collector::printFareMarketHeaderFFR(PricingTrx& trx,
                                           const FareMarket& fm)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "FARE MARKET : \n";
  dc << " \n" << FareMarketUtil::getFullDisplayString(fm) << "\n";
  dc << "TICKETING DATE : " << trx.dataHandle().ticketDate(). toIsoExtendedString() << "\n";
  dc << " \n";
  return;
}

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag240Collector::printPaxTypeFare
//
// Description:  This method will print the PaxTypeFare info
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag240Collector::printPaxTypeFare(PricingTrx& trx, const PaxTypeFare& paxTypeFare)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " \n";
  dc << "----------------- FARE PROCESSING -----------------------------\n";
  dc << "FARE-" << std::setw(10) << paxTypeFare.fareClass();
  dc <<  std::setw(8) << Money(paxTypeFare.fareAmount(), paxTypeFare.currency()) << "   ";
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc.precision(2);
  dc << std::setw(8) << paxTypeFare.nucFareAmount() << "NUC" << '\n';

  CarrierCode carrier = paxTypeFare.carrier();
  if (paxTypeFare.fare()->isIndustry())
    carrier = INDUSTRY_CARRIER;

  dc << "  VENDOR-" << std::setw(8) << paxTypeFare.vendor() << "CARRIER-" << std::setw(4) << carrier ;
  dc << "RULE-" << std::setw(6) << paxTypeFare.ruleNumber() << "TARIFF-" ;
  dc <<  std::setw(5) << paxTypeFare.tcrRuleTariff() << "OWRT-";

  std::string owrt = " ";
  if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
    owrt = "X";
  else if (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    owrt = "R";
  else if (paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
    owrt = "O";
  else
    owrt = " ";
  dc <<  std::setw(2) << owrt << '\n';

  std::string fareBasisC = paxTypeFare.createFareBasis(trx, false);
  if (fareBasisC.size() > 12)
    fareBasisC = fareBasisC.substr(0, 12) + "*";
  dc << "  FBCODE-" << std::setw(20) << fareBasisC << "PRIVATE-";
  dc << std::setw(3) << (paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF ? "Y" : "N");
  dc << "TYPE-" << std::setw(7) << paxTypeFare.fcaFareType() << "CAT35-" ;
  std::string cat35DCT = " " ;
  if (paxTypeFare.isNegotiated())
    cat35DCT = paxTypeFare.fcaDisplayCatType();
  dc << std::setw(2) << cat35DCT << '\n';

  dc << "  FARE STATUS-" << std::setw(2) << cnvFlags(paxTypeFare);
  dc << " PRIME RBD-" ;
  vector<BookingCode> bkgCodes;
  paxTypeFare.getPrimeBookingCode(bkgCodes);
  if (!bkgCodes.empty())
  {
    std::string bc = " " ;
    for (BookingCode bc : bkgCodes)
      dc << " " << bc;
  }
  dc << " \n";

  std::string dir;
  if (paxTypeFare.directionality() == TO)
    dir = " TO ";
  else if (paxTypeFare.directionality() == FROM)
    dir = " FROM ";
  else
    dir = "      ";
  LocCode origMarket = (paxTypeFare.isReversed()) ? paxTypeFare.fare()->market2() : paxTypeFare.fare()->market1();

  if (paxTypeFare.directionality() == TO || paxTypeFare.directionality() == FROM)
    dir += origMarket;
  dc << "  DIR-" <<std::setw(9) << dir << '\n';

  dc << " \n";
  return;
}

void
Diag240Collector::printDiagSecurityHShakeNotFound()
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << " \nNO FARE FOCUS PROCESSING - SECURITY HANDSHAKE NOT FOUND\n";
}

void
Diag240Collector::printDiagFareFocusRulesNotFound()
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << " \nNO FARE FOCUS PROCESSING - FARE FOCUS RULES ARE NOT FOUND\n";
}

void
Diag240Collector::printFareFocusRuleNoneFound()
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "  FARE FOCUS RULE STATUS- NOT MATCH ANY RULE";
  dc << "\n";
}

void
Diag240Collector::printFareFocusLookup(const uint64_t fareFocusRuleId, StatusFFRuleValidation rc)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "  FARE FOCUS RULE ID: " << fareFocusRuleId << "  STATUS- ";
  displayStatus(rc);
}

void
Diag240Collector::printFareFocusRuleStatus(StatusFFRuleValidation rc)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc << "***                  RULE STATUS : ";
  displayStatus(rc);
  dc << " \n";
}

void
Diag240Collector::printFareFocusLookup(StatusFFRuleValidation rc)
{
  if (!_active)
    return;
//  DiagCollector& dc = (DiagCollector&)*this;
  displayStatus(rc);
}

void
Diag240Collector::displayStatus(StatusFFRuleValidation status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  // Currently max length for error msg is 15
  switch (status)
  {
  case PASS_FF:
    dc << "MATCH";
    break;
  case FAIL_FF_VENDOR:
    dc << "NOT MATCH VENDOR";
    break;
  case FAIL_FF_GEO:
    dc << "NOT MATCH GEO";
    break;
  case FAIL_FF_DIRECTIONALITY:
    dc << "NOT MATCH DRT";
    break;
  case FAIL_FF_CARRIER:
    dc << "NOT MATCH CARRIER";
    break;
  case FAIL_FF_SOURCE_PCC:
    dc << "NOT MATCH SOURCE";
    break;
  case FAIL_FF_RULE:
    dc << "NOT MATCH RULE";
    break;
  case FAIL_FF_RULE_TARIFF:
    dc << "NOT MATCH RULE TARIFF";
    break;
  case FAIL_FF_FARE_CLASS:
    dc << "NOT MATCH FARE CLASS";
    break;
  case FAIL_FF_FARE_TYPE:
    dc << "NOT MATCH FARE TYPE";
    break;
  case FAIL_FF_BOOKING_CODE:
    dc << "NOT MATCH BOOKING CODE";
    break;
  case FAIL_FF_OWRT:
    dc << "NOT MATCH OWRT";
    break;
  case FAIL_FF_DCT:
    dc << "NOT MATCH CAT35 DCT";
    break;
  case FAIL_FF_PUBLIC_IND:
    dc << "NOT MATCH PUBLIC";
    break;
  case FAIL_FF_LOOKUP_EMPTY:
    dc << "*** FARE FOCUS LOOKUP IS EMPTY";
    break;
  case FAIL_FF_RULES_EMPTY:
    dc << "*** FARE FOCUS RULES ARE EMPTY";
    break;
  case FAIL_FF_RULES_SECURITY_HANDSHAKE:
    dc << "NOT MATCH SECURITY HANDSHAKE";
    break;
  case FAIL_FF_SECURITY:
    dc << "NOT MATCH FF SECURITY";
    break;
  case FAIL_FF_MATCH_TRAVEL_DATE_RANGE_X5:
    if (!fallback::fixed::fallbackFFRaddFocusCodeInFFR())
      dc << "NOT MATCH TRAVEL DATE";
    break;

  default:
    dc << "UNKNOWN STATUS";
    break;
  }
  dc << "\n";
}

void
Diag240Collector::printFareFocusRuleInfo(const FareFocusRuleInfo* ffri)
{
  if (!_active)
    return;
  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  dc << "--------------- FARE FOCUS RULE DATA  -------------------------\n";
  dc << "FARE FOCUS RULE ID : " << ffri->fareFocusRuleId() << "\n";
  dc << "EFF/DISC DATES     : " << ffri->effDate().toIsoExtendedString() << "  " << ffri->discDate().toIsoExtendedString() << "\n";
  dc << "STATUS             : " << ffri->statusCode() << "\n";
  dc << "SOURCE PCC         : " << ffri->sourcePCC() << "\n";
  dc << "VENDOR CODE        : " << ffri->vendor() << "\n";
  dc << "CARRIER CODE       : " << ffri->carrier() << "\n";
  dc << "RULE               : " << ffri->ruleCode() << "\n";
  dc << "RULE TARIFF        : " << ffri->ruleTariff() << "\n";
  dc << "FARE TYPE          : " << ffri->fareType() << "\n";

  std::string owrt = " ";
  if (ffri->owrt() == ONE_WAY_MAY_BE_DOUBLED)
    owrt = "X";
  else if (ffri->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    owrt = "R";
  else if (ffri->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
    owrt = "O";
  dc << "OWRT INDICATOR     : " << owrt << "\n";

  std::string dct = " ";
  if (ffri->displayType() == 'L')
    dct = "L";
  if (ffri->displayType() == 'T')
    dct = "T";
  if (ffri->displayType() == 'Q')
    dct = "L OR T";
  dc << "CAT35 DISPLAY TYPE : " << dct << "\n";
  dc << "PUBLIC INDICATOR   : " << ffri->publicPrivateIndicator() << "\n";

  dc << "GEO LOC1/LOC2      : " <<ffri->loc1().locType() << " " << ffri->loc1().loc() ;
  dc << "   " << ffri->loc2().locType() << " " << ffri->loc2().loc() << "\n";
  dc << "DIRECTIONALITY     : " << ffri->directionality() << "\n";

  dc << "SECURITY TABLE     : " << ffri->securityItemNo() << "\n";
  dc << "FARE CLASS TABLE   : " << ffri->fareClassItemNo() << "\n";
  dc << "BOOKING CLASS TABLE: " << ffri->bookingCodeItemNo() << "\n";
  dc << "TRAVEL RANGE TABLE : " << ffri->travelDayTimeApplItemNo() << "\n";
}

} // namespace
