//----------------------------------------------------------------------------
//  File:        Diag868Collector.C
//  Authors:
//  Created:
//
//  Description: Diagnostic 868 Fare RETAIL Rule
//
//  Updates:
//          06/24/14 - fareRETAIL rule
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

#include "Diagnostic/Diag868Collector.h"

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
#include "DBAccess/FareFocusAccountCdInfo.h"
#include "DBAccess/FareFocusAccountCdDetailInfo.h"
#include "DBAccess/FareFocusBookingCodeInfo.h"
#include "DBAccess/FareFocusCarrierInfo.h"
#include "DBAccess/FareFocusFareClassInfo.h"
#include "DBAccess/FareFocusLocationPairInfo.h"
#include "DBAccess/FareFocusSecurityInfo.h"
#include "DBAccess/FareFocusSecurityDetailInfo.h"
#include "DBAccess/FareFocusRuleCodeInfo.h"
#include "DBAccess/FareRetailerCalcInfo.h"
#include "DBAccess/FareRetailerCalcDetailInfo.h"
#include "DBAccess/FareRetailerResultingFareAttrInfo.h"
#include "DBAccess/FareRetailerRuleLookupInfo.h"
#include "DBAccess/FareRetailerRuleInfo.h"
#include "DBAccess/FareFocusDisplayCatTypeInfo.h"
#include "DBAccess/FareFocusDaytimeApplDetailInfo.h"
#include "DBAccess/FareFocusDaytimeApplInfo.h"
#include "Diagnostic/DiagnosticUtil.h"
#include "DBAccess/FareFocusPsgTypeInfo.h"
#include "Rules/RuleUtil.h"


#include <iomanip>
#include <iostream>

using namespace std;

namespace tse
{

FALLBACK_DECL(fallbackFRRProcessingRetailerCode);

// ----------------------------------------------------------------------------
// <PRE>
//
// @function void Diag868Collector::printFareMarketHeader
//
// Description:  This method will print the FareMarket Header info
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag868Collector::printFareMarketHeaderFRR(PricingTrx& trx,
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
// @function void Diag868Collector::printPaxTypeFare
//
// Description:  This method will print the PaxTypeFare info
//
// </PRE>
// ----------------------------------------------------------------------------
void
Diag868Collector::printPaxTypeFare(PricingTrx& trx, const PaxTypeFare& paxTypeFare)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::left, std::ios::adjustfield);
  dc << " \n";
  dc << "----------------- NEGOTIATED FARE PROCESSING -------------------\n";
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

  dc << std::setw(2) << DiagnosticUtil::getOwrtChar(paxTypeFare) << '\n';

  std::string fareBasisC = paxTypeFare.createFareBasis(trx, false);
  if (fareBasisC.size() > 12)
    fareBasisC = fareBasisC.substr(0, 12) + "*";
  dc << "  FBCODE-" << std::setw(20) << fareBasisC << "PRIVATE-";
  dc << std::setw(3) << (paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF ? "Y" : "N");
  dc << "TYPE-" << std::setw(7) << paxTypeFare.fcaFareType() << "DCT-" ;
  std::string cat35DCT = " " ;
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

  dc << "  PTC-" << paxTypeFare.fcasPaxType();

  dc << " \n";
  return;
}

void
Diag868Collector::printDiagSecurityHShakeNotFound(const PseudoCityCode& pcc)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;

  dc << "NO FRR PROCESSING FOR PCC " << pcc  << " - SECURITY HANDSHAKE NOT FOUND" << "\n";
}

void
Diag868Collector::printDiagFareRetailerRulesNotFound()
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << " \nNO FARE RETAILER PROCESSING - FARE RETAILER RULES ARE NOT FOUND\n";
}

void
Diag868Collector::printFareRetailerRuleStatus(PricingTrx& trx, StatusFRRuleValidation rc)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "***                  RULE STATUS : ";
  displayStatus(trx, rc);
  dc << " \n";
}

void
Diag868Collector::displayStatus(PricingTrx& trx, StatusFRRuleValidation status)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  // Currently max length for error msg is 15
  switch (status)
  {
  case PASS_FR:
    dc << "MATCH";
    break;
  case FAIL_FR_VENDOR:
    dc << "NOT MATCH VENDOR";
    break;
  case FAIL_FR_GEO:
    dc << "NOT MATCH GEO";
    break;
  case FAIL_FR_DIRECTIONALITY:
    dc << "NOT MATCH DRT";
    break;
  case FAIL_FR_CARRIER:
    dc << "NOT MATCH CARRIER";
    break;
  case FAIL_FR_RULE:
    dc << "NOT MATCH RULE";
    break;
  case FAIL_FR_RULE_TARIFF:
    dc << "NOT MATCH RULE TARIFF";
    break;
  case FAIL_FR_FARE_CLASS:
    dc << "NOT MATCH FARE CLASS";
    break;
  case FAIL_FR_EXCLUDE_FARE_CLASS:
    dc << "MATCH EXCLUDE FARE CLASS";
    break;
  case FAIL_FR_EXCLUDE_GEO:
    dc << "MATCH EXCLUDE GEO";
    break;
  case FAIL_FR_FARE_TYPE:
    dc << "NOT MATCH FARE TYPE";
    break;
  case FAIL_FR_SECURITY:
    dc << "NOT MATCH FR SECURITY";
    break;
  case FAIL_FR_ACCOUNTCD:
    dc << "NOT MATCH ACCOUNT CODE";
    break;
  case FAIL_FR_EXCLUDE_DCT:
    dc << "NOT MATCH EXCLUDE DISPLAY CAT TYPE";
    break;
  case FAIL_FR_PASSENGERTYPECODE:
    dc << "NOT MATCH PTC";
    break;
  case FAIL_FR_BOOKING_CODE:
    dc << "NOT MATCH BOOKING CODE";
    break;
  case FAIL_FR_MATCH_TRAVEL_DATE_RANGE_X5:
      dc << "NOT MATCH TRAVEL DATE RANGE";
    break;
  case FAIL_FR_ALL:
    dc << "FARE RETAILER RULE STATUS- NOT MATCH ANY RULE";
    break;
  case FAIL_FR_PUBLIC_PRIVATE:
    dc << "NOT MATCH PUBLIC PRIVATE";
    break;
  case FAIL_FR_CAT35_DISPLAY_TYPE:
    dc << "NOT MATCH CAT35 DISPLAY TYPE";
    break;
  case FAIL_FR_MATCH_OWRT:
    dc << "NOT MATCH OWRT";
    break;
  case FAIL_FR_MATCH_RETAILERCODE:
    dc << "NOT MATCH RETAILER CODE";
    break;

  default:
    dc << "UNKNOWN STATUS";
    break;
  }
  dc << "\n";
}

void
Diag868Collector::printFareRetailerRuleLookupInfo(const FareRetailerRuleLookupInfo* frrl)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  dc << "FARE RETAILER RULE TYPE : " << frrl->applicationType();
  dc << "  SOURCE PCC : " << frrl->sourcePcc() << "\n";
  for (const auto& lookupID : frrl->fareRetailerRuleLookupIds())
  {
    dc << "  RULE ID:  " << lookupID._fareRetailerRuleId;
    dc << "   SEQNO:  " << lookupID._ruleSeqNo << "\n";
  }
}
void
Diag868Collector::printFareRetailerRuleLookupHeader(const FareRetailerRuleLookupInfo* frrli) const
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "---------------- FARE RETAILER RULE PROCESS --------------------\n";
  dc << "FARE RETAILER RULE TYPE : " << frrli->applicationType();
  dc << "  SOURCE PCC : " << frrli->sourcePcc() << "\n";
}
void
Diag868Collector::printFareRetailerRuleLookupInfo(PricingTrx& trx, const FareRetailerRuleInfo* frri,
                                                  StatusFRRuleValidation rc)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);

  dc << "  RULE ID:  " << frri->fareRetailerRuleId() << "\n";
  dc << "  SEQNO  :  " << frri->ruleSeqNo() << "  STATUS- ";
  displayStatus(trx, rc);
}
void
Diag868Collector::printFareRetailerRuleInfo(PricingTrx& trx,
                                            const FareRetailerRuleInfo* frri,
                                            const FareFocusSecurityInfo* ffsi,
                                            bool  ddAll)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(ios::left, ios::adjustfield);
  dc << "---------------- FARE RETAILER RULE DATA  ----------------------\n";
  dc << "APPLICATION IND      : " << frri->applicationInd() << "\n";
  dc << "EFF DATE             : " << frri->effDate().toIsoExtendedString()  << "\n";
  dc << "DISC DATES           : " << frri->discDate().toIsoExtendedString() << "\n";
  dc << "STATUS               : " << frri->statusCd() << "\n";
  dc << "FARE RET RULE ID     : " << frri->fareRetailerRuleId() << "\n";
  dc << "SEQUENCE NUMBER      : " << frri->ruleSeqNo() << "\n";
  dc << "SOURCE PCC           : " << frri->sourcePseudoCity() << "\n";
  dc << "VENDOR CODE          : " << frri->vendor() << "\n";
  dc << "RULE TARIFF          : " << frri->ruleTariff() << "\n";
  dc << "FARE TYPE            : " << frri->fareType() << "\n";

  std::string owrt = " ";
  if (frri->owrt() == ONE_WAY_MAY_BE_DOUBLED)
    owrt = "X";
  else if (frri->owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    owrt = "R";
  else if (frri->owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
    owrt = "O";
  else if (frri->owrt() == ANY_ONE_WAY)
    owrt = "X OR O";
  dc << "OWRT INDICATOR       : " << owrt << "\n";

  std::string dct = " ";
  if (frri->displayCatType() == 'C')
     dct = "C";
  if (frri->displayCatType() == 'L')
    dct = "L";
  if (frri->displayCatType() == 'T')
    dct = "T";
  if (frri->displayCatType() == 'Q')
    dct = "L OR T";
  dc << "CAT35 DISPLAY TYPE   : " << dct << "\n";
  dc << "FARE DCT TABLE       : " << frri->displayCatTypeExcludeItemNo() << "\n";
  if (ddAll && frri->displayCatTypeExcludeItemNo() != 0)
  {
    printDispayCatTypeInfo(trx, frri, dc);
  }
  char Ind = frri->publicPrivateInd();
  if (Ind == 'P')
  dc << "PUBLIC INDICATOR     : " << Ind << "\n";
  else if (Ind == 'V')
  dc << "PUBLIC INDICATOR     : " << Ind << "\n";
  else
  dc << "PUBLIC INDICATOR     : " << Ind << "\n";

  dc << "GEO LOC1/LOC2        : " <<frri->loc1().locType() << " " << frri->loc1().loc() ;
  dc << "   " << frri->loc2().locType() << " " << frri->loc2().loc() << "\n";
  dc << "DIRECTIONALITY       : " << frri->directionality() << "\n";
  dc << "EXCLUDE LOC PAIR     : " << frri->locationPairExcludeItemNo() << "\n";
  std::string globalDirStr;
  globalDirectionToStr(globalDirStr, frri->globalDir());
  dc << "GLOBAL DIRECTION     : " << globalDirStr << "\n";

  dc << "CARRIER TABLE        : " << frri->carrierItemNo() << "\n";
  if (ddAll && frri->carrierItemNo() != 0)
    printCarriersTableInfo(trx, frri, dc);

  dc << "RULE CODE TABLE      : " << frri->ruleCdItemNo() << "\n";
  if (ddAll && frri->ruleCdItemNo() != 0)
    printRuleCodeTableInfo(trx, frri, dc);

  dc << "SECURITY TABLE       : " << frri->securityItemNo() << "\n";
  if (ddAll && frri->securityItemNo() != 0)
    printSecurityTableInfo(ffsi, dc);

  dc << "FARE CLASS TABLE     : " << frri->fareClassItemNo() << "\n";
  if (ddAll && frri->fareClassItemNo() != 0)
    printFareClassTableInfo(trx, frri, dc);

  dc << "EXCLUDE FARE CLASS   : " << frri->fareClassExcludeItemNo() << "\n";
  dc << "POS DAY TIME         : " << frri->posDayTimeApplItemNo() << "\n";
  dc << "ROUTING TABLE        : " << frri->routingItemNo() << "\n";
  dc << "TKT DESIGNATOR TABLE : " << frri->routingItemNo() << "\n";
  dc << "RESULTING TABLE      : " << frri->resultingFareAttrItemNo() << "\n";
  if (ddAll && frri->resultingFareAttrItemNo() != 0)
    printResultingTableInfo(trx, frri, dc);

  dc << "CALCULATION TABLE    : " << frri->fareRetailerCalcItemNo() << "\n";
  if (ddAll && frri->fareRetailerCalcItemNo() != 0)
    printCalculatingTableInfo(trx, frri, dc);

  dc << "BOOKING CODE TABLE   : " << frri->bookingCdItemNo() << "\n";
  if (ddAll && frri->bookingCdItemNo() != 0)
    printBookingCodeTableInfo(trx, frri, dc);

  dc << "PAXTYPE CODE TABLE   : " << frri->psgTypeItemNo() << "\n";
  if (ddAll && frri->psgTypeItemNo() > 0)
    printPaxTypeCode(trx, frri, dc);

  dc << "ACCOUNT CODE TABLE   : " << frri->accountCdItemNo() << "\n";
  if (ddAll && frri->accountCdItemNo() != 0)
    printAccountCodeTableInfo(trx, frri, dc);

  dc << "TRAVEL RANGE TABLE   : " << frri->travelDayTimeApplItemNo() << "\n";
  if (ddAll && frri->travelDayTimeApplItemNo() != 0)
    printMatchTravelRangeX5(trx, frri, dc);

  dc << "EXCLUDE LOC PAIR     : " << frri->locationPairExcludeItemNo() << "\n";
  if (ddAll && frri->locationPairExcludeItemNo() != 0)
    printLocationPairExcludeInfo(trx, frri, dc);

  printRetailerCodeInfo(trx, frri, dc);
}

void
Diag868Collector::printRetailerCodeInfo(PricingTrx& trx,
                                           const FareRetailerRuleInfo* frri,
                                           DiagCollector& dc) const
{
  if (!_active)
    return;

  if (!fallback::fallbackFRRProcessingRetailerCode(&trx))
    dc << "RETAILER CODE        : " << frri->fareRetailerCode() << "\n";
}

void
Diag868Collector::printLocationPairExcludeInfo(PricingTrx& trx,
                                               const FareRetailerRuleInfo* frri,
                                               DiagCollector& dc) const
{
  if (!_active)
    return;

  const FareFocusLocationPairInfo* fareFocusLocationPairInfo =
    trx.dataHandle().getFareFocusLocationPair(frri->locationPairExcludeItemNo(),
                                              trx.dataHandle().ticketDate());

  if (!fareFocusLocationPairInfo || fareFocusLocationPairInfo->details().empty())
    return;

  int counter = fareFocusLocationPairInfo->details().size();

  dc << "  * TABLE DATA       : ";
  for (auto fflpdi : fareFocusLocationPairInfo->details())
  {
    dc << fflpdi->loc1().locType() << " " << fflpdi->loc1().loc() << "   "
       << fflpdi->loc2().locType() << " " << fflpdi->loc2().loc() << "\n";

    if (--counter != 0)
      dc << "                     : ";
  }
}

void
Diag868Collector::printCarriersTableInfo(PricingTrx& trx, const FareRetailerRuleInfo* frri, DiagCollector& dc) const
{
  if (!_active)
    return;

  const std::vector<FareFocusCarrierInfo*> ruleCarriers =
        trx.dataHandle().getFareFocusCarrier(frri->carrierItemNo(),
        trx.dataHandle().ticketDate());

  if (ruleCarriers.empty())
    return;

  dc << "  * TABLE DATA       : ";
  for (CarrierCode carrierRule : ruleCarriers.front()->carrier())
  {
    dc << carrierRule << " ";
  }
  dc << "\n";
}

void
Diag868Collector::printDispayCatTypeInfo(PricingTrx& trx, const FareRetailerRuleInfo* frri, DiagCollector& dc) const
{
  if (!_active)
    return;

  const FareFocusDisplayCatTypeInfo* ffDCT =
      trx.dataHandle().getFareFocusDisplayCatType(frri->displayCatTypeExcludeItemNo(),
                                              trx.dataHandle().ticketDate());

  if (ffDCT == nullptr || (ffDCT->displayCatType()).empty())
     return;

  dc << "  * TABLE DATA       : ";

  const std::vector<Indicator>& dctVec = ffDCT->displayCatType();
  for (const Indicator& dctFRR : dctVec)
  {
    dc << dctFRR << " ";
  }
  dc << "\n";
}

void
Diag868Collector::printRuleCodeTableInfo(PricingTrx& trx, const FareRetailerRuleInfo* frri, DiagCollector& dc) const
{
  if (!_active)
    return;

  const std::vector<FareFocusRuleCodeInfo*>& ffrciV =
        trx.dataHandle().getFareFocusRuleCode(frri->ruleCdItemNo(),
        trx.dataHandle().ticketDate());

  if (ffrciV.empty())
     return;

  dc << "  * TABLE DATA       : ";
  for (RuleNumber rn : ffrciV.front()->ruleCd())
  {
    dc << rn << " ";
  }
  dc << "\n";
}

void
Diag868Collector::printSecurityTableInfo(const FareFocusSecurityInfo* ffsi, DiagCollector& dc) const
{
  if (!_active)
    return;

  dc << "  * TABLE DATA       : ";
  int counter =0 ;
  for (FareFocusSecurityDetailInfo* ffsdi : ffsi->details())
  {
    dc << ffsdi->pseudoCityType() << "-" << ffsdi->pseudoCity() << " ";
    counter ++;
    if (counter == 5)
    {
      dc << "\n" << "                     : ";
      counter = 0;
    }
  }
  dc << "\n";
}

void
Diag868Collector::printMatchTravelRangeX5(PricingTrx& trx,
                                          const FareRetailerRuleInfo* frri,
                                          DiagCollector& dc) const
{
  if (!_active)
    return;

  const FareFocusDaytimeApplInfo* ffdtInfo =
  trx.dataHandle().getFareFocusDaytimeAppl(frri->travelDayTimeApplItemNo(), trx.dataHandle().ticketDate());
  std::vector<FareFocusDaytimeApplDetailInfo*> ffDetails = ffdtInfo->details();
  if (ffDetails.empty())
    return;

  int counter = ffDetails.size();

  dc << "  * TABLE DATA       : ";
  for(auto dtls: ffDetails)
  {
    dc << dtls->startDate().dateToSqlString()<< " - " << dtls->stopDate().dateToSqlString();
    if (--counter != 0)
    dc <<"\n" << "                     : ";
  }
  dc <<"\n";
}

void
Diag868Collector::printFareClassTableInfo(PricingTrx& trx, const FareRetailerRuleInfo* frri, DiagCollector& dc) const
{
  if (!_active)
    return;

  const std::vector<FareFocusFareClassInfo*>& ffFareClassInfoVec =
        trx.dataHandle().getFareFocusFareClass(frri->fareClassItemNo(),
        trx.dataHandle().ticketDate());

  if (ffFareClassInfoVec.empty())
    return;

  dc << "  * TABLE DATA        : ";
  for (const FareFocusFareClassInfo* ffClassInfo : ffFareClassInfoVec)
  {
    int counter =0 ;
    for (const FareClassCodeC& fareClassCode : ffClassInfo->fareClass())
    {
      dc << fareClassCode << " ";
      if (counter == 3)
      {
        dc << "\n" << "                     : ";
        counter = 0;
      }
    }
  }
  dc << "\n";
}

void
Diag868Collector::printResultingTableInfo(PricingTrx& trx, const FareRetailerRuleInfo* frri, DiagCollector& dc) const
{
  if (!_active)
    return;

  const FareRetailerResultingFareAttrInfo* frResultingFareAttr =
        trx.dataHandle().getFareRetailerResultingFareAttr(frri->resultingFareAttrItemNo(),
                                                           trx.dataHandle().ticketDate());

  dc << "  * TABLE DATA       :";
  dc << " RDST- " << frResultingFareAttr->redistributeInd();
  dc << " UPDATE- " << frResultingFareAttr->sellInd();
  dc << " SELL- " << frResultingFareAttr->updateInd();
  dc << " VNET- " << frResultingFareAttr->viewNetInd() << "\n";
  dc << "                     : TKT DESIGNATOR- " << frResultingFareAttr->tktDesignator() << "\n";
  dc << "                     : TKT INDICATOR- " << frResultingFareAttr->ticketInd() << "\n";
  dc << "                     : ACCOUNT TYPE- " << frResultingFareAttr->accountCdType();
  dc << " CODE- " << frResultingFareAttr->accountCd();
  dc << "\n";
}

void
Diag868Collector::printCalculatingTableInfo(PricingTrx& trx, const FareRetailerRuleInfo* frri, DiagCollector& dc) const
{
  if (!_active)
    return;

  const std::vector<FareRetailerCalcInfo*>& frFareCalculationVec =
        trx.dataHandle().getFareRetailerCalc(frri->fareRetailerCalcItemNo(),
        trx.dataHandle().ticketDate());

  if (frFareCalculationVec.empty())
        return;

  for (const FareRetailerCalcInfo* frCalcInfo : frFareCalculationVec)
  {
    dc << "  * TABLE DATA       :";
    for (const FareRetailerCalcDetailInfo* frCalcDetail : frCalcInfo->details())
    {
      dc << " CALCULATION TYPE- " << frCalcDetail->calculationTypeCd();
      dc << " CALC IND- " << frCalcDetail->fareCalcInd() << "\n";
      dc << "                     : PRC1- " << frCalcDetail->percent1() << "\n";
      dc << "                     : PRC1 MIN1- " << frCalcDetail->percentMin1();
      dc << " PRC1 MAX1- " << frCalcDetail->percentMax1() << "\n";
      dc << "                     : AMT1- " << frCalcDetail->amount1();
      dc << " " << frCalcDetail->amountCurrency1() << "\n";
      dc << "                     : AMT MIN1- " << frCalcDetail->amountMin1();
      dc << " AMT MAX1-  " << frCalcDetail->amountMax1();
      dc << " " << frCalcDetail->amountCurrency1() << "\n";
      dc << "                     : PRC2- " << frCalcDetail->percent2() << "\n";
      dc << "                     : PRC2 MIN2- " << frCalcDetail->percentMin2();
      dc << " PRC2 MAX2- " << frCalcDetail->percentMax2() << "\n";
      dc << "                     : AMT2- " << frCalcDetail->amount2();
      dc << " " << frCalcDetail->amountCurrency2() << "\n";
      dc << "                     : AMT MIN2- " << frCalcDetail->amountMin2();
      dc << " AMT MAX2- " << frCalcDetail->amountMax2();
      dc << " " << frCalcDetail->amountCurrency2();
    }
  }
  dc << "\n";
}

void
Diag868Collector::printAccountCodeTableInfo(PricingTrx& trx,
                                            const FareRetailerRuleInfo* frri,
                                            DiagCollector& dc) const
{
  if (!_active)
    return;

  const FareFocusAccountCdInfo* fareFocusAccountCdInfo =
    trx.dataHandle().getFareFocusAccountCd(frri->accountCdItemNo(), trx.dataHandle().ticketDate());

  if (fareFocusAccountCdInfo == nullptr)
    return;

  if (fareFocusAccountCdInfo->details().empty())
    return;

  dc << "  * TABLE DATA       : ";
  int counter = fareFocusAccountCdInfo->details().size();
  for (FareFocusAccountCdDetailInfo* ffacdi : fareFocusAccountCdInfo->details())
  {
    dc << ffacdi->accountCdType() << "- " << ffacdi->accountCd();
    if (--counter != 0)
      dc << "\n" << "                     : ";
  }

  dc << "\n";
}

void
Diag868Collector::printPaxTypeFareAccountCode(const AccountCode& accountCode)
{
  if (!_active)
    return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc << "PAXTYPE FARE CORP ID : " << accountCode << "\n";
}

void
Diag868Collector::printPaxTypeCode(PricingTrx& trx,
                                  const FareRetailerRuleInfo* frri,
                                  DiagCollector& dc)
{
  if (!_active)
    return;

  const FareFocusPsgTypeInfo* ffpti =
     trx.dataHandle().getFareFocusPsgType(frri->psgTypeItemNo(), trx.dataHandle().ticketDate());

  dc << "  * TABLE DATA       : ";
  if (ffpti != nullptr && ffpti->psgType().size() > 0)
  {
    const std::vector<PaxTypeCode>& ptcV = ffpti->psgType();
    for (auto code : ptcV)
    {
      dc << code << " ";
    }
  }
  dc << "\n";
}

void
Diag868Collector::printBookingCodeTableInfo(PricingTrx& trx, const FareRetailerRuleInfo* frri, DiagCollector& dc) const
{
  if (!_active)
    return;

  const std::vector<FareFocusBookingCodeInfo*> bookincodes =
        trx.dataHandle().getFareFocusBookingCode(frri->bookingCdItemNo(),
        trx.dataHandle().ticketDate());

  if (bookincodes.empty())
    return;
  int counter =0 ;
  dc << "  * TABLE DATA       : ";
  for (const BookingCode bookingCode : bookincodes.front()->bookingCode())
  {
    dc << bookingCode << " ";
    if (counter == 6)
    {
      dc << "\n" << "                     : ";
      counter = 0;
    }
  }
  dc << "\n";
}

void
Diag868Collector::printCalculation(PricingTrx& trx,
                                PaxTypeFare* ptf,
                                AdjustedFareCalc& calcObj,
                                const FareRetailerCalcDetailInfo* calcDetail,
                                MoneyAmount currAmt,
                                MoneyAmount currAmtNuc,
                                const FareRetailerRuleContext& context)
{
  if (!_active)
      return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::fixed, std::ios::floatfield);
  dc << "\n------- ADJUSTED SELLING FARE CALCULATIONS  --------\n";

  if (ptf != nullptr)
  {
    dc << "FARE CLASS:" << ptf->createFareBasis(trx, true);
    dc << "  ";
    dc << "SOURCE PCC:" << context._sourcePcc;
    dc << "  ";

    if (context._frri != nullptr)
    {
      dc << "FRR ID:" << (context._frri)->fareRetailerRuleId() << "\n";
    }
  }
  if (calcDetail != nullptr)
  {
    MoneyAmount precent = 0;
    MoneyAmount amount = 0;
    int noDecAmt = 0;
    int noDecPercent = 0;
    calcObj.getSellingInfo(precent, amount, noDecAmt, noDecPercent);
    dc.precision(2);
    dc << "  CALC DATA: " << "IND: " << " " << calcDetail->fareCalcInd();
    dc << "  ";
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc << "  PERCENT: " << precent;
    dc << "  ";
    dc << "  AMOUNT : " << Money(amount, calcObj.getCurrencyCode());
    dc << "\n";

    dc << "  CALCULATED FARE AMOUNT: " << Money(currAmt, (*ptf).currency()) << "\n";
    dc << "  CALCULATED NUC AMOUNT : " << currAmtNuc << " NUC " << "\n";
  }
}

void
Diag868Collector::printFareCreation(PricingTrx* trx,
                                PaxTypeFare* ptf) const
{
  if (!_active)
      return;

  if (ptf != nullptr)
  {
    DiagCollector& dc = (DiagCollector&)*this;
    dc.setf(std::ios::fixed, std::ios::floatfield);

    dc << "CREATED PAX TYPE FARE  : " << ptf->createFareBasis(trx, true) << "\n";
    dc.setf(std::ios_base::floatfield, std::ios::adjustfield);
    dc << "  BASE AMOUNT          : " << Money(ptf->fareAmount(), (*ptf).currency()) << "\n";
    dc << "  ORIGINAL FARE AMOUNT : " << Money(ptf->originalFareAmount(), (*ptf).currency()) << "\n";
    dc.precision(2);
    dc << "  NUC AMOUNT           : " << ptf->nucFareAmount() << " NUC \n";
    dc << "  ORIGINAL NUC AMOUNT  : " << ptf->nucOriginalFareAmount() << " NUC \n";
  }
}

void
Diag868Collector::printMinimumAmount(PricingTrx* trx,
                                PaxTypeFare* ptf,
                                AdjustedSellingCalcData* calcData) const
{
  if (!_active)
      return;

  DiagCollector& dc = (DiagCollector&)*this;
  dc.setf(std::ios::fixed, std::ios::floatfield);
  if (ptf != nullptr)
  {
    dc.setf(std::ios::fixed, std::ios::floatfield);
    dc << "\n--- MINIMUM AMOUNT SELECTED FOR ADJUSTED SELLING FARE ---\n";

    dc << "FARE CLASS: " << ptf->createFareBasis(trx, true);
    dc << "  ";
    if (calcData != nullptr)
    {
      dc << "SOURCE PCC: " << calcData->getSourcePcc();
      dc << "  ";
      dc << "FRR ID: " << calcData->getFareRetailerRuleId() << "\n";
      dc.setf(std::ios_base::floatfield, std::ios::adjustfield);
      dc.precision(2);
      dc << "  BASE AMOUNT     : " << Money(calcData->getCalculatedAmt(), (*ptf).currency()) << "\n";
      dc << "  NUC FARE AMOUNT : " << calcData->getCalculatedNucAmt() << " NUC \n";
    }
  }
}

} // namespace
