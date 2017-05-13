//----------------------------------------------------------------------------
//
//  File:  GenerateRuleRequestFormatter.cpp
//  Description: See GenerateRuleRequestFormatter.h file
//  Created:  May 24, 2005
//  Authors:  Mike Carroll
//
//  Copyright Sabre 2003
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

#include "RTG/GenerateRuleRequestFormatter.h"

#include "Common/ErrorResponseException.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/Money.h"
#include "DataModel/Agent.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/FareByRuleCtrlInfo.h"
#include "DBAccess/CombinabilityRuleInfo.h"
#include "DBAccess/GeneralFareRuleInfo.h"

#include <algorithm>
#include <vector>

namespace tse
{
static Logger
logger("atseintl.RTG.GenerateRuleRequestFormatter");

Logger& GenerateRuleRequestFormatter::getLogger()
{
  return logger;
}

void
GenerateRuleRequestFormatter::addAGIType(const Agent& agent,
                                         const Billing& billing,
                                         XMLConstruct& construct,
                                         FareDisplayTrx& trx)
{
  construct.openElement("AGI");
  construct.addAttribute("A20", agent.tvlAgencyPCC());
  construct.addAttribute("A21", agent.mainTvlAgencyPCC());
  construct.addAttribute("AB0", agent.tvlAgencyIATA());
  construct.addAttribute("AB1", agent.homeAgencyIATA());
  construct.addAttribute("A90", agent.agentFunctions());
  construct.addAttribute("N0G", agent.agentDuty());
  construct.addAttribute("A80", agent.airlineDept());
  construct.addAttribute("B00", agent.cxrCode());
  construct.addAttribute("C40", agent.currencyCodeAgent());
  // construct.addAttribute("A10", agent.agentCity());

  if (!trx.getRequest()->salePointOverride().empty())
    construct.addAttribute("A10", trx.getRequest()->salePointOverride());
  else
    construct.addAttribute("A10", agent.agentCity());

  char tmpBuf[10];
  sprintf(tmpBuf, "%d", agent.coHostID());
  construct.addAttribute("Q01", tmpBuf);

  construct.addAttribute("AE0", billing.partitionID());
  construct.closeElement();
}

//----------------------------------------------------------------------------
// GenerateRuleRequestFormatter::addRCSHeader (all sigs should behave the same)
//----------------------------------------------------------------------------
void
GenerateRuleRequestFormatter::addRCSHeader(const GeneralFareRuleInfo* ruleInfo,
                                           XMLConstruct& construct)
{
  construct.addAttributeChar("S33", ruleInfo->loc1().locType());
  construct.addAttribute("S34", ruleInfo->loc1().loc());
  construct.addAttributeChar("S35", ruleInfo->loc2().locType());
  construct.addAttribute("S36", ruleInfo->loc2().loc());
  construct.addAttribute("B00", ruleInfo->carrierCode());
  construct.addAttribute("D06", ruleInfo->effDate().dateToString(YYYYMMDD, "-").c_str());
  construct.addAttribute("D05", ruleInfo->discDate().dateToString(YYYYMMDD, "-").c_str());
  // International
  construct.addAttribute("P95", !isRecordScopeDomestic() ? "true" : "false");
  construct.addAttribute("N0W", "0");
  construct.addAttribute("B50", ruleInfo->fareClass().c_str());
  construct.addAttributeChar("P04", ruleInfo->owrt());
  // construct.addAttributeChar("Q3X", ruleInfo->jointCarrierTblItemNo());
  char tmpBuf[20];
  sprintf(tmpBuf, "%d", ruleInfo->jointCarrierTblItemNo());
  construct.addAttribute("Q3X", tmpBuf);
}
void
GenerateRuleRequestFormatter::addRCSHeader(const FootNoteCtrlInfo* ruleInfo,
                                           XMLConstruct& construct)
{
  construct.addAttributeChar("S33", ruleInfo->loc1().locType());
  construct.addAttribute("S34", ruleInfo->loc1().loc());
  construct.addAttributeChar("S35", ruleInfo->loc2().locType());
  construct.addAttribute("S36", ruleInfo->loc2().loc());
  construct.addAttribute("B00", ruleInfo->carrierCode());
  construct.addAttribute("D06", ruleInfo->effDate().dateToString(YYYYMMDD, "-").c_str());
  construct.addAttribute("D05", ruleInfo->discDate().dateToString(YYYYMMDD, "-").c_str());
  // International
  construct.addAttribute("P95", !isRecordScopeDomestic() ? "true" : "false");
  construct.addAttribute("N0W", "0");
  construct.addAttribute("B50", ruleInfo->fareClass().c_str());
  construct.addAttributeChar("P04", ruleInfo->owrt());
  // construct.addAttributeChar("Q3X", ruleInfo->jointCarrierTblItemNo());
  char tmpBuf[20];
  sprintf(tmpBuf, "%d", ruleInfo->jointCarrierTblItemNo());
  construct.addAttribute("Q3X", tmpBuf);
}
void
GenerateRuleRequestFormatter::addRCSHeader(const CombinabilityRuleInfo* ruleInfo,
                                           XMLConstruct& construct)
{
  construct.addAttributeChar("S33", ruleInfo->loc1().locType());
  construct.addAttribute("S34", ruleInfo->loc1().loc());
  construct.addAttributeChar("S35", ruleInfo->loc2().locType());
  construct.addAttribute("S36", ruleInfo->loc2().loc());
  construct.addAttribute("B00", ruleInfo->carrierCode());
  construct.addAttribute("D06", ruleInfo->effDate().dateToString(YYYYMMDD, "-").c_str());
  construct.addAttribute("D05", ruleInfo->discDate().dateToString(YYYYMMDD, "-").c_str());
  // International
  construct.addAttribute("P95", !isRecordScopeDomestic() ? "true" : "false");
  construct.addAttribute("N0W", "0");
  construct.addAttribute("B50", ruleInfo->fareClass().c_str());
  construct.addAttributeChar("P04", ruleInfo->owrt());
  // construct.addAttributeChar("Q3X", ruleInfo->jointCarrierTblItemNo());
  char tmpBuf[20];
  sprintf(tmpBuf, "%d", ruleInfo->jointCarrierTblItemNo());
  construct.addAttribute("Q3X", tmpBuf);
}
void
GenerateRuleRequestFormatter::addRCSHeader(const FareByRuleCtrlInfo* ruleInfo,
                                           XMLConstruct& construct)
{
  construct.addAttributeChar("S33", ruleInfo->loc1().locType());
  construct.addAttribute("S34", ruleInfo->loc1().loc());
  construct.addAttributeChar("S35", ruleInfo->loc2().locType());
  construct.addAttribute("S36", ruleInfo->loc2().loc());
  construct.addAttribute("B00", ruleInfo->carrierCode());
  construct.addAttribute("D06", ruleInfo->effDate().dateToString(YYYYMMDD, "-").c_str());
  construct.addAttribute("D05", ruleInfo->discDate().dateToString(YYYYMMDD, "-").c_str());
  // International
  construct.addAttribute("P95", !isRecordScopeDomestic() ? "true" : "false");
  construct.addAttribute("N0W", "0");
  // TODO needed for FBR ???
  //  construct.addAttribute("B50", ruleInfo->fareClass());
  //  construct.addAttributeChar("P04", ruleInfo->owrt());
  // construct.addAttributeChar("Q3X", ruleInfo->jointCarrierTblItemNo());
  char tmpBuf[20];
  sprintf(tmpBuf, "%d", ruleInfo->jointCarrierTblItemNo());
  construct.addAttribute("Q3X", tmpBuf);
}

void
GenerateRuleRequestFormatter::addQ4RAttribute(const RuleType& ruleType, XMLConstruct& construct)
{
  char tmpBuf[2];

  // Rule type
  sprintf(tmpBuf, "%d", ruleType);
  construct.addAttribute("Q4R", tmpBuf);
}

void
GenerateRuleRequestFormatter::addCSBType(const CombinabilityRuleInfo& ruleInfo,
                                         XMLConstruct& construct)
{
  char tmpBuf[20];

  construct.openElement("CSB");

  // Same points table item number key
  sprintf(tmpBuf, "%d", ruleInfo.samepointstblItemNo());
  construct.addAttribute("Q3Z", tmpBuf);

  // Single open jaw indicator
  sprintf(tmpBuf, "%c", ruleInfo.sojInd());
  construct.addAttribute("N1A", tmpBuf);

  // Open jaw occurrence indicator
  sprintf(tmpBuf, "%c", ruleInfo.sojorigIndestInd());
  construct.addAttribute("N1B", tmpBuf);

  // Double open jaw indicator
  sprintf(tmpBuf, "%c", ruleInfo.dojInd());
  construct.addAttribute("N1C", tmpBuf);

  // Carrier restrictions indicator
  sprintf(tmpBuf, "%c", ruleInfo.dojCarrierRestInd());
  construct.addAttribute("P96", tmpBuf);

  // Tariff/Rule restrictions indicator
  sprintf(tmpBuf, "%c", ruleInfo.dojTariffRuleRestInd());
  construct.addAttribute("P97", tmpBuf);

  // Fare class/type restrictions indicator
  sprintf(tmpBuf, "%c", ruleInfo.dojFareClassTypeRestInd());
  construct.addAttribute("P98", tmpBuf);

  // Circle trip indicator indicator
  sprintf(tmpBuf, "%c", ruleInfo.ct2Ind());
  construct.addAttribute("P99", tmpBuf);

  // CT carrier restrictions indicator
  sprintf(tmpBuf, "%c", ruleInfo.ct2CarrierRestInd());
  construct.addAttribute("PA1", tmpBuf);

  // CT tariff/rule restrictions indicator
  sprintf(tmpBuf, "%c", ruleInfo.ct2TariffRuleRestInd());
  construct.addAttribute("PA2", tmpBuf);

  // CT Fare class/type restrictions indicator
  sprintf(tmpBuf, "%c", ruleInfo.ct2FareClassTypeRestInd());
  construct.addAttribute("PA3", tmpBuf);

  // CT more than 2 components indicator
  sprintf(tmpBuf, "%c", ruleInfo.ct2plusInd());
  construct.addAttribute("PA4", tmpBuf);

  // CT2 carrier restrictions
  sprintf(tmpBuf, "%c", ruleInfo.ct2plusCarrierRestInd());
  construct.addAttribute("PA5", tmpBuf);

  // CT2 tariff/rule restrictions
  sprintf(tmpBuf, "%c", ruleInfo.ct2plusTariffRuleRestInd());
  construct.addAttribute("PA6", tmpBuf);

  // End-on-end indicator
  sprintf(tmpBuf, "%c", ruleInfo.eoeInd());
  construct.addAttribute("N1D", tmpBuf);

  // EOE carrier restrictions
  sprintf(tmpBuf, "%c", ruleInfo.eoeCarrierRestInd());
  construct.addAttribute("PA7", tmpBuf);

  // EOE tariff/rule restrictions
  sprintf(tmpBuf, "%c", ruleInfo.eoeTariffRuleRestInd());
  construct.addAttribute("PA8", tmpBuf);

  // EOE fare class/type restrictions
  sprintf(tmpBuf, "%c", ruleInfo.eoeFareClassTypeRestInd());
  construct.addAttribute("PA9", tmpBuf);

  // Add-on indicator
  sprintf(tmpBuf, "%c", ruleInfo.arbInd());
  construct.addAttribute("N1E", tmpBuf);

  // Add-on carrier restrictions
  sprintf(tmpBuf, "%c", ruleInfo.arbCarrierRestInd());
  construct.addAttribute("PAA", tmpBuf);

  // Add-on tariff/rule restrictions
  sprintf(tmpBuf, "%c", ruleInfo.arbTariffRuleRestInd());
  construct.addAttribute("PAB", tmpBuf);

  // Add-on fare class/type restrictions
  sprintf(tmpBuf, "%c", ruleInfo.arbFareClassTypeRestInd());
  construct.addAttribute("PAC", tmpBuf);

  construct.closeElement();
}

void
GenerateRuleRequestFormatter::addC25Type(std::string& fareBasis,
                                         CurrencyCode curr,
                                         Indicator fareInd,
                                         XMLConstruct& construct)
{
  construct.openElement("C25");
  // @TODO Fill in values
  // Origin city
  construct.addAttribute("A11", " ");

  // Destination city
  construct.addAttribute("A12", " ");

  // Fare basis code
  construct.addAttribute("B40", fareBasis);

  // Fare amount
  construct.addAttribute("C50", " ");

  // Currency number of decimals
  construct.addAttribute("Q05", " ");

  // Fare indicator
  construct.addAttributeChar("N0K", fareInd);

  // Fare currency code
  construct.addAttribute("C40", curr);

  construct.closeElement();
}

//----------------------------------------------------------------------------
// GenerateRuleRequestFormatter::addC25Type
//   info on base fare used for discount (cat 19-22, 25 calc)
//----------------------------------------------------------------------------
void
GenerateRuleRequestFormatter::addDISType(std::string& fareBasis,
                                         Indicator owrt, // (raw ATPCO in TseConst)
                                         const PaxTypeCode& paxTypeCode,
                                         XMLConstruct& construct)
{
  Indicator RTGowrt;
  switch (owrt)
  {
  case ONE_WAY_MAY_BE_DOUBLED:
    RTGowrt = 'X';
    break;
  case ROUND_TRIP_MAYNOT_BE_HALVED:
    RTGowrt = 'R';
    break;
  case ONE_WAY_MAYNOT_BE_DOUBLED:
    RTGowrt = 'O';
    break;
  default:
    RTGowrt = 'H';
    break;
  }
  construct.openElement("DIS");
  construct.addAttributeChar("P04", RTGowrt);
  construct.addAttribute("B40", fareBasis);
  construct.addAttribute("B70", paxTypeCode);
  construct.closeElement();
}

int
GenerateRuleRequestFormatter::asFixedPoint(double x, int numDecimals)
{
  int fac = 1;
  switch (numDecimals)
  {
  case 2:
    fac = 100;
    break;
  case 0:
    break;
  case 1:
    fac = 10;
    break;
  case 3:
    fac = 1000;
    break;
  case 4:
    fac = 10000;
    break;
  case 5:
    fac = 100000;
    break;
  default:
    for (int i = 5, fac = 100000; i < numDecimals; i++)
      fac *= 10;
    break;
  }
  // calc in float, round to int
  x = (x * fac) + .5;
  return (int)x;
}
//----------------------------------------------------------------------------
// GenerateRuleRequestFormatter::addC35Type
//     For fields that describe how the fare was calulated (NOK - Q05), use only
//     data from the last redist (i.e. hide wholesaler calculations)
//----------------------------------------------------------------------------
void
GenerateRuleRequestFormatter::addC35Type(PseudoCityCode& creatorPCC,
                                         Indicator calcInd,
                                         Money& ruleMoney,
                                         int noDecAmt,
                                         Percent percent,
                                         int noDecPercent,
                                         bool needCalcData,
                                         Indicator displayCatType,
                                         Indicator canSell,
                                         Indicator canTkt,
                                         Indicator canUpd,
                                         Indicator canRedist,
                                         Indicator locale,
                                         XMLConstruct& construct)
{
  construct.openElement("C35");

  // fields from markup or Table979
  construct.addAttribute("A20", creatorPCC);
  if (needCalcData)
  {
    construct.addAttributeChar("N0K", calcInd);
    construct.addAttributeInteger("C62", asFixedPoint(ruleMoney.value(), noDecAmt));
    construct.addAttribute("C40", ruleMoney.code());
    construct.addAttributeInteger("Q05", noDecAmt);
    construct.addAttributeInteger("C61", asFixedPoint(percent, noDecPercent));
    construct.addAttributeInteger("Q06", noDecPercent);
  }

  // from PaxTypeFare
  construct.addAttributeChar("N0P", displayCatType);

  // security flags
  construct.addAttributeChar("N1M", locale);
  construct.addAttributeChar("P70", canSell);
  construct.addAttributeChar("P29", canTkt);
  construct.addAttributeChar("PBD", canUpd);
  construct.addAttributeChar("PBE", canRedist);

  construct.closeElement();
}

void
GenerateRuleRequestFormatter::addTVLType(const FareDisplayTrx& trx,
                                         XMLConstruct& construct,
                                         bool skipDiag854)
{
  // Add travel element
  construct.openElement("TVL");

  // Add ticketing date
  std::string ticketDT = trx.getRequest()->ticketingDT().dateToString(YYYYMMDD, "-");
  ticketDT += " ";
  ticketDT += trx.getRequest()->ticketingDT().timeToString(HHMMSS, ":");
  construct.addAttribute("D06", ticketDT.c_str());

  // Add diagnostic number if applicable
  if (!skipDiag854 && FareDisplayUtil::isHostDBInfoDiagRequest(trx))
  {
    construct.addAttributeInteger("Q40", 854);
  }

  construct.closeElement();
}

void
GenerateRuleRequestFormatter::addRFIType(const PaxTypeFare& ptf, XMLConstruct& construct)
{
  char tmpBuf[2];
  construct.openElement("RFI");
  sprintf(tmpBuf, "%d", ptf.tcrTariffCat());
  construct.addAttribute("Q4R", tmpBuf);
  construct.closeElement();
}
}
