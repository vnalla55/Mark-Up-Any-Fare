//----------------------------------------------------------------------------
//  File:           TaxUtility.h
//  Created:        27/07/2009
//  Authors:        Piotr Badarycz
//
//  Description: Utilitarian functions.
//
//  Updates:
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

#ifndef TAX_UTILITY_H
#define TAX_UTILITY_H

#include <map>

#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/CurrencyConversionRequest.h"

namespace tse
{
class AirSeg;
class Loc;
class FarePath;
class FareUsage;
class PricingTrx;
class TaxResponse;
class TaxCodeReg;
class TravelSeg;
class Itin;
class PaxType;
class PaxTypeFare;
class FareMarket;
class BSRCollectionResults;
class NUCCollectionResults;
class Money;

namespace taxUtil
{
enum LocCategory
{
  ALASKA,
  HAWAII,
  US,
  OTHER
};

bool
isYQorYR(const TaxCode& code);
enum LocCategory
checkLocCategory(const Loc& loc);
bool
soldInUS(PricingTrx& trx);
bool
isBufferZone(const Loc& loc);
bool
isStopOver(TravelSeg* current, TravelSeg* previous);
void
findFareBreaks(std::map<uint16_t, FareUsage*>& fareBreaks, const FarePath& farePath);
bool
hasHiddenStopInLoc(const AirSeg* airSeg, const LocCode& loc);
bool
isTransitSeq(const TaxCodeReg& taxCodeReg);
bool
isTaxOnOC(PricingTrx& trx, TaxCodeReg& taxCodeReg);
bool
isUS(const Loc& loc);
bool
isSurfaceSegmentAFactor(const TaxResponse& taxResponse, const uint16_t& startSeg);
bool
isMostDistantUS(PricingTrx& trx, TaxResponse& taxResponse);
bool
isMostDistantUSOld(PricingTrx& trx, TaxResponse& taxResponse);
MoneyAmount
locateAkFactor(PricingTrx& trx, const Loc* zoneLoc, const LocCode& locCode);
MoneyAmount
locateHiFactor(PricingTrx& trx, const LocCode& locCode);
uint32_t
calculateMiles(PricingTrx& trx,
               TaxResponse& taxResponse,
               const Loc& market1,
               const Loc& market2,
               std::vector<TravelSeg*>& tvs);
bool
doUsTaxesApplyOnYQYR(const PricingTrx& trx, const FarePath& farePath);
bool
doesUS2Apply(uint8_t startIndex, uint8_t endIndex, const PricingTrx& trx,
             const TaxResponse& taxResponse, const TaxCodeReg& taxCodeReg);
bool
isAnciliaryItin(const PricingTrx& pricingTrx, const Itin& itin);

const PaxType*
findActualPaxType(PricingTrx& trx, const FarePath* farePath, const uint16_t startIndex);

MoneyAmount
convertCurrency(PricingTrx& trx,
                MoneyAmount moneyAmount,
                const CurrencyCode& paymentCurrency,
                const CurrencyCode& calculationCurrency,
                const CurrencyCode& baseFareCurrency,
                CurrencyConversionRequest::ApplicationType applType,
                bool useInternationalRounding);

const std::vector<PaxTypeFare*>*
locatePaxTypeFare(const FareMarket* fareMarketReTrx,
                  const PaxTypeCode& paxTypeCode);

struct IsFirstIndex
{
  IsFirstIndex(uint16_t firstIndex) : _firstIndex(firstIndex) {}

  bool operator()(const std::pair<uint16_t, uint16_t>& indexes) const
  {
    return indexes.first == _firstIndex;
  }

  uint16_t _firstIndex;
};

void
addConversionDetailsToDiag(PricingTrx& trx,
                           const Money& sourceMoney,
                           const Money& targetMoney,
                           const BSRCollectionResults& resultBsr,
                           const NUCCollectionResults& resultNuc);

bool
isGstTax(const TaxCode& code);

} // namespace taxUtil
} // namespace tse
#endif // TAX_UTILITY_H
