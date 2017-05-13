//----------------------------------------------------------------------------
//  Copyright Sabre 2014
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

#include "Common/IntlJourneyUtil.h"

#include "Common/Assert.h"
#include "Common/FallbackUtil.h"
#include "Common/Foreach.h"
#include "Common/ShoppingUtil.h"
#include "Common/TrxUtil.h"
#include "DataModel/AirSeg.h"

#include <tr1/functional>

namespace tse
{

namespace IntlJourneyUtil
{
class IntlJourneyBuilder
{
public:
  IntlJourneyBuilder(const std::vector<TravelSeg*>& segs, const size_t startPos)
    : _inputSegs(segs),
      _startSeg(nullptr),
      _startPos(startPos),
      _forwardAllowed(true),
      _backwardAllowed(true)
  {
    TSE_ASSERT(startPos < segs.size());
    _startSeg = _inputSegs[startPos]->toAirSeg();
    TSE_ASSERT(_startSeg);
  }

  void build()
  {
    typedef std::tr1::function<void(IntlJourneyBuilder*)> ExtendFunc;

    static constexpr bool SAME_CXR = true;
    static constexpr bool DIFF_CXR = false;
    static ExtendFunc extendFunctions[] = {
      &IntlJourneyBuilder::addStartSeg,
      std::tr1::bind(&IntlJourneyBuilder::extend, std::tr1::placeholders::_1, 1, SAME_CXR),
      std::tr1::bind(&IntlJourneyBuilder::extend, std::tr1::placeholders::_1, -1, SAME_CXR),
      std::tr1::bind(&IntlJourneyBuilder::extend, std::tr1::placeholders::_1, 1, DIFF_CXR),
      std::tr1::bind(&IntlJourneyBuilder::extend, std::tr1::placeholders::_1, 2, SAME_CXR),
      std::tr1::bind(&IntlJourneyBuilder::extend, std::tr1::placeholders::_1, -1, DIFF_CXR),
      std::tr1::bind(&IntlJourneyBuilder::extend, std::tr1::placeholders::_1, -2, SAME_CXR),
      std::tr1::bind(&IntlJourneyBuilder::extend, std::tr1::placeholders::_1, 2, DIFF_CXR),
      std::tr1::bind(&IntlJourneyBuilder::extend, std::tr1::placeholders::_1, -2, DIFF_CXR)
    };

    for (ExtendFunc extendFunc : extendFunctions)
    {
      extendFunc(this);
      if (_matchedSegs.size() >= 3)
        break;
    }
  }

  void getSegs(std::vector<TravelSeg*>& out)
  {
    for (TravelSeg* ts : _inputSegs)
    {
      if (std::find(_matchedSegs.begin(), _matchedSegs.end(), ts) != _matchedSegs.end())
        out.push_back(ts);
    }
  }

private:
  void addStartSeg() { _matchedSegs.push_back(_startSeg); }

  void extend(const int32_t extendRelPos, const bool extendIfSameCxr)
  {
    if (!isDirectionAllowed(extendRelPos))
      return;

    const int32_t extendPos = _startPos + extendRelPos;
    if (extendPos < 0 || extendPos >= static_cast<int32_t>(_inputSegs.size()))
      return;

    AirSeg* extendSeg = _inputSegs[extendPos]->toAirSeg();
    if (!extendSeg || extendSeg->legId() != _startSeg->legId())
    {
      isDirectionAllowed(extendRelPos) = false;
      return;
    }

    const bool isSameCxr = extendSeg->marketingCarrierCode() == _startSeg->marketingCarrierCode();
    if (extendIfSameCxr == isSameCxr)
      _matchedSegs.push_back(extendSeg);
  }

  bool& isDirectionAllowed(int32_t relPos)
  {
    return (relPos >= 0) ? _forwardAllowed : _backwardAllowed;
  }

  const std::vector<TravelSeg*>& _inputSegs;
  AirSeg* _startSeg;
  int32_t _startPos;
  bool _forwardAllowed;
  bool _backwardAllowed;
  std::vector<TravelSeg*> _matchedSegs;
};

bool
getInterlineKey(AirSeg* startSeg, const std::vector<TravelSeg*>& inputSegs, std::vector<TravelSeg*>& key)
{
  size_t startPos = std::find(inputSegs.begin(), inputSegs.end(), startSeg) - inputSegs.begin();
  if (startPos == inputSegs.size())
    return false;

  IntlJourneyBuilder jnyBuilder(inputSegs, startPos);
  jnyBuilder.build();
  jnyBuilder.getSegs(key);
  return true;
}

bool
processTsAlreadyInJourney(const Journeys& journeys, TravelSeg* ts, Itin& itin)
{
  // Check whether the segment is already included in the journey previously
  // defined for the same carrier.

  for (const CosAndKey& cosAndKey : journeys)
  {
    JourneyCosList& cosVec = *cosAndKey.first;
    const std::vector<TravelSeg*>& key = cosAndKey.second;
    TSE_ASSERT(cosVec.size() == key.size());

    const std::vector<TravelSeg*>::const_iterator tsInKeyIt = std::find(key.begin(), key.end(), ts);
    if (tsInKeyIt == key.end())
      continue;

    size_t tsInKeyPos = std::distance(key.begin(), tsInKeyIt);
    itin.interlineJourneyInfo()[ts] = &cosVec[tsInKeyPos];
    itin.interlineJourneyMarket()[ts] = std::make_pair(key.front(), key.back());
    return true;
  }
  return false;
}

void
determineRemainingSegs(const CarrierCode& cxr,
                       const Journeys& journeys,
                       const Itin& itin,
                       std::vector<TravelSeg*>& segs)
{
  std::vector<TravelSeg*>::const_iterator tsBegin = itin.travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tsEnd = itin.travelSeg().end();

  // The new journey for the same carrier should not include the segment previously defined.
  for (const CosAndKey& cosAndKey : journeys)
  {
    const std::vector<TravelSeg*>& key = cosAndKey.second;

    REVERSE_FOREACH (TravelSeg* ts, key)
    {
      const AirSeg* as = ts->toAirSeg();
      if (!as || as->marketingCarrierCode() != cxr)
        continue;

      std::vector<TravelSeg*>::const_iterator tsIt = std::find(tsBegin, tsEnd, ts);
      if (tsIt != tsEnd)
        tsBegin = tsIt + 1;
      break;
    }
  }

  segs.assign(tsBegin, tsEnd);
}

void
constructJourneyInfo(PricingTrx& trx, Itin& itin)
{
  const bool cosFixActivated = TrxUtil::isCosExceptionFixActivated(trx);
  std::map<CarrierCode, Journeys> journeyMap;
  std::vector<TravelSeg*> remainingSegs;
  std::vector<TravelSeg*> tsKey;

  uint16_t currSegIdx = 0;

  for (TravelSeg* travelSeg : itin.travelSeg())
  {
    const TravelSeg* nextTvlSeg = nullptr;

    nextTvlSeg = TrxUtil::getInterlineAvailNextTravelSeg(travelSeg, itin.travelSeg());

    if (!nextTvlSeg || !TrxUtil::interlineAvailabilityApply(trx, travelSeg, nextTvlSeg))
        continue;

    AirSeg* airSeg = travelSeg->toAirSeg();
    const CarrierCode& cxr = airSeg->marketingCarrierCode();

    Journeys& journeys = journeyMap[cxr];

    if (processTsAlreadyInJourney(journeys, travelSeg, itin))
      continue;

    determineRemainingSegs(cxr, journeys, itin, remainingSegs);

    if (!getInterlineKey(airSeg, remainingSegs, tsKey))
      continue;

    size_t tsPos = std::distance(tsKey.begin(), std::find(tsKey.begin(), tsKey.end(), travelSeg));
    JourneyCosList* interlineCosVec = cosFixActivated
                                          ? ShoppingUtil::getClassOfServiceNoThrow(trx, tsKey)
                                          : &ShoppingUtil::getClassOfService(trx, tsKey);

    if (interlineCosVec && tsPos < interlineCosVec->size())
    {
      itin.interlineJourneyInfo()[travelSeg] = &interlineCosVec->at(tsPos);
      itin.interlineJourneyMarket()[travelSeg] = std::make_pair(tsKey.front(), tsKey.back());
      journeys.push_back(std::make_pair(interlineCosVec, tsKey));
    }
    currSegIdx++;
  }
}

void
constructJourneyInfo(PricingTrx& trx)
{
  for (Itin* itin : trx.itin())
  {
    constructJourneyInfo(trx, *itin);
  }
}
}
}
