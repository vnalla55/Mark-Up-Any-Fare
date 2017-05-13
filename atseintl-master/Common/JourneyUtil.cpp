#include "Common/JourneyUtil.h"

#include "Common/ItinUtil.h"
#include "Common/MCPCarrierUtil.h"
#include "DataModel/Billing.h"
#include "DataModel/FareMarket.h"
#include "DataModel/FarePath.h"
#include "DataModel/OAndDMarket.h"
#include "DataModel/PricingOptions.h"

namespace tse
{
std::vector<ClassOfService*>*
JourneyUtil::availability(const TravelSeg* tvlSeg, Itin* itin)
{
  std::vector<ClassOfService*>* cosVec = nullptr;

  SegOAndDMap::const_iterator i = itin->segmentOAndDMarket().find(tvlSeg);

  if (i != itin->segmentOAndDMarket().end())
    cosVec = i->second->getCosVector(tvlSeg);

  return cosVec;
}

void
JourneyUtil::initOAndDCOS(Itin* itin)
{
  if (itin)
  {
    std::vector<OAndDMarket*>::const_iterator odI = itin->oAndDMarkets().begin();
    std::vector<OAndDMarket*>::const_iterator odIE = itin->oAndDMarkets().end();

    for (; odI != odIE; odI++)
      (*odI)->initializeCOS();
  }
}

bool
JourneyUtil::checkIfFmSegInFlowOd(const FareMarket* fm, const SegOAndDMap& oAndDMap)
{
  std::vector<TravelSeg*>::const_iterator tvlI = fm->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlIE = fm->travelSeg().end();

  for (; tvlI != tvlIE; tvlI++)
  {
    SegOAndDMap::const_iterator i = oAndDMap.find(*tvlI);
    if (i != oAndDMap.end() && i->second->validAfterRebook())
      return true;
  }

  return false;
}

bool
JourneyUtil::checkIfSegInFlowOd(const TravelSeg* seg, const SegOAndDMap& oAndDMap)
{
  return (oAndDMap.find(seg) != oAndDMap.end());
}

void
JourneyUtil::addToSegmentOAndDMarketMap(const OAndDMarket* od, SegOAndDMap& oAndDMap)
{
  std::vector<TravelSeg*>::const_iterator tvlI = od->fareMarket()->travelSeg().begin();
  std::vector<TravelSeg*>::const_iterator tvlIE = od->fareMarket()->travelSeg().end();

  for (; tvlI != tvlIE; tvlI++)
  {
    TravelSeg* seg = *tvlI;
    if (LIKELY(od->hasTravelSeg(seg)))
    {
      // Journey after differential may override existing OAndDMarket
      OAndDMarket* existingOd = const_cast<OAndDMarket*>(getOAndDMarketFromSegMap(seg, oAndDMap));
      if (existingOd)
        existingOd->validAfterRebook() = false;

      oAndDMap[seg] = od;
    }
  }
}

void
JourneyUtil::addOAndDMarket(PricingTrx& trx,
                            Itin& itin,
                            FarePath* fpath,
                            FareMarket* fm,
                            const bool isLocalJourneyCarrier,
                            const std::map<const TravelSeg*, bool>* marriedSegs)
{
  const bool isItinAnalyzerPhase = !fpath;
  std::vector<OAndDMarket*>& oAndDs = fpath ? fpath->oAndDMarkets() : itin.oAndDMarkets();
  SegOAndDMap& segMap = fpath ? fpath->segmentOAndDMarket() : itin.segmentOAndDMarket();

  OAndDMarket* od;
  trx.dataHandle().get(od);
  od->initializeOD(*fm,
                   trx.getRequest()->isLowFareRequested(),
                   isLocalJourneyCarrier,
                   isItinAnalyzerPhase,
                   trx.getOptions()->allowLATAMDualRBD(),
                   marriedSegs);

  oAndDs.push_back(od);
  addToSegmentOAndDMarketMap(od, segMap);
}

void
JourneyUtil::addThruFMs(const std::vector<OAndDMarket*>& oAndDs, std::set<FareMarket*>& thruFMs)
{
  std::vector<OAndDMarket*>::const_iterator i = oAndDs.begin();
  std::vector<OAndDMarket*>::const_iterator iE = oAndDs.end();

  for (; i != iE; i++)
    if ((*i)->validAfterRebook())
      thruFMs.insert(const_cast<FareMarket*>((*i)->fareMarket()));
}

void
JourneyUtil::getThruFMs(const Itin* itin, const FarePath* fpath, std::vector<FareMarket*>& thruFMs)
{
  std::set<FareMarket*> uniqFMs;

  addThruFMs(fpath->oAndDMarkets(), uniqFMs);
  addThruFMs(itin->oAndDMarkets(), uniqFMs);

  thruFMs.insert(thruFMs.end(), uniqFMs.begin(), uniqFMs.end());
}

OAndDMarket*
JourneyUtil::getOAndDMarketFromFM(const Itin& itin, const FareMarket* fm)
{
  std::vector<OAndDMarket*>::const_iterator odI = itin.oAndDMarkets().begin();
  std::vector<OAndDMarket*>::const_iterator odIE = itin.oAndDMarkets().end();

  for (; odI != odIE; odI++)
    if ((*odI)->fareMarket() == fm)
      return *odI;

  return nullptr;
}

const OAndDMarket*
JourneyUtil::getOAndDMarketFromSegMap(const TravelSeg* tvlSeg, const SegOAndDMap& oAndDMap)
{
  SegOAndDMap::const_iterator i = oAndDMap.find(tvlSeg);
  if (i != oAndDMap.end())
    return i->second;

  return nullptr;
}

const OAndDMarket*
JourneyUtil::getOAndDMarketFromSegment(const TravelSeg* seg,
                                       const Itin* itin,
                                       const FarePath* fpath)
{
  if (UNLIKELY(fpath))
  {
    const OAndDMarket* od = getOAndDMarketFromSegMap(seg, fpath->segmentOAndDMarket());
    if (od)
      return od;
  }

  return getOAndDMarketFromSegMap(seg, itin->segmentOAndDMarket());
}

const FareMarket*
JourneyUtil::getFlowMarketFromSegment(const TravelSeg* seg, const Itin* itin, const FarePath* fpath)
{
  const OAndDMarket* od = getOAndDMarketFromSegment(seg, itin, fpath);
  if (od)
    return od->fareMarket();

  return nullptr;
}

//--------------------------------------------------------------------------------------
// getMarriedSegments: Returns true if there are all or out-of-sequence married segments
//--------------------------------------------------------------------------------------
bool
JourneyUtil::getMarriedSegments(const PricingTrx& trx,
                                const FareMarket& fm,
                                std::map<const TravelSeg*, bool>& marriedSegments,
                                bool& journeyByMarriage,
                                bool& outOfSeqMarriage)
{
  journeyByMarriage = false;
  outOfSeqMarriage = false;

  if (fm.travelSeg().size() < 2)
    return false;

  if (fm.sideTripTravelSeg().size())
    return false;

  std::vector<TravelSeg*>::const_iterator tvlI = fm.travelSeg().begin();
  AirSeg* startAirSeg = dynamic_cast<AirSeg*>(*tvlI);

  if ((startAirSeg == nullptr) || (startAirSeg->marriageStatus() != AirSeg::MARRIAGE_START))
    return false;

  std::vector<TravelSeg*>::const_iterator tvlB = fm.travelSeg().end() - 1;
  AirSeg* lastAirSeg = dynamic_cast<AirSeg*>(*tvlB);

  if ((lastAirSeg == nullptr) || (lastAirSeg->marriageStatus() != AirSeg::MARRIAGE_END))
    return false;

  bool journeyByMarriageCarriers =
      startAirSeg->journeyByMarriageCarrier() && lastAirSeg->journeyByMarriageCarrier();

  if (!journeyByMarriageCarriers && (startAirSeg->carrier() != lastAirSeg->carrier()))
    return false;

  marriedSegments[*tvlI] = true;
  marriedSegments[*tvlB] = true;
  bool outOfSeqIndicator = false;

  int marriedSegCount = 2;
  for (++tvlI; tvlI != tvlB; tvlI++)
  {
    const AirSeg* airSeg = dynamic_cast<AirSeg*>(*tvlI);
    if (airSeg == nullptr)
      marriedSegments[*tvlI] = false;
    else
    {
      if (++marriedSegCount > MAX_MARRIED_SEGS)
        return false;

      char flag = airSeg->marriageStatus();
      if (flag == AirSeg::MARRIAGE_MIDDLE)
        marriedSegments[*tvlI] = true;
      else
      {
        marriedSegments[*tvlI] = false;
        outOfSeqIndicator = true;
      }
    }
  }

  journeyByMarriage = journeyByMarriageCarriers;
  outOfSeqMarriage = outOfSeqIndicator;

  return true;
}

bool
JourneyUtil::usePricingJourneyLogic(PricingTrx& trx)
{
  // Journey logic is active for WPNC/WPNCB but not WPNCS):
  if (trx.getRequest()->isLowFareRequested())
    return true;

  if (trx.getRequest()->isLowFareNoAvailability())
    return false;

  // for WP, return true only if journey by marriage is effective
  return true;
}

bool
JourneyUtil::useShoppingJourneyLogic(PricingTrx& trx)
{
  if ((trx.billing()->actionCode() == "WPNI.C") && trx.getRequest()->isLowFareRequested())
    return true;

  return false;
}

bool
JourneyUtil::useJourneyLogic(PricingTrx& trx)
{
  if (!journeyActivated(trx))
    return false;

  if (LIKELY(trx.getTrxType() == PricingTrx::MIP_TRX))
    return useShoppingJourneyLogic(trx);

  if (trx.getTrxType() == PricingTrx::PRICING_TRX)
    return usePricingJourneyLogic(trx);

  return false;
}

bool
JourneyUtil::journeyActivated(PricingTrx& trx)
{
  if (((trx.getTrxType() == PricingTrx::MIP_TRX) &&
       trx.getOptions()->journeyActivatedForShopping()) ||
      ((trx.getTrxType() == PricingTrx::PRICING_TRX) &&
       trx.getOptions()->journeyActivatedForPricing()))
    return true;

  return false;
}
}
