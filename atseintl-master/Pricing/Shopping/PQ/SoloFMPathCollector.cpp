#include "Pricing/Shopping/PQ/SoloFMPathCollector.h"

#include "Common/Assert.h"
#include "Common/ItinUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TravelSegAnalysis.h"
#include "Common/TseUtil.h"
#include "Common/ValidatingCarrierUpdater.h"
#include "DataModel/FareMarket.h"
#include "DataModel/OwrtFareMarket.h"
#include "DataModel/ShoppingTrx.h"
#include "DBAccess/DataHandle.h"
#include "Pricing/FareMarketMerger.h"
#include "Pricing/FareMarketPath.h"
#include "Pricing/MergedFareMarket.h"
#include "Pricing/Shopping/PQ/ConxRouteCxrPQItem.h"
#include "Pricing/Shopping/PQ/CxrFMCollector.h"

#include <boost/bind.hpp>
#include <boost/iterator/filter_iterator.hpp>

#include <map>
#include <set>

namespace tse
{
namespace shpq
{

namespace
{

struct DummyFMFinder
{
  DummyFMFinder(const MergedFareMarket* mfm) : _dummyMergedFM(mfm) {}
  bool operator()(const MergedFareMarket* mfm)
  {
    if ((mfm->boardMultiCity() == _dummyMergedFM->offMultiCity()) &&
        (mfm->offMultiCity() == _dummyMergedFM->boardMultiCity()))
      return true;
    return false;
  }

private:
  const MergedFareMarket* _dummyMergedFM;
};

// Function object to copy info between two FareMakrets or FareMarket and MergedFareMarket
struct CopyFareMarketInfo
{
  CopyFareMarketInfo(ShoppingTrx& trx) : _trx(trx) {}

  template <class T>
  void copy(FareMarket& sourceFM, T& targetFM) const
  {
    // common part for FareMarket and MergedFareMarket
    targetFM.origin() = sourceFM.origin();
    targetFM.destination() = sourceFM.destination();

    targetFM.boardMultiCity() = sourceFM.boardMultiCity();
    targetFM.offMultiCity() = sourceFM.offMultiCity();

    targetFM.setGlobalDirection(sourceFM.getGlobalDirection());
    targetFM.geoTravelType() = sourceFM.geoTravelType();
    targetFM.travelBoundary() = sourceFM.travelBoundary();

    targetFM.travelSeg() = sourceFM.travelSeg();

    targetFM.sideTripTravelSeg() = sourceFM.sideTripTravelSeg();

    // copy details depend on type of targetFM
    copyDetails(sourceFM, targetFM);
  }

  void copy(FareMarket& sourceFM, tse::MergedFareMarket& targetFM) const
  {
    // common part for FareMarket and MergedFareMarket
    targetFM.origin() = sourceFM.origin();
    targetFM.destination() = sourceFM.destination();

    targetFM.boardMultiCity() = sourceFM.boardMultiCity();
    targetFM.offMultiCity() = sourceFM.offMultiCity();

    targetFM.globalDirection() = sourceFM.getGlobalDirection();
    targetFM.geoTravelType() = sourceFM.geoTravelType();
    targetFM.travelBoundary() = sourceFM.travelBoundary();

    targetFM.travelSeg() = sourceFM.travelSeg();

    targetFM.sideTripTravelSeg() = sourceFM.sideTripTravelSeg();

    // copy details depend on type of targetFM
    copyDetails(sourceFM, targetFM);
  }

private:
  void copyDetails(FareMarket& sourceFM, FareMarket& targetFM) const
  {
    targetFM.governingCarrier() = sourceFM.governingCarrier();
    targetFM.governingCarrierPref() = sourceFM.governingCarrierPref();

    targetFM.fareBasisCode() = sourceFM.fareBasisCode();
    targetFM.fbcUsage() = sourceFM.fbcUsage();
    targetFM.fareCalcFareAmt() = sourceFM.fareCalcFareAmt();

    targetFM.inBoundAseanCurrencies() = sourceFM.inBoundAseanCurrencies();
    targetFM.outBoundAseanCurrencies() = sourceFM.outBoundAseanCurrencies();
    targetFM.travelDate() = sourceFM.travelDate();
  }

  void copyDetails(FareMarket& sourceFM, MergedFareMarket& targetFM) const
  {
    uint32_t a, b;
    const bool res = ShoppingUtil::getFareMarketLegIndices(_trx, &sourceFM, a, b);
    TSE_ASSERT(res);
    targetFM.setSegNums(a + 1, b + 1);
    if (sourceFM.direction() == FMDirection::OUTBOUND)
      targetFM.outboundGovCxr() = sourceFM.governingCarrier();
    else if (sourceFM.direction() == FMDirection::INBOUND)
      targetFM.inboundGovCxr() = sourceFM.governingCarrier();
    if (std::find(targetFM.governingCarrier().begin(),
                  targetFM.governingCarrier().end(),
                  sourceFM.governingCarrier()) == targetFM.governingCarrier().end())
      targetFM.governingCarrier().push_back(sourceFM.governingCarrier());
  }

private:
  ShoppingTrx& _trx;
};

struct PaxTypeBucketCmp
{
  PaxTypeBucketCmp(PaxTypeBucket* cortege = nullptr) : _paxTypeCortege(cortege) {}
  bool operator()(const PaxTypeBucket& cortege)
  {
    return _paxTypeCortege->requestedPaxType() == cortege.requestedPaxType();
  }
  void setSearchedCortege(PaxTypeBucket* cortege) { _paxTypeCortege = cortege; }

private:
  PaxTypeBucket* _paxTypeCortege;
};

void
copyPaxTypeBucketInfo(PaxTypeBucket& fromCortege, PaxTypeBucket& toCortege)
{
  toCortege.requestedPaxType() = fromCortege.requestedPaxType();
  toCortege.paxIndex() = fromCortege.paxIndex();
  toCortege.actualPaxType() = fromCortege.actualPaxType();
  toCortege.inboundCurrency() = fromCortege.inboundCurrency();
  toCortege.outboundCurrency() = fromCortege.outboundCurrency();
  toCortege.fareQuoteOverrideCurrency() = fromCortege.fareQuoteOverrideCurrency();
}

DateTime
findMaxDepartureDT(std::vector<FareMarket*>& fareMarketVec)
{
  DateTime maxDepartureDT(0);
  if (LIKELY(!fareMarketVec.empty()))
  {
    FareMarket& fm = *fareMarketVec.back();
    maxDepartureDT = fm.travelSeg().back()->departureDT();
    ApplicableSOP* appSOPs = fm.getApplicableSOPs();
    if (LIKELY(appSOPs))
    {
      for (const ApplicableSOP::value_type& appSop : *appSOPs)
      {
        for (const SOPUsage& sopUsage : appSop.second)
        {
          if (sopUsage.itin_)
          {
            const TravelSeg& tvlSeg = *sopUsage.itin_->travelSeg()[sopUsage.endSegment_];
            if (maxDepartureDT < tvlSeg.departureDT())
              maxDepartureDT = tvlSeg.departureDT();
          }
        }
      }
    }
  }
  return maxDepartureDT;
}

} // namespace

void
SoloFMPathCollector::addPaxTypeFare(OwrtFareMarketPtr sourceFM, MergedFareMarket* mfm)
{
  typedef std::vector<PaxTypeBucket> PaxTypeBucketVec;
  typedef PaxTypeBucketVec::iterator PaxTypeBucketIterator;

  FareMarket* fromFM = sourceFM->getFareMarket();
  FareMarket* targetFM = nullptr;
  _trx.dataHandle().get(targetFM);

  CopyFareMarketInfo copyFMInfo(_trx);
  copyFMInfo.copy(*fromFM, *mfm);
  copyFMInfo.copy(*fromFM, *targetFM);

  PaxTypeBucketIterator fromCortIt = fromFM->paxTypeCortege().begin();
  PaxTypeBucketIterator fromCortItEnd = fromFM->paxTypeCortege().end();

  uint16_t paxTypeNum = 0;
  PaxTypeBucketCmp cortegeFinder;
  bool tag2FareIndicator = false;
  bool validFares = false;
  for (; fromCortIt != fromCortItEnd; ++fromCortIt, ++paxTypeNum)
  {
    PaxTypeBucket& fromCortege = *fromCortIt;
    cortegeFinder.setSearchedCortege(&fromCortege);

    PaxTypeBucketIterator toCortBeginIt = targetFM->paxTypeCortege().begin();
    PaxTypeBucketIterator toCortItEnd = targetFM->paxTypeCortege().end();

    PaxTypeBucketIterator toCortege = std::find_if(toCortBeginIt, toCortItEnd, cortegeFinder);
    if (toCortege == toCortItEnd)
    {
      PaxTypeBucket newCortege;
      newCortege.paxTypeFare().reserve(fromCortege.paxTypeFare().size());
      toCortege = targetFM->paxTypeCortege().insert(targetFM->paxTypeCortege().end(), newCortege);

      copyPaxTypeBucketInfo(fromCortege, *toCortege);
    }

    FareMarketMerger::ValidPaxTypeFareCopier validPTFCopier(
        _trx, nullptr, paxTypeNum, toCortege->paxTypeFare(), skipper::CarrierBrandPairs());
    for_each(fromCortege.paxTypeFare().begin(),
             fromCortege.paxTypeFare().end(),
             boost::bind(&FareMarketMerger::ValidPaxTypeFareCopier::operator(),
                         boost::ref(validPTFCopier),
                         _1));
    if (validPTFCopier.validFares())
      validFares = true;
    if (validPTFCopier.tag2FareIndicator())
      tag2FareIndicator = true;
  }
  if (validFares)
  {
    mfm->mergedFareMarket().push_back(targetFM);
    if (mfm->geoTravelType() == GeoTravelType::Domestic || mfm->geoTravelType() == GeoTravelType::Transborder)
    {
      if (tag2FareIndicator)
        mfm->tag2FareIndicator() = MergedFareMarket::Tag2FareIndicator::Present;
      else
        mfm->tag2FareIndicator() = MergedFareMarket::Tag2FareIndicator::Absent;
    }
  }
}

SoloFMPathCollector::FMPathItinPair
SoloFMPathCollector::getOrCreateFareMarketPath(const ConxRouteCxrPQItem* const pqItem)
{
  ConxRouteCxrPQItem::OwrtFMVector owrtFMVector(pqItem->getFMVector());

  SoloFMPathKey fmKey;
  for (auto& elem : owrtFMVector)
  {
    fmKey.addFareMarket(elem->getFareMarket());
  }

  FMPathItinPair fmPathItinPair(nullptr, nullptr);

  FMPathItinMap::iterator fmPathIt = _fmPathItinMap.find(fmKey);
  if (fmPathIt == _fmPathItinMap.end())
  {
    fmPathItinPair = buildFareMarketPath(owrtFMVector);
    _fmPathItinMap.insert(FMPathItinMap::value_type(fmKey, fmPathItinPair));
  }
  else
  {
    fmPathItinPair = fmPathIt->second;
  }

  return fmPathItinPair;
}

SoloFMPathCollector::FMPathItinPair
SoloFMPathCollector::buildFareMarketPath(OwrtFMVector& owrtFMVector)
{
  MergedFareMktVector mergedFMVector;

  // Variables to build itin
  std::vector<TravelSeg*> travelSegVec;
  std::vector<FareMarket*> fareMarketVec;

  FMPathItinPair fmPathItinPair(nullptr, nullptr);
  ConxRouteCxrPQItem::OwrtFMVector::iterator it = owrtFMVector.begin(), end = owrtFMVector.end();
  for (; it != end; ++it)
  {
    FareMarket* fm = (*it)->getFareMarket();
    if (LIKELY(fm))
    {
      travelSegVec.insert(travelSegVec.end(), fm->travelSeg().begin(), fm->travelSeg().end());
      fareMarketVec.push_back(fm);
    }

    if (!addOwrtFareMarket(*it, mergedFMVector))
      return fmPathItinPair;
  }

  _trx.dataHandle().get(fmPathItinPair.second);
  Itin* itin = fmPathItinPair.second;
  itin->travelSeg() = travelSegVec;
  itin->fareMarket() = fareMarketVec;
  itin->setMaxDepartureDT(findMaxDepartureDT(fareMarketVec));
  setItin(*itin);

  _trx.dataHandle().get(fmPathItinPair.first);
  FareMarketPath* fmPath = fmPathItinPair.first;
  fmPath->fareMarketPath() = mergedFMVector;

  return fmPathItinPair;
}

bool
SoloFMPathCollector::addOwrtFareMarket(OwrtFareMarketPtr owrtFM,
                                       MergedFareMktVector& mergedFMVector)
{
  MergedFareMarket* mfm = nullptr;
  FareMarket* fm = owrtFM->getFareMarket();
  MergedFMKey mfmKey(fm);
  MergedFMMap::iterator mfmIt = _mergedFMMap.find(mfmKey);

  if (mfmIt == _mergedFMMap.end())
  {
    _trx.dataHandle().get(mfm);
    addPaxTypeFare(owrtFM, mfm);
    _mergedFMMap.insert(MergedFMMap::value_type(mfmKey, mfm));
  }
  else // share existing MergedFM
    mfm = mfmIt->second;

  if (mfm->mergedFareMarket().empty())
    return false;
  mfm->collectRec2Cat10() = true;

  mergedFMVector.push_back(mfm);
  return true;
}

void
SoloFMPathCollector::setItin(Itin& itin)
{
  TravelSegAnalysis tvlSegAnalysis;
  Boundary tvlBoundary = tvlSegAnalysis.selectTravelBoundary(itin.travelSeg());
  ItinUtil::setGeoTravelType(tvlSegAnalysis, tvlBoundary, itin);

  itin.setTravelDate(TseUtil::getTravelDate(itin.travelSeg()));
  itin.bookingDate() = TseUtil::getBookingDate(itin.travelSeg());

  ItinUtil::setFurthestPoint(_trx, &itin);
  ItinUtil::setItinCurrencies(itin, _trx.ticketingDate());

  ValidatingCarrierUpdater validatingCarrier(_trx);
  validatingCarrier.update(itin);

  if ((itin.geoTravelType() == GeoTravelType::International) &&
      (!itin.tripCharacteristics().isSet(Itin::RussiaOnly)))
    itin.useInternationalRounding() = true;
}
}
} // namespace tse::shpq
