//-------------------------------------------------------------------
//
//  File:        ElementFilter.cpp
//  Authors:     Mike Carroll
//  Created:     July 24, 2005
//  Description: Base class for a data filter
//
//
//  Copyright Sabre 2003
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

#include "FareDisplay/Templates/ElementFilter.h"
#include "Common/FallbackUtil.h"
#include "Common/CurrencyConversionFacade.h"
#include "Common/CurrencyConversionRequest.h"
#include "Common/CurrencyRoundingUtil.h"
#include "Common/CurrencyUtil.h"
#include "Common/FareDisplayResponseUtil.h"
#include "Common/FareDisplayTax.h"
#include "Common/FareDisplayUtil.h"
#include "Common/Money.h"
#include "Common/RoutingUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/Vendor.h"
#include "DataModel/AdjustedSellingCalcData.h"
#include "DataModel/Agent.h"
#include "DataModel/FBRPaxTypeFareRuleData.h"
#include "DataModel/FareDisplayOptions.h"
#include "DataModel/FareDisplayRequest.h"
#include "DataModel/FareDisplayTrx.h"
#include "DataModel/IndustryFare.h"
#include "DataModel/NegPaxTypeFareRuleData.h"
#include "DataModel/PaxTypeFare.h"
#include "DataModel/PaxTypeFareRuleData.h"
#include "DataModel/PrivateIndicator.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareTypeMatrix.h"
#include "DBAccess/FDAddOnFareInfo.h"
#include "FareDisplay/Templates/AdvancePurchaseFilter.h"
#include "FareDisplay/Templates/ElementField.h"
#include "FareDisplay/Templates/FareDisplayConsts.h"
#include "FareDisplay/Templates/InOutIndFilter.h"
#include "FareDisplay/Templates/MinMaxFilter.h"
#include "FareDisplay/Templates/SameDayChangeFilter.h"
#include "FareDisplay/Templates/SeasonFilter.h"
#include "FareDisplay/Templates/TravelTicketFilter.h"
#include "Fares/FDFareCurrencySelection.h"
#include "Routing/RoutingConsts.h"
#include "Rules/RuleConst.h"

namespace tse
{
FALLBACK_DECL(fallbackFixFQRedstColumnError);

namespace ElementFilter
{
void
vendorCode(Field& field, FareDisplayTrx& trx, bool vendorFWS)
{

  if (field.strValue() == Vendor::ATPCO)
    field.strValue() = " ";
  else if (field.strValue() == Vendor::SITA)
    field.strValue() = ".";
  else if (field.strValue() == Vendor::SABD)
    field.strValue() = "D";
  else if (field.strValue() == Vendor::HIKE)
    field.strValue() = "H";
  else if (trx.dataHandle().getVendorType(field.strValue()) == TRAVEL_AGENCY)
  {
    if (vendorFWS)
      field.strValue() = "W";
    else
      field.strValue() = "F";
  }
  else if (trx.dataHandle().getVendorType(field.strValue()) == VENDOR_CARRIER)
    field.strValue() = "D";
  else if (field.strValue() == Vendor::FMS || field.strValue() == Vendor::POFO)
    field.strValue() = "P";
  else
    field.strValue() = "F";
}

void
currencyCode(Field& field, FareDisplayTrx& trx)
{
  // As Addon Fare doesn't pass through FVO, itin::calculationCurrency() doesn't hold
  // the appropriate displaycurrency
  if (trx.getRequest()->inclusionCode() == FD_ADDON)
  {
    CurrencyCode displayCurrency;
    FDFareCurrencySelection::getDisplayCurrency(trx, displayCurrency);
    field.strValue() = displayCurrency;
    return;
  }
  Itin* itin = trx.itin().front();
  field.strValue() = itin->calculationCurrency();
}

void
formatMoneyAmount(Field& field, FareDisplayTrx& trx)
{

  // Get the display Currency from the agent AAA or override currency
  currencyCode(field, trx);
  Money moneyPayment(field.strValue());
  char tmpBuf[20];
  sprintf(tmpBuf, "%.*f", moneyPayment.noDec(trx.ticketingDate()), field.moneyValue());
  field.strValue() = tmpBuf;
}

void
formatMoneyAmount(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{

  Itin* itin = trx.itin().front();
  CurrencyRoundingUtil curRoundingUtil;
  field.moneyValue() = curRoundingUtil.roundMoneyAmount(
      field.moneyValue(), itin->calculationCurrency(), paxTypeFare.currency(), trx);
  currencyCode(field, trx);
  Money moneyPayment(field.strValue());
  char tmpBuf[20];
  sprintf(tmpBuf, "%.*f", moneyPayment.noDec(trx.ticketingDate()), field.moneyValue());
  field.strValue() = tmpBuf;
}

void
formatOriginalFare(Field& field, FareDisplayTrx& trx, const CurrencyCode& currency)
{
  field.strValue() = currency;
  Money moneyPayment(field.strValue());
  char tmpBuf[20];
  sprintf(tmpBuf, "%.*f", moneyPayment.noDec(trx.ticketingDate()), field.moneyValue());
  field.strValue() = tmpBuf;
}

void
travelTicket(Field& field, Field& field2, FareDisplayInfo* fareDisplayInfo)
{
  if (fareDisplayInfo == nullptr)
    return;

  TravelTicketFilter::formatData(*fareDisplayInfo, field, field2);
}

void
advancePurchase(Field& field, FareDisplayInfo* fareDisplayInfo)
{
  if (fareDisplayInfo == nullptr)
    return;

  AdvancePurchaseFilter::formatData(*fareDisplayInfo, field);
}

void
minMaxStay(Field& field, FareDisplayInfo* fareDisplayInfo)
{
  if (fareDisplayInfo == nullptr)
    return;

  MinMaxFilter::formatData(*fareDisplayInfo, field);
}

void
routing(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{
  if (field.valueFieldSize() == 1)
  {
    if (trx.isDomestic())
      field.strValue() = IS_ROUTING;
    else if (RoutingUtil::isRouting(paxTypeFare, trx))
    {
      field.strValue() = IS_ROUTING;
    }
    else
      field.strValue() = IS_NOT_ROUTING;
  }
  else
  {
    // Get the FareDislayInfo
    FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();

    if (fareDisplayInfo == nullptr)
      return;

    if (fareDisplayInfo->routingSequence().empty() || fareDisplayInfo->routingSequence() == "0")
    {
      if (paxTypeFare.routingNumber() == "SEVN" || paxTypeFare.routingNumber() == "EIGH")
      {
        field.strValue() = " ANY";
      }
      else
      {
        field.strValue() =
            FareDisplayResponseUtil::routingNumberToString(paxTypeFare.routingNumber());
      }
    }
    else
    {
      field.strValue() = fareDisplayInfo->routingSequence();
    }
  }
}

void
journeyType(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{

  if (trx.getOptions()->halfRoundTripFare() == 'T')
  {
    if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
      field.strValue() = "X";
    else
      field.strValue() = "H";
  }
  else if (trx.getOptions()->roundTripFare() == 'T')
  {
    field.strValue() = "R";
  }
  else if (trx.getOptions()->oneWayFare() == 'T')
  {
    if (!trx.isTwoColumnTemplate()) // for 1-column display "X" or "O"
    {
      if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
        field.strValue() = "X";
      else
        field.strValue() = "O";
    }
    else
      field.strValue() = " "; // nothing for 2-column display
  }
  else // No journey type in the request
  {
    if (!trx.isTwoColumnTemplate()) // for 1-column display
    {
      if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
        field.strValue() = "X";
      else if (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
        field.strValue() = "R";
      else if (paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
        field.strValue() = "O";
      else
        field.strValue() = " ";
    }
    else // for 2-column display
    {
      if (trx.getOptions()->journeyInd() == YES)
      {
        if (((paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED) &&
             (trx.getOptions()->doubleForRoundTrip() == YES)) ||
            (paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED))
          field.strValue() = "R";
        else
          field.strValue() = " ";
      }
      else
        field.strValue() = " ";
    }
  }
}

void
privateFareIndicator(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{
  FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();

  if (fareDisplayInfo == nullptr)
    return;

  PrivateIndicator filter;
  bool isFQ = true;
  filter.privateIndicatorOld(paxTypeFare, field.strValue(), true, isFQ);
}

void
passFail(Field& field)
{
  if (field.boolValue())
    field.strValue() = CAT_PASSED;
  else
    field.strValue() = CAT_FAILED;
}

void
paxType(Field& field)
{
  if (field.strValue().empty())
    field.strValue() = EMPTY_PAX_TYPE;
}

void
fareClassification(Field& field, PaxTypeFare& paxTypeFare)
{

  //  Fare Type Codes
  //
  //  Cat 25 Fare by Rule Fare ................B
  //  Discounted Cat 25 Fare by Rule Fare .....Z
  //  Discounted Fare  (Cats 19-20-21-22) .....D
  //  Negotiated Cat 35 Fare ..................N
  //  Negotiated Cat 35 Fare .................. @  (Append @)
  //  Industry MultiLateral Fare ..............M
  //  Industry Fare ...........................Y
  //  Valid Industry Fare ..................... -  (Append -)
  //  Constructed Fare ........................C
  //  Published Fare ..........................P
  //  Web Fare ................................ *  (Append *)
  //
  if (paxTypeFare.isDiscounted())
  {
    if (paxTypeFare.isFareByRule())
    {
      // Discounted Fare by Rule Fare = Z
      field.strValue() = "Z";
    }
    else
    {
      // Discounted Fare = D
      field.strValue() = "D";
    }
    if (paxTypeFare.isNegotiated())
    {
      // Negotiated Fare - Append @
      field.strValue().append("@");
    }
  }
  else if (paxTypeFare.isFareByRule())
  {
    field.strValue() = "B";
    if (paxTypeFare.isNegotiated())
    {
      field.strValue().append("@");
    }
  }

  else if (paxTypeFare.isNegotiated())
  {
    field.strValue() = "N";
  }

  else if (paxTypeFare.fare()->isIndustry())
  {
    const IndustryFare* indFare = dynamic_cast<const IndustryFare*>(paxTypeFare.fare());
    if ((indFare != nullptr) && (indFare->isMultiLateral()))
    {
      field.strValue() = "M";
    }
    else
    {
      field.strValue() = "Y";
    }

    if ((indFare != nullptr) && (!indFare->validForPricing()))
    {
      field.strValue().append("-");
    }
  }
  else if (paxTypeFare.fare()->isConstructed())
  {
    field.strValue() = "C";
  }
  else
  {
    field.strValue() = "P";
  }

  if (paxTypeFare.isWebFare())
  {
    if (field.strValue().length() > 1)
    {
      field.strValue().replace(1, 1, "*");
    }
    else
    {
      field.strValue().append("*");
    }
  }
}

void
fareByRuleInd(Field& field, const PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.isFareByRule())
    field.strValue() = "Y";
  else
    field.strValue() = EMPTY_STRING();
}

void
ruleTariff(Field& field, const PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.tcrRuleTariff())
    field.strValue() = boost::lexical_cast<std::string>(paxTypeFare.tcrRuleTariff());
  else
    field.strValue() = EMPTY_STRING();
}

void
fareRule(Field& field, const PaxTypeFare& paxTypeFare)
{
  field.strValue() = paxTypeFare.ruleNumber();
}

void
fareType(Field& field, const PaxTypeFare& paxTypeFare)
{
  field.strValue() = paxTypeFare.fcaFareType().c_str();
}

void
routingTypeNumber(Field& field, const PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{
  if(!RoutingUtil::isRouting(paxTypeFare, trx))
    field.strValue() = "MPM";
  else
  {
    field.strValue() = "";
    bool isConstr = paxTypeFare.isConstructed();
    RoutingNumber routingNumber = paxTypeFare.routingNumber();

    if (routingNumber.empty() && paxTypeFare.hasConstructedRouting())
    {
      isConstr = true;
      routingNumber = paxTypeFare.fare()->constructedFareInfo()->fareInfo().routingNumber();
    }

    if(isConstr)
      field.strValue() = 'C';
    field.strValue() += routingNumber;
  }
}

void
redistribution(Field& field, const PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{
  if (paxTypeFare.hasCat35Filed())
  {
    const NegPaxTypeFareRuleData* negRuleData = paxTypeFare.getNegRuleData();
    if (negRuleData && negRuleData->fareRetailerRuleId())
    {
      if (!fallback::fallbackFixFQRedstColumnError(&trx))
      {
        PseudoCityCode sourcePCC = negRuleData->sourcePseudoCity();
        const Agent& agent = *(trx.getRequest()->ticketingAgent());
        if ((sourcePCC != agent.tvlAgencyPCC()) && (sourcePCC != agent.mainTvlAgencyPCC()))
          field.strValue() = sourcePCC;
      }
      else
        field.strValue() = negRuleData->sourcePseudoCity();
      return;
    }
  }
  field.strValue() = EMPTY_STRING();
}

void
netFareIndicator(Field& field, const PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{
  field.strValue() = EMPTY_STRING();

  if(!TrxUtil::isFqFareRetailerEnabled(trx))
  {
    if (trx.getRequest()->inclusionCode() == FD_NET)
    {
      if (paxTypeFare.fareDisplayCat35Type() == RuleConst::NET_FARE)
        field.strValue() = NET_QUALIFIER;
      else if (paxTypeFare.fareDisplayCat35Type() == RuleConst::REDISTRIBUTED_FARE)
        field.strValue() = REDISTRIBUTE_QUALIFIER;
    }
    return;
  }

  if (paxTypeFare.getAdjustedSellingCalcData() && !trx.getOptions()->isXRSForFRRule())
    field.strValue() = SELLING_QUALIFIER;
  else if(trx.getOptions()->isPDOForFRRule() && paxTypeFare.isAdjustedSellingBaseFare())
    field.strValue() = ORIGINAL_SELLING_QUALIFIER;
  else if (trx.getRequest()->inclusionCode() == FD_NET)
  {
    if (paxTypeFare.fareDisplayCat35Type() == RuleConst::NET_FARE)
      field.strValue() = NET_QUALIFIER;
    else if (paxTypeFare.fareDisplayCat35Type() == RuleConst::REDISTRIBUTED_FARE)
      field.strValue() = REDISTRIBUTE_QUALIFIER;
  }
}

void
displayCatType(Field& field, const PaxTypeFare& paxTypeFare)
{
  if (isalpha(paxTypeFare.fcaDisplayCatType()))
    field.strValue() = paxTypeFare.fcaDisplayCatType();
  else
    field.strValue() = EMPTY_STRING();
}

bool
isAutoPriceNotSupported(const PaxTypeFare& ptf, FareDisplayTrx& trx)
{
  if (ptf.vendor() == Vendor::ATPCO)
  {
    switch (ptf.fareTariff())
    {
    case 202: // RWRPV
      return true;
    case 334: // RW1
    case 340: // CT1
    case 346: // RW2
    case 430: // CT2
      return false;
    }
  }

  if (ptf.vendor() == Vendor::SITA)
  {
    switch (ptf.fareTariff())
    {
    case 334: // around the world - 334SITA
    case 340: // circle trip - 340SITA
      return true;
    }
  }

  return false;
}

void
autoPrice(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{
  if (isAutoPriceNotSupported(paxTypeFare, trx))
  {
    field.strValue() = AUTO_PRICE_NOT_SUPPORTED;
    return;
  }

  // Get the FareDislayInfo
  FareDisplayInfo* fareDisplayInfo = paxTypeFare.fareDisplayInfo();

  if (fareDisplayInfo == nullptr)
    return;

  if (fareDisplayInfo->isAutoPriceable())
    field.strValue() = AUTO_PRICE_YES;
  else if (fareDisplayInfo->displayOnly())
    field.strValue() = AUTO_PRICE_NO_INHIBITED;
  else if (fareDisplayInfo->incompleteR3Rule())
    field.strValue() = AUTO_PRICE_NO_INCOMPLETE_R3;
  else if (fareDisplayInfo->unavailableR1Rule())
    field.strValue() = AUTO_PRICE_NO_UNAVAILABLE_R1;
  else if (fareDisplayInfo->unavailableR3Rule())
    field.strValue() = AUTO_PRICE_NO_UNAVAILABLE_R3;
  else if (fareDisplayInfo->unsupportedPaxType())
    field.strValue() = AUTO_PRICE_NO_UNSUPPORTED_PT;
  else
    field.strValue() = AUTO_PRICE_NOT_SUPPORTED;
}

void
pricingCat(Field& field, PaxTypeFare& paxTypeFare)
{
  if (paxTypeFare.fcaPricingCatType() == PRICE_FIELD_N)
    field.strValue() = PRICE_FIELD_NORMAL;
  else
    field.strValue() = PRICE_FIELD_SPECIAL;
}

void
sameDayChange(Field& field, FareDisplayInfo* fareDisplayInfo)
{
  if (fareDisplayInfo == nullptr)
    return;

  SameDayChangeFilter::formatData(*fareDisplayInfo, field);
}

void
onewayTripAmount(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{
  if ((paxTypeFare.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED) ||
      ((paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED) &&
       (trx.getOptions()->roundTripFare() == 'T')))
    field.strValue() = "  ";
  else
  {
    formatMoneyAmount(field, trx);
  }
}

void
halfRoundTripAmount(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{

  if (paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED)
    field.moneyValue() = (paxTypeFare.nucFareAmount());
  else
  {
    // Get fare amount
    MoneyAmount fareAmount = paxTypeFare.nucFareAmount();
    // Halve and truncate fare amount
    CurrencyUtil::halveNUCAmount(fareAmount);
    // Assign prober value to display field
    field.moneyValue() = fareAmount;
  }
  formatMoneyAmount(field, trx);
}

void
roundTripAmount(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx)
{
  if ((paxTypeFare.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED) || (trx.getOptions()->oneWayFare() == 'T'))
    field.strValue() = "          ";

  else if ((paxTypeFare.owrt() == ONE_WAY_MAY_BE_DOUBLED) &&
           (trx.getOptions()->doubleForRoundTrip() == 'N') &&
           (trx.getOptions()->roundTripFare() != 'T'))
    field.strValue() = "          ";

  else
  {
    if (paxTypeFare.isRoundTrip())
    {

      field.moneyValue() = paxTypeFare.convertedFareAmount();
    }
    else
    {
      field.moneyValue() = paxTypeFare.convertedFareAmountRT();
    }

    formatMoneyAmount(field, trx);
  }
}

void
inOutInd(std::vector<ElementField>& fields,
         Field& field1,
         FareDisplayInfo* fareDisplayInfo,
         Indicator& dateFormat)
{
  if (fareDisplayInfo == nullptr)
    return;

  InOutIndFilter::formatData(*fareDisplayInfo, field1, fields, dateFormat);
}

void
season(std::vector<ElementField>& fields,
       Field& field1,
       FareDisplayInfo* fareDisplayInfo,
       FareDisplayTrx& trx,
       Indicator& dateFormat)
{
  if (fareDisplayInfo == nullptr)
    return;

  SeasonFilter::formatData(*fareDisplayInfo, field1, fields, trx, dateFormat);
}

void
addOnDirection(Field& field, FDAddOnFareInfo& addOnInfo)
{
  const Indicator FROM_INTERIOR = 'O';
  const Indicator TO_INTERIOR = 'D';
  const Indicator BOTH = '=';

  Indicator direction = addOnInfo.directionality();

  if (addOnInfo.vendor() == SITA_VENDOR_CODE)
  {
    if (direction == FROM_INTERIOR)
      field.strValue() = "F";
    else if (direction == TO_INTERIOR)
      field.strValue() = "T";
    else if (direction == BOTH)
      field.strValue() = "B";
  }
  else
  {
    if (direction == FROM)
      field.strValue() = "F";
    else if (direction == TO)
      field.strValue() = "T";
    else if (direction == BETWEEN)
      field.strValue() = "B";
    else if (direction == WITHIN)
      field.strValue() = "W";
    else if (direction == BOTH)
      field.strValue() = "H";
    else if (direction == ORIGIN)
      field.strValue() = "O";
    else
      field.strValue() = "T";
  }
}

void
directionality(Field& field, PaxTypeFare& paxTypeFare)
{
  // @todo did not find the translation in FRD
  Directionality direction = paxTypeFare.directionality();
  if (direction == FROM)
    field.strValue() = "F";
  else if (direction == TO)
    field.strValue() = "T";
  else if (direction == BETWEEN)
    field.strValue() = "B";
  else if (direction == WITHIN)
    field.strValue() = "W";
  else if (direction == BOTH)
    field.strValue() = "H";
  else if (direction == ORIGIN)
    field.strValue() = "O";
  else
    field.strValue() = "T";
}

void
addOnJourneyType(Field& field, FDAddOnFareInfo& addOnInfo)
{

  if (addOnInfo.owrt() == ONE_WAY_MAY_BE_DOUBLED)
    field.strValue() = "X";
  else if (addOnInfo.owrt() == ONE_WAY_MAYNOT_BE_DOUBLED)
    field.strValue() = "O";
  else if (addOnInfo.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
    field.strValue() = "R";
  else if (addOnInfo.vendor() == SITA_VENDOR_CODE &&
           (addOnInfo.owrt() == SITA_OWRT_ADDON_4 || addOnInfo.owrt() == SITA_OWRT_ADDON_5))
    field.strValue() = "X";
}

void
addOnRouting(Field& field, const FDAddOnFareInfo* addOnInfo)
{
  // const FDAddOnFareInfo* addOnInfo = fareDisplayInfo.addOnFareInfoPtr();
  field.strValue() = addOnInfo->addOnRoutingSeq();
}

void
addOnMinMaxStay(Field& field, FareDisplayInfo& fareDisplayInfo)
{
  MinMaxFilter::formatData(fareDisplayInfo, field);
}

void
addOnTravelTicket(Field& field, FareDisplayInfo& fareDisplayInfo)
{
  if (!fareDisplayInfo.isAutoPriceable())
  {
    field.strValue() = NOT_AUTO_PRICEABLE;
  }
  field.strValue() = " -";
}

void
getFailCode(Field& field, PaxTypeFare& paxTypeFare)
{
  const PaxTypeFare::BookingCodeStatus& bookingCodeStatus = paxTypeFare.bookingCodeStatus();

  if (paxTypeFare.actualPaxType() == nullptr)
    field.strValue() = FC_ACTUAL_PAX_TYPE_NULL;
  else if (paxTypeFare.cat35InvalidAgency())
    field.strValue() = FC_CAT_35_INVALID_AGENCY;
  else if (paxTypeFare.cat35IncompleteData())
    field.strValue() = FC_CAT_35_INCOMPLETE_DATA;
  else if (paxTypeFare.cat35ViewNetIndicator())
    field.strValue() = FC_CAT_35_VIEWNETINDICATOR;
  else if (paxTypeFare.cat35FailedRule())
    field.strValue() = FC_CAT_35_FAILED_RULE;
  else if (paxTypeFare.cat25FailedRule())
    field.strValue() = FC_CAT_25_FAILED_RULE;
  else if (paxTypeFare.missingFootnoteData())
    field.strValue() = FC_MISSING_FOOTNOTE_DATA;
  else if (paxTypeFare.fareMarket() == nullptr)
    field.strValue() = FC_FARE_MARKET_NULL;
  else if (paxTypeFare.fare() == Fare::emptyFare())
    field.strValue() = FC_EMPTY_FARE;
  else if (paxTypeFare.fareClassMatch())
    field.strValue() = FC_FAILED_FARE_CLASS_MATCH;
  else if (paxTypeFare.ticketDesignatorMatch())
    field.strValue() = FC_FAILED_TKTDES_MATCH;
  else if (paxTypeFare.bookingCodeMatch())
    field.strValue() = FC_FAILED_BOOKING_CODE_MATCH;
  else if (paxTypeFare.invalidWEB())
    field.strValue() = FC_WEB_INVALID;
  else if (paxTypeFare.notValidForInclusionCode())
    field.strValue() = FC_INCLUSION_CODE_FAIL;
  else if (paxTypeFare.notSelectedForRD())
    field.strValue() = FC_FARE_NOT_SELECTED_FOR_RD;
  else if (paxTypeFare.fareClassAppTagUnavailable())
    field.strValue() = FC_FARECLASS_APP_TAG_UNAVAILABLE;
  else if (paxTypeFare.notValidForDirection())
    field.strValue() = FC_NOT_VALID_FOR_DIRECTION;
  else if (paxTypeFare.paxTypeMatch())
    field.strValue() = FC_FAILED_PAX_TYPE_MATCH;
  else if (paxTypeFare.dateCheck())
    field.strValue() = FC_FAILED_DATE_CHECK;
  else if (paxTypeFare.notValidForOutboundCurrency())
    field.strValue() = FC_OUTBOUND_CURRENCY_FAIL;
  else if (paxTypeFare.privateFareCheck())
    field.strValue() = FC_NOT_PRIVATE_FARE;
  else if (paxTypeFare.publicFareCheck())
    field.strValue() = FC_NOT_PUBLIC_FARE;
  else if (paxTypeFare.industryFareCheck())
    field.strValue() = FC_INDUSTRY_FARE_FILTERED_OUT;
  else if (paxTypeFare.invalidForCWT())
    field.strValue() = FC_INVALID_FOR_CWT;
  else if (paxTypeFare.invalidFareCurrency())
    field.strValue() = FC_INVALID_FARE_CURRENCY;
  else if (!paxTypeFare.areAllCategoryValid())
    field.strValue() = FC_NOT_ALL_CATEGORIES_VALID;
  else if (!(bookingCodeStatus.isSet(PaxTypeFare::BKS_NOT_YET_PROCESSED) ||
             !bookingCodeStatus.isSet(PaxTypeFare::BKS_FAIL)))
  {
    field.strValue() = FC_BOOKING_CODE_STATUS_FAILURE;
  }
  else if (!(!paxTypeFare.isRoutingProcessed() || paxTypeFare.isRoutingValid()))
    field.strValue() = FC_ROUTING_FAILURE;
  else if (paxTypeFare.failedFareGroup())
    field.strValue() = FC_FAILED_FARE_GROUP;
  else if (paxTypeFare.fareClassAppInfo() == FareClassAppInfo::emptyFareClassApp())
    field.strValue() = FC_EMPTY_FARE_CLASS_APP_INFO;
  else if (paxTypeFare.fareClassAppSegInfo() == FareClassAppSegInfo::emptyFareClassAppSeg())
    field.strValue() = FC_EMPTY_FARE_CLASS_APP_SEG_INFO;
  else if (paxTypeFare.baseFareException())
    field.strValue() = FC_BASE_FARE_EXCEPTION;
  else if (paxTypeFare.notOWorRTfare())
    field.strValue() = FC_OWRT_DISPLAY_TYPE;
  else if (!paxTypeFare.fare()->isValid())
    field.strValue() = FC_FARE_IS_INVALID;
  else if (paxTypeFare.matchedFareFocus())
    field.strValue() = FC_MATCHED_FARE_FOCUS_RULE;
  else if (paxTypeFare.notMatchedFareRuleTariff())
    field.strValue() = FC_NOT_MATCHED_FARE_RULE_TARIFF;
  else if (paxTypeFare.notMatchedFareRuleNumber())
    field.strValue() = FC_NOT_MATCHED_FARE_RULE_NUMBER;
  else if (paxTypeFare.notMatchedFareTypeCode())
    field.strValue() = FC_NOT_MATCHED_FARE_TYPE_CODE;
  else if (paxTypeFare.notMatchedFareDisplayType())
    field.strValue() = FC_NOT_MATCHED_FARE_DISPLAY_TYPE;
  else if (paxTypeFare.notMatchedFarePrivateIndicator())
    field.strValue() = FC_NOT_MATCHED_FARE_PRIVATE_INDICATOR;
  else if (paxTypeFare.notMatchedFareCat35Select())
    field.strValue() = FC_NOT_MATCHED_FARE_NEGOTIATED_SELECT;
  else if (paxTypeFare.notMatchedFareCat25Select())
    field.strValue() = FC_NOT_MATCHED_FARE_RULE_SELECT;
  else if (paxTypeFare.notMatchedFareCat15Select())
    field.strValue() = FC_NOT_MATCHED_FARE_SALE_RESTRICT;
  else
    field.strValue() = FC_FARE_VALID;


}

void
fareConstrInd(Field& field)
{
  field.strValue() = ASTERISK;
}

void
nonCOCCurrency(Field& field,
               PaxTypeFare& paxTypeFare,
               CurrencyCode displayCurrency)
{
  // Initialize string
  field.strValue() = EMPTY_STRING();

  if (paxTypeFare.currency() != displayCurrency)
  {
    field.strValue() = ASTERISK;
  }
}

void
netFareNONCOC(Field& field,
              PaxTypeFare& paxTypeFare,
              FareDisplayTrx& trx,
              CurrencyCode displayCurrency)
{
  field.strValue() = EMPTY_STRING();

  if(!TrxUtil::isFqFareRetailerEnabled(trx))
  {
    if (paxTypeFare.fareDisplayCat35Type() == RuleConst::NET_FARE)
      field.strValue() = NET_QUALIFIER;
    else if (paxTypeFare.fareDisplayCat35Type() == RuleConst::REDISTRIBUTED_FARE)
      field.strValue() = REDISTRIBUTE_QUALIFIER;
    else if ((!displayCurrency.empty()) && (paxTypeFare.currency() != displayCurrency))
      field.strValue() = ASTERISK;
    return;
  }

  if (paxTypeFare.getAdjustedSellingCalcData() && !trx.getOptions()->isXRSForFRRule())
    field.strValue() = SELLING_QUALIFIER;
  else if(trx.getOptions()->isPDOForFRRule() && paxTypeFare.isAdjustedSellingBaseFare())
    field.strValue() = ORIGINAL_SELLING_QUALIFIER;
  else
  {
    if (paxTypeFare.fareDisplayCat35Type() == RuleConst::NET_FARE)
      field.strValue() = NET_QUALIFIER;
    else if (paxTypeFare.fareDisplayCat35Type() == RuleConst::REDISTRIBUTED_FARE)
      field.strValue() = REDISTRIBUTE_QUALIFIER;
    else if ((!displayCurrency.empty()) && (paxTypeFare.currency() != displayCurrency))
      field.strValue() = ASTERISK;
  }
}

// --------------------------
// getAmount
// For FL Display
// --------------------------
void
getAmount(Field& field,
          PaxTypeFare& paxTypeFare,
          FareDisplayTrx& trx,
          enum FieldColumnElement columnElement)
{
  const static std::string FL_NA = "  NA  ";

  MoneyAmount owTaxAmount(0);
  FareDisplayTax::getTotalOWTax(trx, paxTypeFare, owTaxAmount);

  Indicator owrt = paxTypeFare.owrt();
  if (columnElement == ONE_WAY_TAX)
  {
    if (owrt == ONE_WAY_MAY_BE_DOUBLED || owrt == ONE_WAY_MAYNOT_BE_DOUBLED)
    {
      field.moneyValue() = owTaxAmount;
      ElementFilter::formatMoneyAmount(field, trx);
      return;
    }
    field.strValue() = FL_NA;
    return;
  }

  if (columnElement == ONE_WAY_TOTAL)
  {
    if (owrt == ONE_WAY_MAY_BE_DOUBLED || owrt == ONE_WAY_MAYNOT_BE_DOUBLED)
    {
      // For FL display the convertedFareAmounts are calculated without the taxes, but includes
      // surcharges
      field.moneyValue() = paxTypeFare.convertedFareAmount() + owTaxAmount;
      ElementFilter::formatMoneyAmount(field, trx);
      return;
    }
    field.strValue() = FL_NA;
    return;
  }

  MoneyAmount rtTaxAmount(0);
  FareDisplayTax::getTotalRTTax(trx, paxTypeFare, rtTaxAmount);

  if (columnElement == RT_TAX)
  {
    if (owrt == ONE_WAY_MAY_BE_DOUBLED || owrt == ROUND_TRIP_MAYNOT_BE_HALVED)
    {
      field.moneyValue() = rtTaxAmount;
      ElementFilter::formatMoneyAmount(field, trx);
      return;
    }
    field.strValue() = FL_NA;
    return;
  }

  if (columnElement == RT_TOTAL)
  {
    if (owrt == ONE_WAY_MAYNOT_BE_DOUBLED)
    {
      field.strValue() = FL_NA;
      return;
    }
    if (owrt == ONE_WAY_MAY_BE_DOUBLED)
      field.moneyValue() = paxTypeFare.convertedFareAmountRT() + rtTaxAmount;
    else
      field.moneyValue() = paxTypeFare.convertedFareAmount() + rtTaxAmount;
    ElementFilter::formatMoneyAmount(field, trx);
  }

  return;
}
}
} // tse
