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

#include "Common/SpanishLargeFamilyEnum.h"
#include "Common/TsePrimitiveTypes.h"

#include <string>
#include <vector>

namespace tse
{
class FareMarket;
class FarePath;
class Itin;
class PaxTypeFare;
class PricingOptions;
class PricingUnit;
class PricingTrx;

namespace SLFUtil
{
static constexpr Percent DISCOUNT_LEVEL_1 = 5.0;
static constexpr Percent DISCOUNT_LEVEL_2 = 10.0;

void applySpanishFamilyDisountToFares(PricingTrx&, FareMarket& fm,
                                      Percent percentDiscount);

void checkSpanishDiscountForIS(PricingTrx& trx,
                               FareMarket& fareMarket);

bool isSpanishResidentAndLargeFamilyCombinedDiscountApplies(
    const std::vector<PricingUnit*>& puCol);

Percent getDiscountPercent(const PricingOptions& options);

bool
isSpanishFamilyDiscountApplicable(const PricingTrx& trx);

DiscountLevel getDiscountLevelFromInt(const int16_t value);

bool isSpanishFamilyDiscountApplicable(const PricingTrx& trx);

std::string getIndicator(const PricingTrx& trx,
                         const Itin& itin,
                         const FarePath& farePath);

} // namespace SLFUtil

} // namespace tse
