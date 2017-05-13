//-------------------------------------------------------------------
//
//  Copyright Sabre 2016
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#pragma once

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"

#include <vector>

namespace tse
{
class Agent;
class FareMarket;
class FareMarketPath;
class FarePath;
class FareUsage;
class Itin;
class PaxTypeFare;
class PricingOptions;
class PricingUnit;
class PricingTrx;
class PUPath;
class YFlexiValidator;

namespace SRFEUtil
{
static const double SPANISH_DISCOUNT = 0.5;

std::set<CarrierCode>
getGoverningCarriers(const FareMarketPath& fmp);

bool
hasGovCarriersActive(const PricingTrx& trx, const FareMarketPath& fmp);

bool
hasLongConnection(const FareMarketPath& fmp);

bool
hasSpanishGovArunk(const FareMarketPath& fmp);

bool
hasValidResidency(const PricingTrx& trx, const FareMarketPath& fmp, StateCode residencyCity);

bool
isApplicableForPOS(const Agent& agent);

bool
isItinApplicable(const Itin& itin);

bool
isPassengerApplicable(LocCode residencyCity, StateCode& residencyState);

bool
isSolutionPatternApplicable(PricingTrx& trx, const FareMarketPath& fmp);

bool
isDiscountApplied(const FarePath& farePath);

StateCode
mapCityToState(LocCode airportCity);

StateCode
mapCityToStateItin(const LocCode& airportCity);

void
initSpanishResidentFares(PUPath& puPath, PricingTrx& trx, const Itin& itin);

MoneyAmount
calculateDiscount(const PricingTrx& trx,
                  const PricingOptions& pricingOptions,
                  const MoneyAmount referenceAmount,
                  const MoneyAmount fareAmount);

MoneyAmount
calculateDiscount(const PricingTrx& trx,
                  const PricingOptions& pricingOptions,
                  const PUPath& puPath,
                  const FareUsage& fu,
                  const CarrierCode& govCrx,
                  const CarrierCode& valCrx);

MoneyAmount
recalculateDiscount(const PricingTrx& trx,
                    const PricingOptions& pricingOptions,
                    const YFlexiValidator& yFlexiValidator,
                    const FareUsage& fu,
                    const CarrierCode& valCrx);

void
applyDiscountUpperBound(const PricingTrx& trx,
                        const PricingOptions& pricingOptions,
                        FarePath& farePath,
                        const PUPath& puPath);

void
restoreFarePathForSpanishResident(FarePath& farePath);

void
clearSpanishResidentDiscountAmt(FarePath& fp);

void
applySpanishResidentDiscount(PricingTrx& trx, FarePath& fp, const PUPath& puPath, bool isDomestic);

void
applyDomesticSpanishResidentDiscount(PricingTrx& trx, FarePath& fp);

void
addFareBasisSuffix(PaxTypeFare& ptf, const PricingTrx& trx);

bool
isSpanishFamilyDiscountApplicable(const PricingTrx& trx);

void
applySpanishLargeFamilyDiscount(const PricingTrx& trx, FarePath& farePath);

bool
isSpanishResidentDiscountAppliesOld(const std::vector<PricingUnit*>& puCol);
}
}
