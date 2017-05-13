#include "DataModel/OAndDMarket.h"

#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/TravelSeg.h"

using namespace tse;

void
OAndDMarket::initializeOD(const FareMarket& fm,
                          const bool isLowFareRequested,
                          const bool isLocalJourneyCarrier,
                          const bool isItinAnalyzerPhase,
                          const bool allowLATAMDualRBD,
                          const std::map<const TravelSeg*, bool>* marriedSegs)
{
  _fareMarket = &fm;
  _validAfterRebook = true;

  if (UNLIKELY(marriedSegs))
  {
    _carrierType = CT_marriage;
    _fmTvlSegMayUseCosFromThis = *marriedSegs;
  }
  else
  {
    _carrierType = (isLocalJourneyCarrier ? CT_local : CT_flow);
    std::vector<TravelSeg*>::const_iterator tvlI = _fareMarket->travelSeg().begin();
    std::vector<TravelSeg*>::const_iterator tvlIE = _fareMarket->travelSeg().end();

    for (; tvlI != tvlIE; tvlI++)
      _fmTvlSegMayUseCosFromThis[*tvlI] = true;

    if (isLocalJourneyCarrier && (isLowFareRequested || allowLATAMDualRBD) && isItinAnalyzerPhase)
      _validAfterRebook = false;
  }
}

void
OAndDMarket::initializeCOS()
{
  std::vector<TravelSeg*>::const_iterator tvlI = _fareMarket->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlIE = _fareMarket->travelSeg().end();
  uint16_t i = 0;

  for (; tvlI != tvlIE; tvlI++, i++)
  {
    TravelSeg* tSeg = *tvlI;
    if (_fmTvlSegMayUseCosFromThis[tSeg] && (i < _fareMarket->classOfServiceVec().size()))
      _seg2CosVectorMap[tSeg] = _fareMarket->classOfServiceVec()[i];
  }
}

const bool
OAndDMarket::maySegUseODCos(TravelSeg* tSeg)
{
  return _fmTvlSegMayUseCosFromThis[tSeg];
}

const bool
OAndDMarket::hasTravelSeg(const TravelSeg* tSeg) const
{
  std::map<const TravelSeg*, bool>::const_iterator i = _fmTvlSegMayUseCosFromThis.find(tSeg);
  if (LIKELY(i != _fmTvlSegMayUseCosFromThis.end()))
    return i->second;

  return false;
}

std::vector<ClassOfService*>*
OAndDMarket::getCosVector(const TravelSeg* tSeg) const
{
  std::vector<ClassOfService*>* cosVec = nullptr;
  const SegToCosMap::const_iterator i = _seg2CosVectorMap.find(tSeg);
  if (UNLIKELY(i != _seg2CosVectorMap.end()))
    cosVec = i->second;

  return cosVec;
}
