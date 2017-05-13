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
#include <utility>

namespace tse
{
class FareMarket;
class FarePath;
class FareUsage;
class FPPQItem;
class MergedFareMarket;
class PricingTrx;
class PUPath;
class SpanishReferenceFareInfo;
class TravelSeg;

class YFlexiValidator final
{
  friend class YFlexiValidatorTest;
public:
  YFlexiValidator(PricingTrx& trx, PUPath& puPath, const CurrencyCode& calcCurr);

  MoneyAmount
  getAmount(const FareMarket& fareMarket, const CarrierCode& valCarrier) const;

  bool updDiscAmountBoundary() const;

private:
  bool
  validateVIApoints(const SpanishReferenceFareInfo& srfInfo,
                    const std::vector<TravelSeg*>& travelSeg) const;

  MoneyAmount
  convertToCurrency(const MoneyAmount& amount, const CurrencyCode& currency) const;

  MoneyAmount setMaxAmount(const MergedFareMarket& mergedFareMarket,
                           const CarrierCode& valCarrier) const;

  bool updDiscAmountBoundaryForValCrx(const CarrierCode& valCrx) const;

  bool
  collectIntersectionOfValCrxsPerMergedFMs(std::set<CarrierCode>& sameValCrxListPerMFM) const;

private:
  PricingTrx& _trx;
  PUPath& _puPath;
  const CurrencyCode& _calcCurr;
};
} // tse namespace
