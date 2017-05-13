//-------------------------------------------------------------------
//
//  File:        ElementFilter.h
//  Created:     July 24, 2005
//  Authors:     Mike Carroll
//
//  Updates:
//
//  Copyright Sabre 2005
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

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"

#include <vector>

namespace tse
{
class ElementField;
class FareDisplayInfo;
class FareDisplayTrx;
class FDAddOnFareInfo;
class Field;
class PaxTypeFare;

namespace ElementFilter
{
void
vendorCode(Field& field, FareDisplayTrx& trx, bool vendorFWS = false);

void
currencyCode(Field& field, FareDisplayTrx& trx);

void
formatMoneyAmount(Field& field, FareDisplayTrx& trx);
void
formatMoneyAmount(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

void
formatOriginalFare(Field& field, FareDisplayTrx& trx, const CurrencyCode& cur);

void
travelTicket(Field& field, Field& field2, FareDisplayInfo*);

void
advancePurchase(Field& field, FareDisplayInfo*);

void
minMaxStay(Field& field, FareDisplayInfo* fareDisplayInfo);

void
routing(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

void
journeyType(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

void
privateFareIndicator(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

void
passFail(Field& field);

void
paxType(Field& field);

void
fareClassification(Field& field, PaxTypeFare& paxTypeFare);

void
fareByRuleInd(Field& field, const PaxTypeFare& paxTypeFare);

void
ruleTariff(Field& field, const PaxTypeFare& paxTypeFare);

void
fareRule(Field& field, const PaxTypeFare& paxTypeFare);

void
fareType(Field& field, const PaxTypeFare& paxTypeFare);

void
routingTypeNumber(Field& field, const PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

void
redistribution(Field& field, const PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

void
netFareIndicator(Field& field, const PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

void
displayCatType(Field& field, const PaxTypeFare& paxTypeFare);

bool
isAutoPriceNotSupported(const PaxTypeFare& ptf, FareDisplayTrx& trx);
void
autoPrice(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

void
pricingCat(Field& field, PaxTypeFare& paxTypeFare);

void
sameDayChange(Field& field, FareDisplayInfo* fareDisplayInfo);

void
roundTripAmount(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

void
onewayTripAmount(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

void
halfRoundTripAmount(Field& field, PaxTypeFare& paxTypeFare, FareDisplayTrx& trx);

void
inOutInd(std::vector<ElementField>& fields,
         Field& field1,
         FareDisplayInfo* fareDisplayInfo,
         Indicator& dateFormat);

void
season(std::vector<ElementField>& fields,
       Field& field1,
       FareDisplayInfo* fareDisplayInfo,
       FareDisplayTrx& trx,
       Indicator& dateFormat);

void
directionality(Field& field, PaxTypeFare& paxTypeFare);
void
addOnDirection(Field& field, FDAddOnFareInfo& addOnInfo);

void
addOnJourneyType(Field& field, FDAddOnFareInfo& addOnFareInfo);

void
addOnRouting(Field& field, const FDAddOnFareInfo* addOnInfo);

void
addOnMinMaxStay(Field& field, FareDisplayInfo& fareDisplayInfo);

void
addOnTravelTicket(Field& field, FareDisplayInfo& fareDisplayInfo);

void
getFailCode(Field& field, PaxTypeFare& paxTypeFare);

  //--------------------------------------------------------------------------
  // @function ElementFilter::nonCOCCurrency
  //
  // Description: Method to determine if non-COC Currency
  //
  // @param field - field to be filtered
  // @param paxTypeFare - PaxTypeFare to be used
  //--------------------------------------------------------------------------
void
nonCOCCurrency(Field& field,
               PaxTypeFare& paxTypeFare,
               CurrencyCode displayCurrency);

  //--------------------------------------------------------------------------
  // @function ElementFilter::netFareNONCOC
  //
  // Description: Method to determine if either net fare or non-COC Currency
  //              applies.  Used in templates that have one location for both.
  // @param field - field to be filtered
  // @param paxTypeFare - PaxTypeFare to be used
  //--------------------------------------------------------------------------
void
netFareNONCOC(Field& field,
              PaxTypeFare& paxTypeFare,
              FareDisplayTrx& trx,
              CurrencyCode displayCurrency);

void
fareConstrInd(Field& field);

void
getAmount(Field& field,
          PaxTypeFare& paxTypeFare,
          FareDisplayTrx& trx,
          enum FieldColumnElement columnElement);
}
} // namespace tse
