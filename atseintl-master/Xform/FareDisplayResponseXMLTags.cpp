//-------------------------------------------------------------------
//
//  File:        FareDisplayResponseXMLTags.cpp
//  Authors:     Hitha Alex
//  Created:     March 8, 2006
//  Description: Base class for a data filter
//
//
//  Copyright Sabre 2006
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//---------------------------------------------------------------------------

#include "Xform/FareDisplayResponseXMLTags.h"

#include "Common/BSRCollectionResults.h"
#include "Common/CabinType.h"
#include "Common/BSRCurrencyConverter.h"
#include "Common/CurrencyCollectionResults.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyConverter.h"
#include "Common/CurrencyUtil.h"
#include "Common/FareDisplayResponseUtil.h"
#include "Common/FareDisplayTax.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Logger.h"
#include "Common/NonFatalErrorResponseException.h"
#include "Common/NUCCollectionResults.h"
#include "Common/NUCCurrencyConverter.h"
#include "Common/TrxUtil.h"
#include "DataModel/Agent.h"
#include "DataModel/FareDisplayInfo.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PrivateIndicator.h"
#include "DataModel/SeasonsInfo.h"
#include "DataModel/TicketInfo.h"
#include "DataModel/TravelInfo.h"
#include "DBAccess/CategoryRuleInfo.h"
#include "DBAccess/FareByRuleApp.h"
#include "DBAccess/FareDisplayInclCd.h"
#include "DBAccess/FareDispRec1PsgType.h"
#include "DBAccess/NegFareSecurityInfo.h"
#include "DBAccess/TaxCodeReg.h"
#include "FareDisplay/FDHeaderMsgText.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "ItinAnalyzer/InclusionCodeRetriever.h"
#include "Rules/RuleConst.h"
#include "Taxes/LegacyTaxes/TaxItem.h"
#include "Taxes/LegacyTaxes/TaxRecord.h"
#include "Xform/FareDisplaySDSTaxInfo.h"

namespace tse
{
using namespace std;

static Logger
logger("atseintl.Xform.FareDisplayResponseXMLTags");

// fare indicators
const std::string ADDED_INDICATOR = "A";
const std::string DELETED_INDICATOR = "X";
const std::string INCREASED_INDICATOR = "I";
const std::string REDUCED_INDICATOR = "R";
const std::string CHANGED_INDICATOR = "C";

// this value defines the mode date not in last 24 hours Indicator
const std::string BLANK_INDICATOR = " ";

// Min_max Stay Indicators
const std::string MAX_STAY_UNITS_MONTH = "M";
const std::string MIN_MAX_STAY_NP = "NP/NP";

// Seasons Indicators
const std::string SEASONS_NO_DATES = "   ---";
const std::string SEASONS_SPACE = " ";
const std::string SEASONS_DASH = "-";
const std::string SEASONS_NP = "   NP";
const char SEASONS_BLANK = ' ';

// Header message consts
const std::string
CENTER("CT");
const std::string
RIGHT("RJ");
const std::string
BLANK_SPACE(" ");

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::FareDisplayResponseXMLTags()
//---------------------------------------------------------------------------
FareDisplayResponseXMLTags::FareDisplayResponseXMLTags() {}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::~FareDisplayResponseXMLTags()
//---------------------------------------------------------------------------
FareDisplayResponseXMLTags::~FareDisplayResponseXMLTags() {}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::vendorCode()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::vendorCode(PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{
  std::string vendorCodeTag = paxTypeFare.vendor();

  if (vendorCodeTag == Vendor::ATPCO)
    return "A";
  else if (vendorCodeTag == Vendor::SITA)
    return "S";
  else if (vendorCodeTag == Vendor::SABD)
    return "D";
  else if (vendorCodeTag == Vendor::HIKE)
    return "H";
  else if (trx.dataHandle().getVendorType(vendorCodeTag) == TRAVEL_AGENCY)
  {
    if (paxTypeFare.vendorFWS())
      return "W";
    return "F";
  }
  else if (trx.dataHandle().getVendorType(vendorCodeTag) == VENDOR_CARRIER)
    return "D";
  else if (vendorCodeTag == Vendor::FMS || vendorCodeTag == Vendor::POFO)
    return "P";
  else
    return "F";
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::advancePurchase()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::advancePurchase(FareDisplayInfo*& fareDisplayInfo)
{

  std::string advancePurchaseTag = "";
  fareDisplayInfo->getAPString(advancePurchaseTag);
  // remove blank spaces
  string::iterator blankIter =
      std::remove(advancePurchaseTag.begin(), advancePurchaseTag.end(), ' ');
  advancePurchaseTag.erase(blankIter, advancePurchaseTag.end());
  if (advancePurchaseTag == "-")
    return "";
  else
    return advancePurchaseTag;
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::minStay()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::minStay(FareDisplayInfo* fareDisplayInfo)
{
  std::ostringstream minStayTag;

  // Check for NP condition (Not auto-Priceable)
  if (!fareDisplayInfo->isAutoPriceable())
  {
    minStayTag << MIN_MAX_STAY_NP;
    return minStayTag.str();
  }

  //  Now apply formatting logic for the Minimum Stay part of the field.

  if (fareDisplayInfo->minStay() == "///") // Check for no minimum stay validation performed
  {
    minStayTag << "";
  }
  else if (fareDisplayInfo->minStay() == "   ") // Check for no minimum stay requirement
  {
    minStayTag << "";
  }
  else if ((isalpha(fareDisplayInfo->minStay()[0])) || // check for day of week data
           (fareDisplayInfo->minStay()[0] == '$')) // check for cross-lorraine data
  {
    minStayTag << fareDisplayInfo->minStay()[0] << fareDisplayInfo->minStay()[1];
  }
  else
  {
    if (fareDisplayInfo->minStay()[1] == '0')
    {
      minStayTag << fareDisplayInfo->minStay()[2];
    }
    else
    {
      if (fareDisplayInfo->minStay().size() == 1)
      {
        minStayTag << fareDisplayInfo->minStay();
      }
      else
      {
        minStayTag << fareDisplayInfo->minStay()[1] << fareDisplayInfo->minStay()[2];
      }
    }
  }
  return minStayTag.str();
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::maxStay()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::maxStay(FareDisplayInfo* fareDisplayInfo)
{
  std::ostringstream maxStayTag;
  // Check for NP condition (Not auto-Priceable)
  if (!fareDisplayInfo->isAutoPriceable())
  {
    maxStayTag << MIN_MAX_STAY_NP;
    return maxStayTag.str();
  }
  if (fareDisplayInfo->maxStay() == "///") // Check for no maximum stay validation performed
  {
    maxStayTag << "";
  }
  else if (fareDisplayInfo->maxStay()[0] == '$') // check for cross-lorraine data
  {
    maxStayTag << fareDisplayInfo->maxStay()[0] << fareDisplayInfo->maxStay()[1] << ' ';
  }
  else if (fareDisplayInfo->maxStayUnit() == "  ") // if maxStayUnit == 2 blank space
  {
    maxStayTag << "";
  }
  else if (fareDisplayInfo->maxStay() == "   ") // Check for no maximum stay requirement
  {
    maxStayTag << "";
  }
  else if (fareDisplayInfo->maxStayUnit() == MAX_STAY_UNITS_MONTH)
  {
    if (fareDisplayInfo->maxStay()[1] != '0')
      maxStayTag << fareDisplayInfo->maxStay()[1];
    maxStayTag << fareDisplayInfo->maxStay()[2] << MAX_STAY_UNITS_MONTH[0];
  }
  else
  {
    if (fareDisplayInfo->maxStay()[0] == '0')
    {
      if (fareDisplayInfo->maxStay()[1] == '0')
      {
        if (fareDisplayInfo->maxStay()[2] == '0')
          maxStayTag << "";
        else
          maxStayTag << fareDisplayInfo->maxStay()[2];
      }
      else
        maxStayTag << fareDisplayInfo->maxStay()[1] << fareDisplayInfo->maxStay()[2];
    }
    else
      maxStayTag << fareDisplayInfo->maxStay();
  }

  return maxStayTag.str();
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::journeyType()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::journeyType(PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{

  if (trx.getOptions()->halfRoundTripFare() == 'T')
  {
    return "H";
  }
  else if (trx.getOptions()->roundTripFare() == 'T')
  {
    return "R";
  }
  else if (trx.getOptions()->oneWayFare() == 'T')
  {

    if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
      return "X";
    else
      return "O";
  }
  else // No journey type in the request
  {

    if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
      return "X";
    else if (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
      return "R";
    else if (paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
      return "O";
    else
      return "";
  }
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::netFareIndicator()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::netFareIndicator(PaxTypeFare& paxTypeFare)
{
  std::string netFareIndicatorTag = "";
  if (paxTypeFare.fareDisplayCat35Type() == RuleConst::NET_FARE)
    netFareIndicatorTag = NET_QUALIFIER; //"N"

  else if (paxTypeFare.fareDisplayCat35Type() == RuleConst::REDISTRIBUTED_FARE)
    netFareIndicatorTag = REDISTRIBUTE_QUALIFIER; //"R"

  else if (paxTypeFare.fareDisplayCat35Type() == RuleConst::SELLING_CARRIER_FARE ||
           paxTypeFare.fareDisplayCat35Type() == RuleConst::SELLING_MARKUP_FARE)
    netFareIndicatorTag = SELLING_QUALIFIER; //"S"

  return netFareIndicatorTag;
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::sameDayChange()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::sameDayChange(FareDisplayInfo*& fareDisplayInfo)
{
  std::string sameDayChangeTag = "";
  formatSameDayChangeData(*fareDisplayInfo, sameDayChangeTag);
  return sameDayChangeTag;
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::routing()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::routing(PaxTypeFare& paxTypeFare, FareDisplayInfo*& fareDisplayInfo)
{
  if (!fareDisplayInfo->routingSequence().empty())
  {
    return fareDisplayInfo->routingSequence();
  }
  else
  {
    std::ostringstream rtgNumberTag;
    rtgNumberTag << FareDisplayResponseUtil::routingNumberToString(paxTypeFare.routingNumber());
    return rtgNumberTag.str();
  }
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::season()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::season(SeasonsInfo*& seasonsInfo)
{
  ostringstream seasonsTag;

  // For FareDisplayUtil methods formatMonth and formatYear
  Indicator monthFormat = '4'; // MMM
  Indicator yearFormat = '5'; // YY

  if (!(seasonsInfo->tvlstartDay()) && !(seasonsInfo->tvlstartmonth()) &&
      !(seasonsInfo->tvlstartyear()) && !(seasonsInfo->tvlStopDay()) &&
      !(seasonsInfo->tvlStopmonth()) && !(seasonsInfo->tvlStopyear()))
  {
    seasonsTag << SEASONS_NO_DATES;
  }
  else
  {
    if (seasonsInfo->tvlstartDay())
    {
      seasonsTag.setf(std::ios::right, std::ios::adjustfield);
      seasonsTag << std::setfill('0') << std::setw(2) << seasonsInfo->tvlstartDay();
    }
    else
      seasonsTag << SEASONS_BLANK;

    FareDisplayUtil::formatMonth(seasonsInfo->tvlstartmonth(), seasonsTag, monthFormat);

    int32_t tvlStartYearValue = 0;
    if (seasonsInfo->tvlstartyear() == 0)
    {
      tvlStartYearValue = -1;
      FareDisplayUtil::formatYear(tvlStartYearValue, seasonsTag, yearFormat);
    }
    else if (seasonsInfo->tvlstartyear() > 2000)
    {
      tvlStartYearValue = seasonsInfo->tvlstartyear() - 2000;
      FareDisplayUtil::formatYear(tvlStartYearValue, seasonsTag, yearFormat);
    }
    else
      FareDisplayUtil::formatYear(seasonsInfo->tvlstartyear(), seasonsTag, yearFormat);

    seasonsTag << SEASONS_DASH;

    if (seasonsInfo->tvlStopDay())
    {
      seasonsTag.setf(std::ios::right, std::ios::adjustfield);
      seasonsTag << std::setfill('0') << std::setw(2) << seasonsInfo->tvlStopDay();
    }
    else
      seasonsTag << SEASONS_BLANK;

    FareDisplayUtil::formatMonth(seasonsInfo->tvlStopmonth(), seasonsTag, monthFormat);

    int32_t tvlStopYearValue = 0;
    if (seasonsInfo->tvlStopyear() == 0)
    {
      tvlStopYearValue = -1;
      FareDisplayUtil::formatYear(tvlStopYearValue, seasonsTag, yearFormat);
    }
    else if (seasonsInfo->tvlStopyear() > 2000)
    {
      tvlStopYearValue = seasonsInfo->tvlStopyear() - 2000;
      FareDisplayUtil::formatYear(tvlStopYearValue, seasonsTag, yearFormat);
    }
    else
      FareDisplayUtil::formatYear(seasonsInfo->tvlStopyear(), seasonsTag, yearFormat);
  }
  return seasonsTag.str();
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags::constructedFareIndicator
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::constructedFareIndicator(PaxTypeFare& paxTypeFare)
{
  std::string constructedFareIndicatorTag = (paxTypeFare.isConstructed()) ? "Y" : "N";

  return constructedFareIndicatorTag;
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags::industryFareIndicator
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::industryFareIndicator(PaxTypeFare& paxTypeFare)
{
  std::string industryFareIndicatorTag = (paxTypeFare.fare()->isIndustry()) ? "Y" : "N";

  return industryFareIndicatorTag;
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags::firstSalesDate
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::firstSalesDate(FareDisplayInfo*& fareDisplayInfo)
{
  if (!fareDisplayInfo->isAutoPriceable())
  {
    return "";
  }

  if (fareDisplayInfo->ticketInfo().size() > 0)
  {
    std::vector<TicketInfo*>::iterator tktIter = fareDisplayInfo->ticketInfo().begin();
    TicketInfo& si = **tktIter;

    if (si.earliestTktDate().isValid())
    {
      return (si.earliestTktDate().dateToString(DDMMMYY, "").c_str());
    }
  }

  return "";
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags::effectiveDate
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::effectiveDate(FareDisplayInfo*& fareDisplayInfo)
{
  if (!fareDisplayInfo->isAutoPriceable())
  {
    return "";
  }

  if (fareDisplayInfo->travelInfo().size() > 0)
  {
    std::vector<TravelInfo*>::iterator tvlIter = fareDisplayInfo->travelInfo().begin();
    TravelInfo& ti = **tvlIter;

    if (ti.earliestTvlStartDate().isValid())
    {
      return (ti.earliestTvlStartDate().dateToString(DDMMMYY, "").c_str());
    }
  }

  return "";
}

std::string
FareDisplayResponseXMLTags::expirationDate(FareDisplayInfo*& fareDisplayInfo)
{
  if (!fareDisplayInfo->isAutoPriceable())
  {
    return "";
  }

  DateTime expDate;
  if (!fareDisplayInfo->travelInfo().empty())
  {
    TravelInfo* travelInfo = fareDisplayInfo->travelInfo().front();
    expDate = travelInfo->latestTvlStartDate(); // Discontinue date
    if (expDate.isValid())
      return expDate.dateToString(DDMMMYY, "").c_str();

    expDate = travelInfo->stopDate(); // could be either Completion date or Return date
    if (expDate.isValid())
      return expDate.dateToString(DDMMMYY, "").c_str();
  }
  return "";
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags::lastTicketDate
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::lastTicketDate(FareDisplayInfo*& fareDisplayInfo)
{
  if (!fareDisplayInfo->isAutoPriceable())
  {
    return "";
  }

  if (fareDisplayInfo->ticketInfo().size() > 0)
  {
    std::vector<TicketInfo*>::iterator tktIter = fareDisplayInfo->ticketInfo().begin();
    TicketInfo& si = **tktIter;

    if (si.latestTktDate().isValid())
    {
      return (si.latestTktDate().dateToString(DDMMMYY, "").c_str());
    }
  }

  return "";
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags::allCabin
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::allCabin(PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{
  if (TrxUtil::isRbdByCabinValueInPriceResponse(trx))
  {
    return std::string(1, paxTypeFare.cabin().getClassAlphaNumAnswer());
  }
  else
  {
    if (paxTypeFare.cabin().isPremiumFirstClass())
    {
      // Premium First Cabin
      return "P";
    }
    else if (paxTypeFare.cabin().isFirstClass())
    {
      // First cabin
      return "F";
    }
    else if (paxTypeFare.cabin().isPremiumBusinessClass())
    {
      // Premium Business cabin
      return "J";
    }
    else if (paxTypeFare.cabin().isBusinessClass())
    {
      // Business cabin
      return "B";
    }
    else if (paxTypeFare.cabin().isPremiumEconomyClass())
    {
      // Premium Economy Cabin
      return "S";
    }
    // Economy cabin
    return "E";
  }
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags::typeOfFare
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::typeOfFare(PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{
  if (paxTypeFare.isWebFare()) // Web
  {
    return "W";
  }
  else if (paxTypeFare.tcrTariffCat() == RuleConst::PRIVATE_TARIFF) // Private
  {
    return "N";
  }
  else if (paxTypeFare.fcaFareType() == "SH")
  {
    return "O";
  }
  else if (paxTypeFare.actualPaxType())
  {
    PaxTypeCode paxType = paxTypeFare.actualPaxType()->paxType();
    InclusionCodeRetriever inclCdRetriever(trx);
    FareDisplayInclCd* fareDisplayInclCd = inclCdRetriever.fetch();
    if (fareDisplayInclCd != nullptr)
    {
      // get rec1 SPOUSE passenger types from data base
      std::vector<FareDispRec1PsgType*> rec1PsgTypes =
          trx.dataHandle().getFareDispRec1PsgType(fareDisplayInclCd->userApplType(),
                                                  fareDisplayInclCd->userAppl(),
                                                  fareDisplayInclCd->pseudoCityType(),
                                                  fareDisplayInclCd->pseudoCity(),
                                                  "SH");
      if (!rec1PsgTypes.empty())
      {
        // search if the actual paxtype is in the rec1 SPOUSE passenger types
        std::vector<FareDispRec1PsgType*>::const_iterator iter = rec1PsgTypes.begin();
        std::vector<FareDispRec1PsgType*>::const_iterator iterEnd = rec1PsgTypes.end();
        for (; iter != iterEnd; iter++)
        {
          if ((*iter)->psgType() == paxType)
            return "O";
        }
      }
      // get rec1 SEAMAN passenger types from data base
      rec1PsgTypes = trx.dataHandle().getFareDispRec1PsgType(fareDisplayInclCd->userApplType(),
                                                             fareDisplayInclCd->userAppl(),
                                                             fareDisplayInclCd->pseudoCityType(),
                                                             fareDisplayInclCd->pseudoCity(),
                                                             "SEA");
      if (!rec1PsgTypes.empty())
      {
        // search if the actual paxtype is in the rec1 SEAMAN passenger types
        std::vector<FareDispRec1PsgType*>::const_iterator iter = rec1PsgTypes.begin();
        std::vector<FareDispRec1PsgType*>::const_iterator iterEnd = rec1PsgTypes.end();
        for (; iter != iterEnd; iter++)
        {
          if ((*iter)->psgType() == paxType)
            return "S";
        }
      }
    } // endif - has inclusion code
  } // endelse - has good paxType

  return "P"; // if none of the above Public
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags::privateFareIndicator
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::privateFareIndicator(FareDisplayInfo*& fareDisplayInfo,
                                                  FareDisplayTrx& trx)
{
  bool isFQ = true;
  std::string privateFareIndTag = "";
  PrivateIndicator::privateIndicatorOld(fareDisplayInfo->paxTypeFare(), privateFareIndTag,
                                        true, isFQ);

  return privateFareIndTag;
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags::corporateId
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::corporateId(PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{
  if (paxTypeFare.isFareByRule())
  {
    // For Cat 25, check record 8
    FBRPaxTypeFareRuleData* fbrData = paxTypeFare.getFbrRuleData(RuleConst::FARE_BY_RULE);

    if (fbrData != nullptr && !fbrData->fbrApp()->accountCode().empty())
    {
      return (fbrData->fbrApp()->accountCode().c_str());
    }
  }

  // Check if any account code on cat 1 record 3
  if (paxTypeFare.matchedCorpID())
  {
    return (trx.getRequest()->corporateID().c_str());
  }

  return "";
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags::baseSellingAmount
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::baseSellingAmount(PaxTypeFare& paxTypeFare,
                                              const DateTime& ticketingDate)
{
  bool cat22Fare = false;

  // Check for cat 22
  if (paxTypeFare.isDiscounted())
  {
    const PaxTypeFareRuleData* ptfRuleData =
        paxTypeFare.paxTypeFareRuleData(RuleConst::CHILDREN_DISCOUNT_RULE);

    if (ptfRuleData &&
        ptfRuleData->categoryRuleInfo()->categoryNumber() == RuleConst::OTHER_DISCOUNT_RULE)
      cat22Fare = true;
  }

  // Check for cat 22 or cat 25 fares
  if (paxTypeFare.isFareByRule() || cat22Fare)
  {
    const MoneyAmount& amount = paxTypeFare.originalFareAmount();
    CurrencyCode ccode = paxTypeFare.currency();
    return formatMoneyAmount(amount, ccode, ticketingDate);
  }

  return "";
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags::baseSellingCurrency
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::baseSellingCurrency(PaxTypeFare& paxTypeFare)
{
  bool cat22Fare = false;

  // Check for cat 22
  if (paxTypeFare.isDiscounted())
  {
    PaxTypeFareRuleData* ptfRuleData = paxTypeFare.paxTypeFareRuleData(19);

    if (ptfRuleData != nullptr && ptfRuleData->categoryRuleInfo()->categoryNumber() == 22)
      cat22Fare = true;
  }

  // Check for cat 22 or cat 25 fares
  if (paxTypeFare.isFareByRule() || cat22Fare)
  {
    return paxTypeFare.fare()->currency().c_str();
  }

  return "";
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags::cat22Indicator
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::cat22Indicator(FareDisplayTrx& fareDisplayTrx,
                                           FareDisplayInfo*& fareDisplayInfo)
{

  // Get agent duty code
  Agent* agent = fareDisplayTrx.getRequest()->ticketingAgent();
  bool dutyCode78 = false;
  if ((agent->agentDuty() == "7") || (agent->agentDuty() == "8") || (agent->agentDuty() == "$"))
  {
    dutyCode78 = true;
  }
  char displayCode = fareDisplayInfo->getCategoryApplicability(dutyCode78, 22);
  if (displayCode == '*' || displayCode == '$' || displayCode == '-' || displayCode == '/')
    return "Y";
  else
    return "N";
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags::cat35FareAmount
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::cat35FareAmount(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.isNegotiated())
  {
    return formatMoneyAmount(paxTypeFare.convertedFareAmount(),
                             trx.itin().front()->calculationCurrency(),
                             trx.ticketingDate());
  }

  return "";
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags:: baseFareAmount
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::baseFareAmount(const PaxTypeFare*& baseFare,
                                           const Fare*& fare,
                                           const DateTime& ticketingDate)
{
  MoneyAmount amount;
  CurrencyCode ccode;
  if (baseFare == nullptr)
  {
    amount = fare->originalFareAmount();
    ccode = fare->currency();
  }
  else
  {
    amount = baseFare->originalFareAmount();
    ccode = baseFare->currency();
  }
  return formatMoneyAmount(const_cast<MoneyAmount&>(amount), ccode, ticketingDate);
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags:: originalFareAmount
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::originalFareAmount(const Fare*& fare, const DateTime& ticketingDate)
{
  const MoneyAmount amount = fare->originalFareAmount();
  CurrencyCode ccode = fare->currency();
  return formatMoneyAmount(amount, ccode, ticketingDate);
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags:: populateSDSTaxInfo
//--------------------------------------------------------------------------
std::vector<FareDisplaySDSTaxInfo*>
FareDisplayResponseXMLTags::getSDSTaxInfo(FareDisplayTrx& fareDisplayTrx,
                                          FareDisplayInfo*& fareDisplayInfo,
                                          PaxTypeFare& paxTypeFare)
{
  std::vector<FareDisplaySDSTaxInfo*> sdsTaxInfoVec;
  std::string owrt = journeyType(paxTypeFare, fareDisplayTrx);
  if (owrt == "H")
    return sdsTaxInfoVec; // no taxes in this case and hence returning empty vector.
  std::vector<TaxRecord*> taxRecordVec;
  std::vector<TaxItem*> taxItemVec;
  if (owrt == "X")
  {
    taxRecordVec = fareDisplayInfo->taxRecordOWRTFareTaxVector();
    taxItemVec = fareDisplayInfo->taxItemOWRTFareTaxVector();
  }
  else if (!owrt.empty())
  {
    taxRecordVec = fareDisplayInfo->taxRecordVector();
    taxItemVec = fareDisplayInfo->taxItemVector();
  }
  sdsTaxInfoVec = buildSDSTaxInfo(fareDisplayTrx, paxTypeFare, taxRecordVec, taxItemVec, owrt);
  return sdsTaxInfoVec;
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags:: buildSDSTaxInfo
//--------------------------------------------------------------------------
std::vector<FareDisplaySDSTaxInfo*>
FareDisplayResponseXMLTags::buildSDSTaxInfo(FareDisplayTrx& fareDisplayTrx,
                                            PaxTypeFare& paxTypeFare,
                                            std::vector<TaxRecord*> taxRecordVec,
                                            std::vector<TaxItem*> taxItemVec,
                                            std::string owrt)
{
  std::vector<FareDisplaySDSTaxInfo*> sdsTaxInfoVec;
  std::vector<TaxRecord*>::const_iterator taxRecIter = taxRecordVec.begin();
  std::vector<TaxRecord*>::const_iterator taxRecIterEnd = taxRecordVec.end();
  for (; taxRecIter != taxRecIterEnd; taxRecIter++)
  {
    if (FareDisplayTax::shouldBypassTax((*taxRecIter)->taxCode()))
      continue;
    MoneyAmount taxAmount = 0.0;
    if (owrt == "O" || owrt == "X") // calculate one way tax
    {
      if (!FareDisplayTax::getOWTax(
              fareDisplayTrx, paxTypeFare, (*taxRecIter)->taxCode(), taxAmount))
        continue;
    }
    else // calculate round trip tax
    {
      if (!FareDisplayTax::getRTTax(
              fareDisplayTrx, paxTypeFare, (*taxRecIter)->taxCode(), taxAmount))
        continue;
    }

    // get fee indicator and tax description
    Indicator feeInd = 'Y';
    TaxDescription taxDescription = "";
    std::vector<TaxItem*>::const_iterator taxItemIter = taxItemVec.begin();
    std::vector<TaxItem*>::const_iterator taxItemIterEnd = taxItemVec.end();
    for (; taxItemIter != taxItemIterEnd; taxItemIter++)
    {
      if ((*taxItemIter)->taxCode() == (*taxRecIter)->taxCode())
      {
        feeInd = (*taxItemIter)->feeInd();
        taxDescription = (*taxItemIter)->taxDescription();
        break;
      }
    }

    // populate sdsTaxInfo vector
    FareDisplaySDSTaxInfo* sdsTaxInfo(nullptr);
    fareDisplayTrx.dataHandle().get(sdsTaxInfo);
    sdsTaxInfo->ticketingTaxCode() = (*taxRecIter)->taxCode();
    sdsTaxInfo->taxAmount() = taxAmount;
    // Add or subtract percentage amount
    int16_t percentage = fareDisplayTrx.getRequest()->addSubLineNumber();
    sdsTaxInfo->taxAmount() *= (1 + percentage * 0.01);
    if (feeInd == 'Y')
    {
      sdsTaxInfo->feeInd() = true;
      sdsTaxInfo->taxCode() = (*taxRecIter)->taxCode() + " FEE";
    }
    else
    {
      sdsTaxInfo->feeInd() = false;
      sdsTaxInfo->taxCode() = (*taxRecIter)->taxCode() + "TAX";
    }
    sdsTaxInfo->taxDescription() = taxDescription;
    sdsTaxInfoVec.push_back(sdsTaxInfo);
  }

  return sdsTaxInfoVec;
}
//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags:: totalAmount
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::totalAmount(const std::vector<FareDisplaySDSTaxInfo*>& sdsTaxInfo,
                                        FareDisplayTrx& fareDisplayTrx,
                                        PaxTypeFare& paxTypeFare)
{
  MoneyAmount fareAmount;
  fareAmount = paxTypeFare.convertedFareAmount(); // HITHA >>>>check this
  if (fareDisplayTrx.getOptions()->halfRoundTripFare() == 'T') //"H"
    fareAmount /= 2;
  if (!sdsTaxInfo.empty())
  {
    fareAmount += getTotalTax(sdsTaxInfo);
  }
  CurrencyCode ccode = fareDisplayTrx.itin().front()->calculationCurrency();
  return formatMoneyAmount(fareAmount, ccode, fareDisplayTrx.ticketingDate());
}

//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags:: getTotalTax
//--------------------------------------------------------------------------
MoneyAmount
FareDisplayResponseXMLTags::getTotalTax(const std::vector<FareDisplaySDSTaxInfo*>& sdsTaxInfo)
{
  MoneyAmount totalTax = 0.0;
  if (!sdsTaxInfo.empty())
  {
    // get the total
    std::vector<FareDisplaySDSTaxInfo*>::const_iterator sdsTaxInfoIter = sdsTaxInfo.begin();
    std::vector<FareDisplaySDSTaxInfo*>::const_iterator sdsTaxInfoIterEnd = sdsTaxInfo.end();
    for (; sdsTaxInfoIter != sdsTaxInfoIterEnd; sdsTaxInfoIter++)
    {
      totalTax += (*sdsTaxInfoIter)->taxAmount();
    }
  }
  return totalTax;
}
//--------------------------------------------------------------------------
// FareDisplayResponseXMLTags::pricingTicketingRestriction
//--------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::pricingTicketingRestriction(PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.fareDisplayCat35Type() == RuleConst::SELLING_MARKUP_FARE ||
      paxTypeFare.fareDisplayCat35Type() == RuleConst::SELLING_CARRIER_FARE)
  {
    const NegPaxTypeFareRuleData* negPTFareRuleData = paxTypeFare.getNegRuleData();

    if (negPTFareRuleData)
    {
      const NegFareSecurityInfo* negFareSecurity = negPTFareRuleData->securityRec();

      // Check Sell/Ticketing restriction
      if (negFareSecurity != nullptr)
      {
        if (negFareSecurity->sellInd() == NO_INDICATOR &&
            negFareSecurity->ticketInd() == NO_INDICATOR)
        {
          // Sell/Ticketing restriction
          return "B";
        }
        else if (negFareSecurity->ticketInd() == NO_INDICATOR)
        {
          // Ticketing restriction
          return "T";
        }
        else if (negFareSecurity->sellInd() == NO_INDICATOR)
        {
          // Sell/Pricing restriction
          return "P";
        }
        else
        {
          // No restriction
          return "N";
        }
      }
    }
  }

  return "";
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::oneWayFare()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::oneWayFare(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare)
{
  if ((paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED) ||
      ((paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED) &&
       (trx.getOptions()->roundTripFare() == 'T')))
    return "";
  else
  {
    MoneyAmount fareAmount = paxTypeFare.convertedFareAmount();
    return formatMoneyAmount(
        fareAmount, trx.itin().front()->calculationCurrency(), trx.ticketingDate());
  }
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::roundTripFare()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::roundTripFare(FareDisplayTrx& trx, PaxTypeFare& paxTypeFare)
{
  if ((paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED) || (trx.getOptions()->oneWayFare() == 'T'))
    return "";
  else if ((paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED) &&
           (trx.getOptions()->doubleForRoundTrip() == 'N') &&
           (trx.getOptions()->roundTripFare() != 'T'))
    return "";
  else
  {
    MoneyAmount fareAmount;

    if (paxTypeFare.isRoundTrip())
      fareAmount = paxTypeFare.convertedFareAmount();
    else
      fareAmount = paxTypeFare.convertedFareAmountRT();

    if (trx.getOptions()->halfRoundTripFare() == 'T') //"H"
      fareAmount /= 2;

    return formatMoneyAmount(
        fareAmount, trx.itin().front()->calculationCurrency(), trx.ticketingDate());
  }
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::formatMoneyAmount()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::formatMoneyAmount(const MoneyAmount& amount,
                                              CurrencyCode& ccode,
                                              const DateTime& ticketingDate)

{
  Money moneyPayment(ccode);
  std::ostringstream formattedAmount;
  formattedAmount.setf(ios::fixed);
  formattedAmount.precision(moneyPayment.noDec(ticketingDate));
  formattedAmount << amount;
  return formattedAmount.str();
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::formatMoneyAmount()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::formatMoneyAmount(const MoneyAmount& amount, CurrencyNoDec& noDec)

{
  std::ostringstream formattedAmount;
  formattedAmount.setf(ios::fixed);
  formattedAmount.precision(noDec);
  formattedAmount << amount;
  return formattedAmount.str();
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::formatSameDayChangeData()
//---------------------------------------------------------------------------
void
FareDisplayResponseXMLTags::formatSameDayChangeData(FareDisplayInfo& fareDisplayInfo,
                                                    std::string& sameDayChangeTag)
{
  // ================================================
  // Same Day Change Indicator Logic follows
  // ================================================
  bool isWithin24 = false;
  TimeDuration oneDay(24, 0, 0);
  const Fare* f = fareDisplayInfo.paxTypeFare().fare();
  DateTime expDate = f->expirationDate();
  DateTime createDate = f->createDate();
  DateTime modDate = f->lastModDate();
  DateTime now = DateTime::localTime();

  // First establish if the modDate falls within the last 24 hours.
  // This window of time determines if anything might be displayed.
  // If the modDate is outside of this window, nothing else
  // matters.
  DateTime TwentyFourHoursAgo(now, -oneDay);
  isWithin24 = modDate.isBetween(TwentyFourHoursAgo, now);

  if (isWithin24)
  {
    // First check for Fare deleted in last 24 hours
    if (expDate == modDate)
    {
      // It's been discontinued, so show it as Fare Deleted
      sameDayChangeTag = DELETED_INDICATOR;
    }
    // Next check for Fare added in last 24 hours
    else if (createDate == modDate)
    {
      // It's been discontinued, so show it as Fare Deleted
      sameDayChangeTag = ADDED_INDICATOR;
    }
    else if (f->increasedFareAmtTag() == YES_INDICATOR)
    {
      sameDayChangeTag = INCREASED_INDICATOR;
    }

    else if (f->reducedFareAmtTag() == YES_INDICATOR)
    {
      sameDayChangeTag = REDUCED_INDICATOR;
    }
    else if ((f->footnoteTag() == YES_INDICATOR) || (f->routingTag() == YES_INDICATOR) ||
             (f->effectiveDateTag() == YES_INDICATOR))
    {
      sameDayChangeTag = CHANGED_INDICATOR;
    }
    else
    {
      // No change indicators, but mode date is within 24 hours
      // so just show it as added.
      sameDayChangeTag = ADDED_INDICATOR;
    }
  }
  else
  {
    // ModDate is not within last 24 hours.
    sameDayChangeTag = BLANK_INDICATOR;
  }
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::yyFareMsg()
//---------------------------------------------------------------------------
std::string
FareDisplayResponseXMLTags::yyFareMsg(FareDisplayTrx& trx)
{
  std::ostringstream s;

  // loop through all fares in order to display header messages
  // when no valid YY fares exist in the fare markets

  if (trx.isDomestic() || trx.isShopperRequest() ||
      trx.getRequest()->requestType() == FARE_MILEAGE_REQUEST)
    return "";

  InclusionCode incCode;
  if (trx.getRequest()->inclusionCode() != ALL_INCLUSION_CODE)
  {
    incCode = trx.getRequest()->inclusionCode();
  }
  else
  {
    incCode = "REQUESTED";
  }

  std::string privateString = "";
  if ((trx.getOptions()->isPrivateFares()) && (incCode == "REQUESTED"))
    privateString = " PRVT";
  else if (trx.getOptions()->isPrivateFares())
    privateString = " PRIVATE";

  Itin* itin = trx.itin().front();
  std::vector<FareMarket*>::const_iterator fmItr = itin->fareMarket().begin();
  std::vector<FareMarket*>::const_iterator fmEnd = itin->fareMarket().end();

  CarrierCode carrierCode;
  int countYY = 0, countValidYY = 0, fareCount = 0;

  for (; fmItr != fmEnd; fmItr++)
  {
    FareMarket* fareMarket = (*fmItr);
    std::vector<PaxTypeFare*>::const_iterator ptfItr = fareMarket->allPaxTypeFare().begin();
    std::vector<PaxTypeFare*>::const_iterator ptfEnd = fareMarket->allPaxTypeFare().end();

    for (; ptfItr != ptfEnd; ptfItr++)
    {
      PaxTypeFare* paxTypeFare = (*ptfItr);
      carrierCode = paxTypeFare->carrier();

      if (carrierCode == INDUSTRY_CARRIER)
      {
        countYY++;
        if (paxTypeFare->isValidForPricing())
          countValidYY++;
      }
      else if (carrierCode != INDUSTRY_CARRIER && paxTypeFare->isValidForPricing())
        fareCount++;
    }
  }

  if (countYY > 0)
  {
    const CarrierCode& carrier = trx.requestedCarrier();
    // YY fares are published
    if (countValidYY == 0)
    {
      s << "*** YY " << incCode << privateString << " FARES NOT PERMITTED " << trx.boardMultiCity()
        << DASH << trx.offMultiCity() << " ON " << carrier << " ***";
    }
    else
    {
      // There are valid YY fares
      s << "*** BELOW ARE YY " << incCode << privateString << " FARES " << trx.boardMultiCity()
        << DASH << trx.offMultiCity() << " VALID ON " << carrier << " ***";
    }
  }
  else
  {
    s << "*** THERE ARE NO YY";

    if ((incCode != "NLX") || ((trx.boardMultiCity()) != (trx.offMultiCity())))
    {
      s << " " << incCode;
    }
    s << privateString;

    s << " FARES PUBLISHED " << trx.boardMultiCity() << "-" << trx.offMultiCity() << " ***";
  }
  return s.str();
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::hdrMsgs()
//---------------------------------------------------------------------------
void
FareDisplayResponseXMLTags::hdrMsgs(FareDisplayTrx& trx, std::vector<std::string>& msgs)
{
  std::vector<tse::FDHeaderMsgText*>& headerMsgList(trx.fdResponse()->headerMsgs());

  if (!headerMsgList.empty())
  {
    std::vector<tse::FDHeaderMsgText*>::iterator iB = headerMsgList.begin();
    std::vector<tse::FDHeaderMsgText*>::iterator iE = headerMsgList.end();

    while (iB != iE)
    {
      if ((*iB)->text().length() > 0)
        msgs.push_back((*iB)->text());

      iB++;
    }
  }

  altCurrencyHdrMsgs(trx, msgs);
}

//---------------------------------------------------------------------------
// FareDisplayResponseXMLTags::altCurrencyHdrMsgs()
//---------------------------------------------------------------------------
void
FareDisplayResponseXMLTags::altCurrencyHdrMsgs(FareDisplayTrx& trx, std::vector<std::string>& msgs)
{
  // For domestic markets donot display Alternate Currency
  // Header messages
  if (trx.isDomestic())
    return;

  // Get alternate currencies
  std::set<CurrencyCode> alternateCurrencies;
  trx.getAlternateCurrencies(alternateCurrencies);

  if (alternateCurrencies.empty())
    return;

  std::ostringstream s;
  s << "MORE FARES FILED IN ";
  for (set<CurrencyCode>::const_iterator i = alternateCurrencies.begin();
       i != alternateCurrencies.end();
       i++)
  {
    if (i != alternateCurrencies.begin())
      s << BLANK_SPACE;

    s << *i;
  }
  msgs.push_back(s.str());
}

void
FareDisplayResponseXMLTags::currencyConversionHdrMsgs(FareDisplayTrx& trx,
                                                      std::vector<std::string>& msgs)
{
  if (trx.allPaxTypeFare().empty())
  {
    // LOG4CXX_DEBUG(logger, "buildDisplay:: allPaxTypeFare is empty. Going back");
    return;
  }

  // Get Display Currency
  // CurrencyCode displayCurrency;
  // FareDisplayUtil::getDisplayCurrency(trx, displayCurrency);
  Itin* itin = trx.itin().front();
  CurrencyCode displayCurrency = itin->calculationCurrency();

  const PaxTypeFare& paxTypeFare = *(trx.allPaxTypeFare().front());

  ostringstream oss[2];
  /*
      initializeLine(&oss[0], 1);
      initializeLine(&oss[1], 1);
      oss[0].seekp(0, ios_base::beg);
      oss[1].seekp(0, ios_base::beg);
  */

  // Display Multiple Currency Header
  if (trx.multipleCurrencies())
  {
    if (displayCurrency == NUC)
    {
      oss[0] << "MULTIPLE SELLING CURRENCIES CONVERTED TO " << displayCurrency
             << " USING CURRENT ROE";
    }
    else
    {
      oss[0] << "MULTIPLE SELLING CURRENCIES CONVERTED TO " << displayCurrency
             << " USING CURRENT BSR";
    }

    msgs.push_back(oss[0].str());
    return;
  }

  // Get Currency Converion Info
  CurrencyCode sourceCurrency;
  CurrencyCode targetCurrency;
  CurrencyCode intermediateCurrency;
  ExchRate exchangeRate1;
  ExchRate exchangeRate2;

  if (!getCurrencyConversionInfo(
          trx, sourceCurrency, targetCurrency, intermediateCurrency, exchangeRate1, exchangeRate2))
  {
    return;
  }

  // Display NUC Conversion Header
  if (displayCurrency == NUC)
  {
    oss[0] << paxTypeFare.currency() << " CONVERTED TO " << displayCurrency << " USING ROE ";
    oss[0].setf(std::ios::fixed, std::ios::floatfield);
    oss[0].setf(std::ios::right, std::ios::adjustfield);
    oss[0] << setw(8) << exchangeRate1 << " - 1 NUC";

    msgs.push_back(oss[0].str());
    return;
  }

  if (TrxUtil::isIcerActivated(trx))
  {
    if (sourceCurrency != targetCurrency)
    {
      oss[0] << sourceCurrency << " CONVERTED TO " << targetCurrency;
      oss[0] << " USING BSR 1 " << sourceCurrency << " - " << exchangeRate1 << " "
             << targetCurrency;
      msgs.push_back(oss[0].str());
    }
    return;
  }

  // Display BSR Currency Conversion Header - Single Currency Conversion
  if (intermediateCurrency.empty())
  {
    if (sourceCurrency != targetCurrency)
    {
      oss[0] << sourceCurrency << " CONVERTED TO " << targetCurrency;
    }

    CurrencyCode aaaCurrency = trx.getRequest()->ticketingAgent()->currencyCodeAgent();
    LocCode aaaLoc = trx.getRequest()->ticketingAgent()->agentCity();
    if (!(trx.getRequest()->salePointOverride().empty()))
    {
      aaaLoc = trx.getRequest()->salePointOverride();
    }

    const Loc* loc = trx.dataHandle().getLoc(aaaLoc, trx.travelDate());
    if (loc)
    {
      NationCode nation = loc->nation();
      CurrencyUtil::getNationCurrency(nation, aaaCurrency, trx.ticketingDate());
    }

    if (targetCurrency == aaaCurrency)
    {
      oss[0] << " USING BSR 1 " << targetCurrency << " - " << exchangeRate1 << " "
             << sourceCurrency;
    }
    else
    {
      oss[0] << " USING BSR 1 " << sourceCurrency << " - " << exchangeRate1 << " "
             << targetCurrency;
    }

    if (!oss[0].str().empty())
    {
      msgs.push_back(oss[0].str());
    }

    return;
  }

  // -------------------------------------------------------------------
  // Display BSR Currency Conversion Header - Double Currency Conversion
  // If IntermediateCurrency Present
  // -------------------------------------------------------------------

  // SourceCur to intermediateCur using bsr 1 intermediateCur - sourceCur
  // IntermediateCur to TargetCur using bsr 1 IntermediateCur - TargetCur

  if (sourceCurrency != intermediateCurrency)
  {
    oss[0] << sourceCurrency << " CONVERTED TO " << intermediateCurrency << " USING BSR 1 "
           << intermediateCurrency << " - ";
    oss[0].setf(std::ios::fixed, std::ios::floatfield);
    oss[0].setf(std::ios::right, std::ios::adjustfield);
    oss[0] << setw(8) << exchangeRate1 << " " << sourceCurrency;
  }

  if (intermediateCurrency != targetCurrency)
  {
    // --------------------------
    // Generate Conversion Line2
    // --------------------------
    if (sourceCurrency != intermediateCurrency)
    {
      oss[1] << "THEN " << intermediateCurrency << " CONVERTED TO " << targetCurrency
             << " USING BSR 1 " << intermediateCurrency << " - ";
      oss[1].setf(std::ios::fixed, std::ios::floatfield);
      oss[1].setf(std::ios::right, std::ios::adjustfield);
      oss[1] << setw(8) << exchangeRate2;
      oss[1] << " " << targetCurrency;
    }
    else
    {
      oss[0] << intermediateCurrency << " CONVERTED TO " << targetCurrency << " USING BSR 1 "
             << intermediateCurrency << " - " << exchangeRate2 << " " << targetCurrency;
    }
  }

  msgs.push_back(oss[0].str());

  if (!oss[1].str().empty())
  {
    msgs.push_back(oss[1].str());
  }

  // LOG4CXX_DEBUG(logger, "Leaving FQCurrencyConversionSection");
  return;
}

bool
FareDisplayResponseXMLTags::getCurrencyConversionInfo(FareDisplayTrx& trx,
                                                      CurrencyCode& sourceCurrency,
                                                      CurrencyCode& targetCurrency,
                                                      CurrencyCode& intermediateCurrency,
                                                      ExchRate& exchangeRate1,
                                                      ExchRate& exchangeRate2)
{
  // Get Source Currency
  const PaxTypeFare& paxTypeFare = *(trx.allPaxTypeFare().front());
  const Money sourceMoney(paxTypeFare.originalFareAmount(), paxTypeFare.currency());

  // Get Target Currency for Display
  // CurrencyCode displayCurrency;
  // FareDisplayUtil::getDisplayCurrency(trx, displayCurrency);
  // Money targetMoney(displayCurrency);
  Itin* itin = trx.itin().front();
  CurrencyCode displayCurrency = itin->calculationCurrency();
  Money targetMoney(displayCurrency);

  if (paxTypeFare.currency() == displayCurrency)
    return false;

  // ----------------------------------
  // NUC Conversion
  // ----------------------------------
  if (displayCurrency == NUC)
  {
    NUCCurrencyConverter nucConverter;
    Money amountNUC(0, NUC);

    Money amount(paxTypeFare.originalFareAmount(), paxTypeFare.currency());

    NUCCollectionResults nucResults;
    nucResults.collect() = true;

    CurrencyConversionRequest curConvReq(amountNUC,
                                         amount,
                                         trx.getRequest()->ticketingDT(),
                                         *(trx.getRequest()),
                                         trx.dataHandle(),
                                         paxTypeFare.isInternational(),
                                         CurrencyConversionRequest::FAREDISPLAY);

    bool convertRC = nucConverter.convert(curConvReq, &nucResults);
    if (!convertRC)
    {
      return false;
    }
    else
    {
      exchangeRate1 = nucResults.exchangeRate();
      sourceCurrency = paxTypeFare.currency();
      targetCurrency = displayCurrency;
    }

    // LOG4CXX_DEBUG(logger,"ROE Exchange Rate " << nucResults.exchangeRate());
    return true;
  }

  CurrencyConversionRequest request(targetMoney,
                                    sourceMoney,
                                    trx.getRequest()->ticketingDT(),
                                    *(trx.getRequest()),
                                    trx.dataHandle(),
                                    false,
                                    CurrencyConversionRequest::FAREDISPLAY,
                                    false,
                                    trx.getOptions());

  BSRCurrencyConverter bsrConverter;
  BSRCollectionResults bsrResults;
  bsrResults.collect() = true;

  try
  {
    bool rc = bsrConverter.convert(request, &bsrResults);
    if (!rc)
    {
      // LOG4CXX_ERROR(logger, "BSR Rate:"<< paxTypeFare.currency()
      //        << " AND " << displayCurrency << " WAS NOT AVAILABLE");
      throw ErrorResponseException(ErrorResponseException::UNABLE_TO_CALCULATE_BSR_NOT_AVAILABLE);
    }
  }
  catch (tse::ErrorResponseException& ex)
  {
    // LOG4CXX_ERROR(logger,"BSR Converter exception: " << ex.what());
    throw NonFatalErrorResponseException(ex.code(), ex.what());
  }

  sourceCurrency = bsrResults.sourceCurrency();
  targetCurrency = bsrResults.targetCurrency();
  intermediateCurrency = bsrResults.intermediateCurrency();
  exchangeRate1 = bsrResults.exchangeRate1();
  exchangeRate2 = bsrResults.exchangeRate2();

  return true;
}

} // tse
