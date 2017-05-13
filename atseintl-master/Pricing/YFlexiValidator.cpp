#include "Pricing/YFlexiValidator.h"

#include "DataModel/FarePath.h"
#include "DataModel/FareUsage.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/Loc.h"
#include "DBAccess/SpanishReferenceFareInfo.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/PricingUtil.h"
#include "Pricing/PU.h"
#include "Pricing/PUPath.h"

#include <algorithm>

namespace tse
{
YFlexiValidator::YFlexiValidator(PricingTrx& trx, PUPath& puPath, const CurrencyCode& calcCurr)
  : _trx(trx), _puPath(puPath), _calcCurr(calcCurr)
{
}

bool
YFlexiValidator::validateVIApoints(const SpanishReferenceFareInfo& srfInfo,
                                   const std::vector<TravelSeg*>& travelSeg) const
{
  using namespace tag;
  const LocCode via1 = srfInfo.get<ViaPointOneAirport>();
  const LocCode via2 = srfInfo.get<ViaPointTwoAirport>();

  size_t tvlSegCounter = 1;
  if (!via1.empty())
    ++tvlSegCounter;
  if (!via2.empty())
    ++tvlSegCounter;

  if(travelSeg.size() != tvlSegCounter)
    return false;

  switch(tvlSegCounter)
  {
    case 1:
      return true;
    case 2:
      return travelSeg[0]->destination()->loc() == via1;
    case 3:
      return travelSeg[0]->destination()->loc() == via1
          && travelSeg[1]->destination()->loc() == via2;
    default:
      return false;
  }
}

MoneyAmount
YFlexiValidator::getAmount(const FareMarket& fareMarket, const CarrierCode& valCarrier) const
{
  using namespace tag;

  const std::vector<SpanishReferenceFareInfo*>& srfInfos =
      _trx.dataHandle().getSpanishReferenceFare(valCarrier,
                                                fareMarket.governingCarrier(),
                                                fareMarket.origin()->loc(),
                                                fareMarket.destination()->loc(),
                                                fareMarket.travelDate());

  MoneyAmount retAmount = 0.0;

  for (const SpanishReferenceFareInfo* srfInfo : srfInfos)
  {
    if (validateVIApoints(*srfInfo, fareMarket.travelSeg()))
    {
      if (srfInfo->get<SCurrencyCode>() != _calcCurr)
        return PricingUtil::convertCurrency(
            _trx, srfInfo->get<FareAmount>(), _calcCurr, srfInfo->get<SCurrencyCode>());
      return srfInfo->get<FareAmount>();
    }
  }
  return retAmount;
}

MoneyAmount
YFlexiValidator::setMaxAmount(const MergedFareMarket& mergedFareMarket,
                              const CarrierCode& valCarrier) const
{
  MoneyAmount maxAmount = 0.0;

  for(const FareMarket* fm : mergedFareMarket.mergedFareMarket())
  {
    MoneyAmount spanishReferenceAmount = getAmount(*fm, valCarrier);
    maxAmount = std::max(maxAmount, spanishReferenceAmount);
    _puPath.addSpanishResidentAmount(fm->origin()->loc(), fm->destination()->loc(),
                                     fm->governingCarrier(), valCarrier, spanishReferenceAmount);
  }

  for(const FareMarket* fm : mergedFareMarket.mergedFareMarket())
    _puPath.addSpanishResidentAmount(fm->origin()->loc(), fm->destination()->loc(), ANY_CARRIER,
                                     valCarrier, maxAmount);

  return maxAmount;
}

bool
YFlexiValidator::updDiscAmountBoundaryForValCrx(const CarrierCode& valCrx) const
{
  for (const PU* pu : _puPath.puPath())
  {
    uint16_t index = 0;
    for (const MergedFareMarket* mfm : pu->fareMarket())
    {
      if(setMaxAmount(*mfm, valCrx) == 0.0)
        return false;
      ++index;
    }
  }
  return true;
}

bool
YFlexiValidator::collectIntersectionOfValCrxsPerMergedFMs(
    std::set<CarrierCode>& sameValCrxListPerMFM) const
{
  for (const PU* pu : _puPath.puPath())
  {
    for (const MergedFareMarket* mfm : pu->fareMarket())
    {
      std::set<CarrierCode> crxList1;
      std::set<CarrierCode> crxList2;
      crxList2.swap(sameValCrxListPerMFM);

      for (const FareMarket* fareMarket : mfm->mergedFareMarket())
      {
        if(fareMarket->validatingCarriers().empty())
          crxList1.insert(_puPath.itin()->validatingCarrier());
        else
        {
          crxList1.insert(fareMarket->validatingCarriers().begin(),
                          fareMarket->validatingCarriers().end());
        }
      }

      if (_puPath.puPath().front()->fareMarket().front() == mfm)
        crxList1.swap(sameValCrxListPerMFM);
      else
        std::set_intersection(crxList1.begin(),
                              crxList1.end(),
                              crxList2.begin(),
                              crxList2.end(),
                              std::inserter(sameValCrxListPerMFM, sameValCrxListPerMFM.end()));

      if (sameValCrxListPerMFM.empty())
        return false;
    }
  }
  return true;
}

bool
YFlexiValidator::updDiscAmountBoundary() const
{
  SpanishData* spanishData;
  _trx.dataHandle().get(spanishData);

  _puPath.setSpanishData(spanishData);

  if(!collectIntersectionOfValCrxsPerMergedFMs(spanishData->puPathValCrxList))
    return false;

  for (const CarrierCode& valCrx : spanishData->puPathValCrxList)
  {
    if(!updDiscAmountBoundaryForValCrx(valCrx))
      return false;
  }
  return true;
}
} // tse namespace
