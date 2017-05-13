//----------------------------------------------------------------------------
//  File:         Diagnostic.C
//  Description:  Root Diagnostic object, Container of all the diagnostic messages
//
//  Authors:      Mohammad Hossan
//  Created:      Dec 2003
//
//  Updates:
//          date - initials - description.
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

#include "Diagnostic/Diagnostic.h"

#include "Common/FareTypeDesignator.h"
#include "Common/FareTypeMatrixUtil.h"
#include "Common/TseConsts.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DBAccess/Loc.h"
#include "Rules/RuleUtil.h"
#include "Util/BranchPrediction.h"

#include <boost/algorithm/string.hpp>

#include <algorithm>
#include <iostream>
#include <memory>
#include <sstream>


namespace tse
{
std::string Diagnostic::_emptyString;

const std::string Diagnostic::NO_LIMIT = "NO";
const std::string Diagnostic::UNLIMITED_RESPONSE = "LR";
const std::string Diagnostic::RESPONSE_SIZE = "RS";

const std::string Diagnostic::SPECIFIC_CATEGORY = "RL";

const std::string Diagnostic::DIAG_CARRIER = "CX";
const std::string Diagnostic::DIAG_VENDOR = "VN";
const std::string Diagnostic::MISCELLANEOUS = "MS";
const std::string Diagnostic::ALL_VALID = "AV";
const std::string Diagnostic::DISPLAY_DETAIL = "DD";
const std::string Diagnostic::IDENTIFICATION = "ID";

const std::string Diagnostic::FVO_RULES_BEFORE_BKGCODE = "RB";
const std::string Diagnostic::FVO_BKGCODE_BEFORE_RULES = "BR";
const std::string Diagnostic::WPNC_SOLO_TEST = "SO";
const std::string Diagnostic::ACTIVATE_JOURNEY = "JR";

const std::string Diagnostic::FARE_MARKET = "FM";
const std::string Diagnostic::GLOBAL_DIRECTION = "GD";

const std::string Diagnostic::FARE_ASSIGNMENT = "FA";
const std::string Diagnostic::FARE_BASIS_CODE = "FB";
const std::string Diagnostic::FARE_CLASS_CODE = "FC";

const std::string Diagnostic::FARE_TARIFF = "FT";

// FARE_TARIFF i FARE_TYPE have same values, that's why TARIFF_NUMBER
const std::string Diagnostic::TARIFF_NUMBER = "TN";

const std::string Diagnostic::FARE_TYPE = "FT";
const std::string Diagnostic::FARE_TYPE_DESIGNATOR = "FD";
const std::string Diagnostic::NO_DUPLICATE = "ND";

const std::string Diagnostic::GATEWAY_PAIR = "GW";
const std::string Diagnostic::ADDON_FARE_CLASS_CODE = "AC";
const std::string Diagnostic::ADDON_FARE_TARIFF = "AT";
const std::string Diagnostic::DATE_INTERVALS = "DI";

const std::string Diagnostic::RULE_NUMBER = "RU";

const std::string Diagnostic::FARE_USAGE_NUMBER = "FN";

const std::string Diagnostic::FARE_PATH = "FP";
const std::string Diagnostic::PRICING_UNIT = "PU";

const std::string Diagnostic::FOOTNOTE = "NT";
const std::string Diagnostic::PAX_TYPE = "PT";

const std::string Diagnostic::TO_ROUTING = "TO";
const std::string Diagnostic::ITIN_TYPE = "IT";
const std::string Diagnostic::SUB_CODE = "SC";
const std::string Diagnostic::SERVICE_GROUP = "SG";

// For Baggage
const std::string Diagnostic::SEQ_NUMBER = "SQ";
const std::string Diagnostic::BAG_PR_FLOW = "PF";
const std::string Diagnostic::BAG_FB = "FB";

const std::string Diagnostic::SRV_GROUP = "SG";
const std::string Diagnostic::SRV_CODE = "SC";
const std::string Diagnostic::RULE_PHASE = "RP";
const std::string Diagnostic::BRAND_ID = "BC";
const std::string Diagnostic::PROGRAM_NAME = "PC";
const std::string Diagnostic::TABLE_DETAILS = "TD";

const std::string Diagnostic::SOP_IDS = "SI";
const std::string Diagnostic::FOS_GENERATOR = "FG";

const std::string Diagnostic::FF_RULE_ID = "ID";

const std::string Diagnostic::TOPLINE_METRICS = "TMETRICS";
const std::string Diagnostic::FULL_METRICS = "FMETRICS";

const std::string Diagnostic::RBD_ALL = "AL";
const std::string Diagnostic::BOOKING_CODE = "BK";
const std::string Diagnostic::CITY_PAIR = "CP";
const std::string Diagnostic::DIAG_CABIN = "CB";
const std::string Diagnostic::TRAVEL_DATE = "TD";
const std::string Diagnostic::TICKET_DATE = "TK";

const std::string Diagnostic::FARE_RETAILER_TYPE = "RT";

const std::string Diagnostic::COMMISSION_CONTRACT = "CC";
const std::string Diagnostic::MAX_PEN = "MAXPEN";
const std::string Diagnostic::PREVALIDATION = "PRE";

const std::string Diagnostic::BRANDING_SERVICE_FORMAT = "BS";
const std::string Diagnostic::CDATA_XML = "XML";

const std::string Diagnostic::CDATA_START_TAG = "<![CDATA[";
const std::string Diagnostic::CDATA_END_TAG = "]]>";

const std::string Diagnostic::MATCHING = "MATCHING";

void
Diagnostic::activate()
{
  _active = true;
}

void
Diagnostic::insertDiagMsg(const std::string& diagMsgs)
{
  TseCallableTrxTask::SequenceID taskSeqID(getCurrentTaskSequenceID());

  const boost::lock_guard<boost::mutex> g(_messageMutex);
  _diagMessages.push_back(ThreadDiagMsgs(taskSeqID, _diagMessages.size(), diagMsgs));
}

std::string
Diagnostic::toString()
{
  std::string str;
  if (isActive())
  {
    const boost::lock_guard<boost::mutex> g(_messageMutex);

    sortDiagMsgs();
    for (const auto& elem : _diagMessages)
    {
      str += std::tr1::get<2>(elem);
    }
  }
  return str;
}

bool
Diagnostic::shouldDisplay(const LocCode& origLoc,
                          const LocCode& boardMultiCity,
                          const LocCode& destLoc,
                          const LocCode& offMultiCity) const
{
  const std::string& diagFareMarket = diagParamMapItem(FARE_MARKET);

  if (!diagFareMarket.empty())
  {
    LocCode boardCity = diagFareMarket.substr(0, 3);
    LocCode offCity = diagFareMarket.substr(3, 3);

    if (((origLoc != boardCity) && (boardMultiCity != boardCity)) ||
        ((destLoc != offCity) && (offMultiCity != offCity)))
      return false;
  }
  return true;
}

bool
Diagnostic::shouldDisplay(const FareMarket& fareMarket) const
{
  return shouldDisplay(fareMarket.origin()->loc(),
                       fareMarket.boardMultiCity(),
                       fareMarket.destination()->loc(),
                       fareMarket.offMultiCity());
}

bool
Diagnostic::shouldDisplay(const PaxTypeFare& paxTypeFare) const
{
  const std::string& diagFareA = diagParamMapItem(FARE_ASSIGNMENT);
  if (UNLIKELY(!diagFareA.empty()))
  {
    if (diagFareA == "N" && !paxTypeFare.isNormal())
      return false;

    if (diagFareA == "S" && !paxTypeFare.isSpecial())
      return false;
  }

  const std::string& diagFareClass = diagParamMapItem(FARE_CLASS_CODE);
  if (UNLIKELY(!diagFareClass.empty()))
  {
    if (!matchFareClass(diagFareClass, paxTypeFare))
    {
      return false;
    }
  }

  const std::string& globalDir = diagParamMapItem(GLOBAL_DIRECTION);
  if (UNLIKELY(!globalDir.empty()))
  {
    GlobalDirection diagGlobalDir;
    if (strToGlobalDirection(diagGlobalDir, globalDir))
    {
      if (diagGlobalDir != paxTypeFare.globalDirection())
        return false;
    }
  }

  const std::string& fareTypeDesignator = diagParamMapItem(FARE_TYPE_DESIGNATOR);
  if (UNLIKELY(!fareTypeDesignator.empty()))
  {
    std::string ftd = FareTypeMatrixUtil::convert(paxTypeFare.fareTypeDesignator());

    if (ftd != fareTypeDesignator)
      return false;
  }

  const std::string& failedCat = diagParamMapItem(SPECIFIC_CATEGORY);
  if (UNLIKELY(failedCat == "F05"))
  {
    if (paxTypeFare.areAllCategoryValid())
      return false;
    if (!paxTypeFare.isCategoryProcessed(5))
      return false;
    if (!paxTypeFare.isCategoryValid(5))
      return true;
    else
      return false;
  }
  return true;
}

bool
Diagnostic::shouldDisplay(const PaxTypeFare& paxTypeFare, const unsigned short& categoryNumber)
    const
{
  const std::string& diagFareMarket = diagParamMapItem(FARE_MARKET);

  if (UNLIKELY(!diagFareMarket.empty()))
  {
    LocCode boardCity = diagFareMarket.substr(0, 3);
    LocCode offCity = diagFareMarket.substr(3, 3);

    const FareMarket& fareMarket = *paxTypeFare.fareMarket();

    if (((fareMarket.origin()->loc() != boardCity) && (fareMarket.boardMultiCity() != boardCity)) ||
        ((fareMarket.destination()->loc() != offCity) && (fareMarket.offMultiCity() != offCity)))
    {
      return false;
    }
  }

  if (UNLIKELY(!shouldDisplay(paxTypeFare)))
    return false;

  const std::string& diagCat = diagParamMapItem(SPECIFIC_CATEGORY);
  if (UNLIKELY(!diagCat.empty()))
  {
    uint16_t filterCatNum = atoi(diagCat.c_str());
    if ((filterCatNum != 0) && (categoryNumber != filterCatNum))
      return false;
  }

  // free to add more filtering if you want
  return true;
}

bool
Diagnostic::shouldDisplay(const PaxTypeFare& paxTypeFare,
                          const unsigned short& categoryNumber,
                          const PaxTypeCode& paxType) const
{
  const std::string& diagPaxType = diagParamMapItem(PAX_TYPE);

  if (UNLIKELY(!diagPaxType.empty()))
  {
    if (diagPaxType != paxType)
      return false;
  }
  return shouldDisplay(paxTypeFare, categoryNumber);
}

bool
Diagnostic::matchFareClass(const std::string& diagFareClass, const PaxTypeFare& paxTypeFare) const
{
  if (RuleUtil::matchFareClass(diagFareClass.c_str(), paxTypeFare.fareClass().c_str()))
  {
    return true;
  }

  if (!paxTypeFare.isFareByRule())
    return false;

  // compare base fare fare class for FareByRule
  const FBRPaxTypeFareRuleData* fbr = paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);
  if (!fbr || !fbr->baseFare())
    return false;

  return RuleUtil::matchFareClass(diagFareClass.c_str(), fbr->baseFare()->fareClass().c_str());
}

std::string
Diagnostic::masterPricingDiagnostic()
{
  std::ostringstream oss;

  oss << "***************************************************************\n";
  oss << "PRICING DIAGNOSTICS MASTER LIST \n";
  oss << "***************************************************************\n";
  oss << "TO ACCESS FARE DISPLAY MASTER LIST, ADD $999 TO FQ ENTRY \n";
  oss << "*************************************************************** \n";
  oss << "ADD CROSS OF LORRAINE Q/* BEFORE EACH PRICING DIAGNOSTIC.\n";
  oss << "ENTER A SLASH AFTER THE NUMBER TO ADD APPLICABLE QUALIFIERS \n";
  // oss << "  /DDALLFARES         ALL VALID/INVALID FARES IN MARKET \n";
  // oss << "  /FCVLXAP6M          SPECIFIC FARE CLASS \n";
  oss << "  /FBBAPOW            SPECIFIC FARE BASIS CODE \n";
  oss << "  /RU                 RULE NUMBER \n";
  oss << "  /DDINFO             DISPLAY RECORD 1 INFORMATION \n";
  oss << "              \n";
  oss << "               \n";
  oss << Diagnostic185 << "             PRICE BY CABIN DIAGNOSTIC\n";
  oss << Diagnostic187 << "             RBD BY CABIN VALIDATION DIAGNOSTIC\n";
  oss << Diagnostic188 << "             TICKET POINT MILEAGE\n";
  oss << Diagnostic192 << "             ITINERARY CHARACTERISTIC \n";
  oss << Diagnostic193 << "             CUSTOMER ACTIVATION CONFIGURATION INFO \n";
  oss << Diagnostic195 << "             RESERVATION AND AVIALABILITY DATA\n";
  oss << Diagnostic199 << "             FARE MARKET, AVAIL., RULE2/SOLO AND JOURNEY INFO\n";
  oss << AllFareDiagnostic << "             FARE COLLECTOR VALID FARES \n";
  oss << Diagnostic202 << "             R2 MATCHING DETAILS \n";
  oss << Diagnostic208 << "             CAT 25 RECORD 8 PROCESSING \n";
  oss << Diagnostic212 << "             CURRENCY DIAGNOSTIC \n";
  oss << Diagnostic225 << "             CAT 25 RECORD 2 PROCESSING \n";
  oss << Diagnostic231 << "             VOLUNTARY EXCHANGE - FARE MATCHING\n";
  oss << Diagnostic233 << "             VOLUNTARY REFUND - FARE MATCHING\n";
  oss << Diagnostic251 << "             ADD-ON ZONES AND TARIFFS \n";
  oss << Diagnostic252 << "             ADD-ON CANDIDATES \n";
  oss << Diagnostic253 << "             ADD-ON CONSTRUCTION TARIFF X REF. AND FC RECORD\n";
  oss << Diagnostic254 << "             CONSTRUCTED FARES \n";
  oss << Diagnostic255 << "             SPECIFIED FARE AND ADD-ON FARE CANDIDATES \n";
  oss << Diagnostic257 << "             CONSTRUCTED FARES \n";
  oss << Diagnostic301 << "             ELIGIBILITY RULE VALIDATION \n";
  oss << Diagnostic302 << "             DAY OF WEEK RULE VALIDATION \n";
  oss << Diagnostic303 << "             SEASON RULE VALIDATION \n";
  oss << Diagnostic304 << "             FLIGHT APPLICATION VALIDATION\n";
  oss << Diagnostic305 << "             ADVANCE RES/TKT RULE VALIDATION \n";
  oss << Diagnostic306 << "             MINIMUM STAY RULE VALIDATION \n";
  oss << Diagnostic307 << "             MAXIMUM STAY RULE VALIDATION \n";
  oss << Diagnostic308 << "             STOPOVERS RULE VALIDATION \n";
  oss << Diagnostic309 << "             TRANSFERS RULE VALIDATION \n";
  oss << Diagnostic311 << "             BLACKOUTS RULE VALIDATION \n";
  oss << Diagnostic312 << "             SURCHARGES RULE VALIDATION \n";
  oss << Diagnostic313 << "             ACCOMPANYING TRAVEL RULE VALIDATION\n";
  oss << Diagnostic314 << "             TRAVEL RESTRICTIONS RULE VALIDATION \n";
  oss << Diagnostic315 << "             SALES RESTRICTIONS RULE VALIDATION \n";
  oss << Diagnostic316 << "             PENALTIES RULE VALIDATION \n";
  oss << Diagnostic319 << "             CHILD/INFANT DISCOUNTS RULE VALIDATION \n";
  oss << Diagnostic323 << "             MISCELLANEOUS PROVISIONS RULE VALIDATION \n";
  oss << Diagnostic325 << "             FARE BY RULE CAT 25 RULE VALIDATION \n";
  oss << Diagnostic331 << "             VOLUNTARY EXCHANGE \n";
  oss << Diagnostic335 << "             NEGOTIATED FARES RULE VALIDATION \n";
  oss << Diagnostic400 << "             BOOKING CODE VALIDATION \n";
  oss << Diagnostic405 << "             BOOKING CODE EXCEPTION \n";
  oss << Diagnostic411 << "             DETAILED BOOKING CODE VALIDATION \n";
  oss << Diagnostic413 << "             DIFFERENTIAL CALCULATION DIAGNOSTIC \n";
  oss << Diagnostic420 << "             MIXED CLASS VALIDATION \n";
  oss << Diagnostic450 << "             ROUTING VALIDATION RESULTS\n";
  oss << Diagnostic452 << "             TPM EXCLUSION ROUTING VALIDATION RESULTS\n";
  oss << Diagnostic455 << "             ROUTING MAP DISPLAY \n";
  oss << Diagnostic499 << "             ROUTING, RULES, BOOKING CODE VALIDATION ON FC \n";
  oss << Diagnostic500 << "             FOOTNOTE, FARE RULE AND GENERAL RULE PROCESSING \n";
  oss << Diagnostic631 << "             COMBINABILITY OPEN JAW ANALYSIS\n";
  oss << Diagnostic632 << "             COMBINABILITY ROUND TRIP ANALYSIS\n";
  oss << Diagnostic633 << "             COMBINABILITY CIRCLE TRIP ANALYSIS\n";
  oss << Diagnostic634 << "             COMBINABILITY END-ON-END ANALYSIS\n";
  oss << Diagnostic660 << "             PRICING UNIT ANALYSIS \n";
  oss << Diagnostic682 << "             PRICING UNIT LIMITATION ANALYSIS \n";
  oss << Diagnostic700 << "             MINIMUM FARE CHECK PROCESS FLOW \n";
  oss << FailTaxCodeDiagnostic << "             TAXES \n";
  oss << Diagnostic852 << "             AUTOMATED FREE BAGGAGE ALLOWANCE \n";
  oss << Diagnostic854 << "             DATABASE SERVER AND PORT NUMBER \n";
  oss << Diagnostic858 << "             TICKET ENDORSEMENTS DIAGNOSTICS  \n";
  oss << Diagnostic865 << "             COMMISSIONS CAT35 DIAGNOSTICS  \n";
  oss << Diagnostic866 << "             TICKET COMMISSION CAP DIAGNOSTICS  \n";
  oss << Diagnostic867 << "             COMMISSION MANAGEMENT DIAGNOSTICS  \n";
  oss << Diagnostic870 << "             SERVICE FEE DIAGNOSTIC          \n";
  oss << Diagnostic875 << "             OC SERVICE FEE S5 PROCESSING \n";
  oss << Diagnostic876 << "             OC SERVICE FEE S6 PROCESSING \n";
  oss << Diagnostic877 << "             OC SERVICE FEE S7 PROCESSING \n";
  oss << Diagnostic880 << "             SLICE @ DICE OC SERVICE FEE DIAGNOSTIC \n";
  oss << Diagnostic881 << "             ANCILLARY FEE PSEUDO FARE CONSTRUCTION \n";
  oss << "*************************************************************** \n";

  return oss.str();
}

std::string
Diagnostic::masterAncillaryPricingDiagnostic()
{
  std::ostringstream oss;

  oss << "***************************************************************\n";
  oss << "ANCILLARY PRICING DIAGNOSTICS MASTER LIST \n";
  oss << "***************************************************************\n";
  oss << "ADD CROSS OF LORRAINE Q/* BEFORE EACH PRICING DIAGNOSTIC.\n";
  oss << "ENTER A SLASH AFTER THE NUMBER TO ADD APPLICABLE QUALIFIERS \n";
  oss << "  /SG                 GROUP CODE \n";
  oss << "  /SC                 SUB CODE \n";
  oss << "  /SQ                 SEQUENCE NUMBER \n";
  oss << "  /FM                 CITY PAIR \n";
  oss << "  /DDINFO             DISPLAY S5 OR S7 DETAILED INFORMATION \n";
  oss << "  /DDPASSED           DISPLAY S5 OR S7 PASSED ONLY \n";
  oss << "              \n";
  oss << "               \n";
  oss << Diagnostic188 << "             TICKET POINT MILEAGE\n";
  oss << Diagnostic195 << "             RESERVATION AND AVIALABILITY DATA\n";
  oss << Diagnostic199 << "             FARE MARKET, AVAIL., RULE2/SOLO AND JOURNEY INFO\n";
  oss << Diagnostic854 << "             DATABASE SERVER AND PORT NUMBER \n";
  oss << Diagnostic875 << "             OC SERVICE FEE S5 PROCESSING \n";
  oss << Diagnostic876 << "             OC SERVICE FEE S6 PROCESSING \n";
  oss << Diagnostic877 << "             OC SERVICE FEE S7 PROCESSING \n";
  oss << Diagnostic880 << "             SLICE @ DICE OC SERVICE FEE DIAGNOSTIC \n";
  oss << Diagnostic881 << "             ANCILLARY FEE PSEUDO FARE CONSTRUCTION \n";
  oss << "*************************************************************** \n";

  return oss.str();
}

bool
Diagnostic::filterRulePhase(int phase) const
{
  const std::string& rulePhase = diagParamMapItem(RULE_PHASE);

  if (rulePhase.empty())
    return false;

  switch (phase)
  {
  case 0:
    return false;
  case 1:
    return rulePhase != "FV";
  case 2:
    return rulePhase != "FC";
  case 3:
    return rulePhase != "PP";
  case 4:
    return rulePhase != "PU";
  case 5:
    return rulePhase != "FP";
  case 6:
    return rulePhase != "CO";
  case 7:
    return rulePhase != "FD";
  case 8:
    return rulePhase != "RD";
  case 9:
    return rulePhase != "DV";
  default:
    ;
  }
  return true;
}

struct Diagnostic::IsMsgFromTrxTask : std::unary_function<Diagnostic::ThreadDiagMsgs, bool>
{
  bool operator()(const ThreadDiagMsgs& diagMsg) const
  {
    return !std::tr1::get<0>(diagMsg).empty();
  }
};

void
Diagnostic::sortDiagMsgs() // sort messages from TrxTask using messages not from TrxTask as barriers
{
  if (!diagParamMapItemPresent("NORM_DIAG"))
    return;

  for (RootDiagMsgs::iterator sortIter(_diagMessages.begin()); sortIter != _diagMessages.end();)
  {
    sortIter = std::find_if(sortIter, _diagMessages.end(), IsMsgFromTrxTask());
    if (sortIter == _diagMessages.end())
      break;

    RootDiagMsgs::iterator sortEndIter(
        std::find_if(sortIter, _diagMessages.end(), std::not1(IsMsgFromTrxTask())));
    std::sort(sortIter, sortEndIter);
    sortIter = sortEndIter;
  }
}

bool
Diagnostic::isDiag455ParamSet() const
{
  DiagParamMapVecIC diagParamIt;

  return (diagnosticType() == Diagnostic455 &&
          (diagParamIt = diagParamMap().find(DISPLAY_DETAIL)) != diagParamMap().end() &&
          diagParamIt->second == "MAP");
}

void
Diagnostic::escapeNullCharacters(std::string& stringToEscape)
{
  boost::replace_all(stringToEscape, std::string(1, '\0'), "\\0");
}

void
Diagnostic::appendAdditionalDataSection(std::string xmlToAdd)
{
  escapeNullCharacters(xmlToAdd);
  _additionalData += Diagnostic::CDATA_START_TAG + \
                     xmlToAdd + \
                     Diagnostic::CDATA_END_TAG + "\n";
}

}

