// ----------------------------------------------------------------
//
//   Copyright Sabre 2013
//
//           The copyright to the computer program(s) herein
//           is the property of Sabre.
//           The program(s) may be used and/or copied only with
//           the written permission of Sabre or in accordance
//           with the terms and conditions stipulated in the
//           agreement/contract under which the program(s)
//           have been supplied.
//
// ----------------------------------------------------------------
#include "Pricing/Shopping/FOS/RestrictionFilter.h"

#include "DataModel/FareMarket.h"
#include "DataModel/ShoppingTrx.h"
#include "DataModel/TravelSeg.h"
#include "Pricing/Shopping/FOS/DetailedSop.h"
#include "Pricing/Shopping/FOS/FosCommonUtil.h"
#include "Pricing/Shopping/FOS/FosTaskScope.h"

#include <algorithm>

namespace tse
{
namespace fos
{

std::string
getFMString(const FareMarket* fareMarket)
{
  return fareMarket->origin()->loc() + fareMarket->destination()->loc();
}

RestrictionFilter::RestrictionFilter(const ShoppingTrx& trx, const FosTaskScope& task)
  : _trx(trx), _numTSPerLeg(0), _checkConnectingCities(false)
{
  if (trx.legs().size() == 1) // OW
  {
    size_t numFos = std::max(int32_t(task.getNumFos()), task.getNumCustomFos());
    if (FosCommonUtil::calcNumOfValidSops(_trx) > numFos)
      _numTSPerLeg = 1;
  }
  else if (trx.legs().size() == 2) // RT
  {
    if (task.checkConnectingFlights())
      _numTSPerLeg = 1;
    if (task.checkConnectingCities())
      _checkConnectingCities = true;
  }
}

void
RestrictionFilter::addExistingFareMarkets(const std::vector<FareMarket*>& fareMarkets)
{
  for (const auto fm : fareMarkets)
  {
    _existingFM.insert(getFMString(fm));
  }
}

bool
RestrictionFilter::isApplicableSolution(const SopCombination& sopCombination) const
{
  if (_numTSPerLeg > 0 &&
      !FosCommonUtil::checkNumOfTravelSegsPerLeg(_trx, sopCombination, _numTSPerLeg))
    return false;

  if (_checkConnectingCities && !FosCommonUtil::checkConnectingCitiesRT(_trx, sopCombination))
    return false;

  for (uint32_t legId = 0; legId < sopCombination.size(); ++legId)
    if (!isApplicableSop(legId, sopCombination[legId]))
      return false;
  return true;
}

const SopDetailsPtrVec&
RestrictionFilter::getFilteredSopDetails(const DetailedSop& orginal)
{
  typedef SopDetailsPtrVec::const_iterator SopDetailsCI;
  SopsWithDetailsSet::iterator foundSop = _sopsWithDetailsFilteredSet.find(orginal);
  if (foundSop != _sopsWithDetailsFilteredSet.end())
  {
    return foundSop->getSopDetailsVec();
  }

  uint32_t legId = orginal.getLegId();
  uint32_t sopId = orginal.getSopId();
  DetailedSop* newSop = &_trx.dataHandle().safe_create<DetailedSop>(legId, sopId);

  SopDetailsCI it = orginal.getSopDetailsVec().begin();
  SopDetailsCI itEnd = orginal.getSopDetailsVec().end();

  for (; it != itEnd; ++it)
  {
    SopDetails* sd = *it;
    if (_existingFM.find(sd->fareMarketOD[0]) == _existingFM.end() ||
        _existingFM.find(sd->fareMarketOD[1]) == _existingFM.end())
    {
      newSop->addDetail(sd);
    }
  }
  _sopsWithDetailsFilteredSet.insert(*newSop);

  return newSop->getSopDetailsVec();
}

bool
RestrictionFilter::isApplicableSop(uint32_t legId, int sopId) const
{
  typedef std::vector<TravelSeg*>::const_iterator TravelSegCI;

  const ShoppingTrx::SchedulingOption& sop = _trx.legs()[legId].sop()[sopId];
  const std::vector<TravelSeg*>& travelSegments = sop.itin()->travelSeg();
  if (UNLIKELY(travelSegments.empty()))
    return false;

  const LocCode& first = travelSegments.front()->origAirport();
  const LocCode& last = travelSegments.back()->destAirport();

  TravelSegCI it = travelSegments.begin() + 1;
  TravelSegCI itEnd = travelSegments.end();
  for (; it != itEnd; ++it)
  {
    TravelSeg* ts = *it;
    if (_existingFM.find(first + ts->origAirport()) == _existingFM.end() ||
        _existingFM.find(ts->origAirport() + last) == _existingFM.end())
    {
      return true;
    }
  }

  return false;
}

} // fos
} // tse
