//-------------------------------------------------------------------
//  Copyright Sabre 2010
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
#include "FreeBagService/BaggageItinAnalyzer.h"

#include "Common/BaggageTripType.h"
#include "Common/FallbackUtil.h"
#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/RtwUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FarePath.h"
#include "DataModel/TravelSeg.h"
#include "Diagnostic/Diag852Collector.h"
#include "Util/IteratorRange.h"

#ifdef UT_DATA_DUMP
#include "test/testdata/TestTravelSegFactory.h"

#endif

namespace tse
{

FALLBACK_DECL(apo40993UsDotBaggageIfStopoverInUs);

namespace
{
Logger logger("atseintl.FreeBagService.BaggageItinAnalyzer");

inline const Loc*
getCheckedPointLoc(const CheckedPoint& cp)
{
  return (cp.second == CP_AT_ORIGIN) ? (*cp.first)->origin() : (*cp.first)->destination();
}

inline decltype(auto)
getSegAfterCheckedPoint(const CheckedPoint& cp)
{
  return (cp.second == CP_AT_ORIGIN) ? cp.first : (cp.first + 1);
}
} // ns

BaggageItinAnalyzer::BaggageItinAnalyzer(PricingTrx& trx, Itin& itin)
  : _trx(trx),
    _itin(itin),
    _wholeTravelInUS(LocUtil::isWholeTravelInUSTerritory(_itin.travelSeg())),
    _diagMgr(_trx, Diagnostic852),
    _farePath(itin.farePath())
{
  _stopOverLength = getStopOverLength();
  _furthestTicketedPointIter = _itin.travelSeg().end();
  if (_diagMgr.isActive())
    _diag852 = dynamic_cast<Diag852Collector*>(&_diagMgr.collector());
}

void
BaggageItinAnalyzer::analyzeAndSaveIntoFarePaths()
{
  if(_farePath.empty())
    return;
#ifdef UT_DATA_DUMP
  {
    for (TravelSeg* tvlSeg : _itin.travelSeg())
    {
      std::stringstream filename;

      filename << UT_DATA_PATH << "AirSeg_" << tvlSeg->origin()->loc()
               << tvlSeg->destination()->loc();
      TestTravelSegFactory::write(filename.str(), *tvlSeg);
    }
  }
#endif

  analyzeTravels();

  cloneBaggageTravelsForAllFarePaths();
}

void
BaggageItinAnalyzer::analyzeTravels()
{
  determineCheckedPoints();
  determineFurthestCheckedPoint();

  if (LIKELY(TrxUtil::isBaggageCTAMandateActivated(_trx)))
    setBaggageTripType();
  else
    setUsDot();

  determineBaggageTravels();
  determineMss();
  removeDummyBaggageTravels();
}

void
BaggageItinAnalyzer::displayDiagnostic() const
{
  printItinAnalysisResults();
  printBaggageTravels();
  printUsTariffCarriers();
}

const TravelSeg*
BaggageItinAnalyzer::furthestTicketedPoint() const
{
  return (_furthestTicketedPointIter == _itin.travelSeg().end() ? nullptr : *_furthestTicketedPointIter);
}

void
BaggageItinAnalyzer::determineCheckedPoints()
{
  const std::vector<TravelSeg*>& travelSegs = _itin.travelSeg();

  if (travelSegs.size() > 1)
  {
    std::vector<TravelSeg*>::const_iterator tvlSegIter = travelSegs.begin();
    for (; tvlSegIter != travelSegs.end() - 1; ++tvlSegIter)
    {
      std::vector<TravelSeg*>::const_iterator nextTvlSegIter = tvlSegIter + 1;
      const TravelSeg* tvlSeg = *tvlSegIter;
      const TravelSeg* nextTvlSeg = *nextTvlSegIter;

      // Multiairport is a checked point
      if (tvlSeg->destAirport() != nextTvlSeg->origAirport())
      {
        _checkedPoints.emplace_back(tvlSegIter, CP_AT_DESTINATION);
        _checkedPoints.emplace_back(nextTvlSegIter, CP_AT_ORIGIN);
        continue;
      }
      // Surface is a checked point
      if (tvlSeg->isAir() && !nextTvlSeg->isAir())
      {
        _checkedPoints.push_back(std::make_pair(tvlSegIter, CP_AT_DESTINATION));
        continue;
      }
      if (!tvlSeg->isAir() && nextTvlSeg->isAir())
      {
        _checkedPoints.emplace_back(nextTvlSegIter, CP_AT_ORIGIN);
        continue;
      }
      // Stopover is a checked point
      if (nextTvlSeg->isStopOver(
              tvlSeg, _itin.geoTravelType(), TravelSeg::BAGGAGE, _stopOverLength))
      {
        _checkedPoints.emplace_back(tvlSegIter, CP_AT_DESTINATION);
      }
    }
  }
  // Last segment is always checked point
  _checkedPoints.emplace_back(travelSegs.end() - 1, CP_AT_DESTINATION);

  finalizeCheckedPointDetermination();
}

void
BaggageItinAnalyzer::determineCheckedPointsForAirOnly()
{
  _checkedPoints.clear();
  _furthestTicketedPointIter = _itin.travelSeg().end();

  const std::vector<TravelSeg*>& travelSegs = _itin.travelSeg();
  std::vector<TravelSeg*>::const_iterator tvlSegIter = travelSegs.begin();

  while (tvlSegIter != travelSegs.end())
  {
    if ((*tvlSegIter)->isNonAirTransportation())
    {
      ++tvlSegIter;
      continue;
    }

    std::vector<TravelSeg*>::const_iterator nextTvlSegIter = tvlSegIter + 1;
    if (nextTvlSegIter == travelSegs.end())
    {
      _checkedPoints.emplace_back(tvlSegIter, CP_AT_DESTINATION);
      break;
    }

    const TravelSeg* nextTvlSeg = *nextTvlSegIter;
    const bool stopOver = nextTvlSeg->isStopOver(
        *tvlSegIter, _itin.geoTravelType(), TravelSeg::BAGGAGE, _stopOverLength);

    if (((*tvlSegIter)->destAirport() !=
         nextTvlSeg->origAirport()) || // Multiairport is a checked point
        nextTvlSeg->isNonAirTransportation()) // Surface and non air is a checked point
    {
      _checkedPoints.emplace_back(tvlSegIter, CP_AT_DESTINATION);

      while (nextTvlSegIter != travelSegs.end() && (*nextTvlSegIter)->isNonAirTransportation())
        ++nextTvlSegIter;

      if (nextTvlSegIter == travelSegs.end())
        break;

      tvlSegIter = nextTvlSegIter;
      _checkedPoints.emplace_back(nextTvlSegIter, CP_AT_ORIGIN);
      continue;
    }
    // Stopover is a checked point
    if (stopOver)
      _checkedPoints.emplace_back(tvlSegIter, CP_AT_DESTINATION);

    ++tvlSegIter;
  }

  finalizeCheckedPointDetermination();
}

void
BaggageItinAnalyzer::finalizeCheckedPointDetermination()
{
  CheckedPointVector temp(_checkedPoints);

  removeMultiAirportConnectionCheckedPoints();
  // Farthest ticketed point is also checked point in some cases
  if (useFurthestTicketedPoint())
  {
    _checkedPoints = temp;
    if (useFurthestTicketedPoint())
    {
      determineFurthestTicketedPoint();
      insertFurthestTicketedPointAsCheckedPoint();
    }
  }
}

void
BaggageItinAnalyzer::insertFurthestTicketedPointAsCheckedPoint()
{
  const int16_t furthestTpOrder = _itin.segmentOrder(*_furthestTicketedPointIter);
  CheckedPointVector::iterator cpI = _checkedPoints.begin();

  for (; cpI != _checkedPoints.end(); ++cpI)
  {
    const CheckedPoint& cp = *cpI;

    if (_itin.segmentOrder(*cp.first) > furthestTpOrder)
    {
      _checkedPoints.insert(cpI, CheckedPoint(_furthestTicketedPointIter, CP_AT_DESTINATION));
      return;
    }
  }
}

bool
BaggageItinAnalyzer::isWithinMulticity(const CheckedPoint& cp1, const CheckedPoint& cp2) const
{
  const LocCode city1 = (*cp1.first)->destAirport();

  if (!isSameCity(city1, (*cp2.first)->origAirport()))
    return false;

  for (TravelSegPtrVecCI tvlSegIter = cp1.first + 1; tvlSegIter < cp2.first; ++tvlSegIter)
  {
    if (!isSameCity(city1, (*tvlSegIter)->origAirport()))
      return false;

    if (!isSameCity((*tvlSegIter)->origAirport(), (*tvlSegIter)->destAirport()))
      return false;
  }
  return true;
}

void
BaggageItinAnalyzer::removeMultiAirportConnectionCheckedPoints()
{
  if (_checkedPoints.empty())
    return;

  CheckedPointVector::iterator cpIt = _checkedPoints.begin();

  while (cpIt != _checkedPoints.end() - 1)
  {
    CheckedPoint currentCp = *cpIt;
    CheckedPoint nextCp = *(cpIt + 1);

    if (currentCp.second == CP_AT_DESTINATION && nextCp.second == CP_AT_ORIGIN &&
        isWithinMulticity(currentCp, nextCp))
    {
      const TravelSeg* ts1 = *(currentCp.first);
      const TravelSeg* ts2 = *(nextCp.first);

      if (ts1->destAirport() != ts2->origAirport() &&
          !ts2->isStopOver(ts1, _itin.geoTravelType(), TravelSeg::BAGGAGE, _stopOverLength))
      {
        cpIt = _checkedPoints.erase(cpIt, cpIt + 2);
        if (cpIt == _checkedPoints.end())
          return;
        continue;
      }
    }
    ++cpIt;
  }
}

int64_t
BaggageItinAnalyzer::getStopOverLength() const
{
  if (_itin.geoTravelType() == GeoTravelType::Transborder ||
      _itin.tripCharacteristics().isSet(Itin::CanadaOnly) || _wholeTravelInUS)
  {
    return 4 * SECONDS_PER_HOUR; // 4h
  }

  if (isWholeTravelInCAOrPanama(_itin.travelSeg()))
    return 6 * SECONDS_PER_HOUR; // 6h

  return 24 * SECONDS_PER_HOUR; // 24h
}

bool
BaggageItinAnalyzer::isWholeTravelInCAOrPanama(const std::vector<TravelSeg*>& travelSegs) const
{
  if (travelSegs.empty())
    return false;

  for (const TravelSeg* tvlSeg : travelSegs)
    if (!isCAorPanama(tvlSeg->origin()) || !isCAorPanama(tvlSeg->destination()))
      return false;

  return true;
}

bool
BaggageItinAnalyzer::isCAorPanama(const Loc* loc) const
{
  return (loc->subarea() == IATA_SUB_AREA_13() || loc->nation() == NATION_PANAMA);
}

bool
BaggageItinAnalyzer::isDestinationOnlyCheckedPoint() const
{
  if (_checkedPoints.empty())
    return false;

  if (_checkedPoints.size() == 1)
    return true;

  const LocCode& dest = _itin.lastTravelSeg()->destination()->loc();
  return std::all_of(_checkedPoints.begin(), _checkedPoints.end(), [&](const CheckedPoint& cp)
  {
    return getCheckedPointLoc(cp)->loc() == dest;
  });
}

bool
BaggageItinAnalyzer::isSameCity(const LocCode& airport1, const LocCode& airport2) const
{
  if (airport1 == airport2)
    return true;

  LocCode airport1Mtc =
      LocUtil::getMultiTransportCity(airport1, " ", GeoTravelType::International, _itin.travelDate());

  if (LIKELY(!airport1Mtc.empty()))
  {
    LocCode airport2Mtc =
        LocUtil::getMultiTransportCity(airport2, " ", GeoTravelType::International, _itin.travelDate());

    return airport1Mtc == airport2Mtc;
  }
  return false;
}

bool
BaggageItinAnalyzer::useFurthestTicketedPoint() const
{
  if (isDestinationOnlyCheckedPoint() &&
      std::count_if(_itin.travelSeg().begin(),
                    _itin.travelSeg().end(),
                    std::not1(std::mem_fun(&TravelSeg::isNonAirTransportation))) > 1)
  {
    if (_itin.geoTravelType() == GeoTravelType::International || _itin.geoTravelType() == GeoTravelType::Transborder)
      return _itin.firstTravelSeg()->origin()->nation() ==
             _itin.lastTravelSeg()->destination()->nation();

    return isSameCity(_itin.firstTravelSeg()->origin()->loc(),
                      _itin.lastTravelSeg()->destination()->loc());
  }
  return false;
}

bool
BaggageItinAnalyzer::isCheckedPoint(const Loc& loc) const
{
  return std::any_of(_checkedPoints.begin(), _checkedPoints.end(), [&](const CheckedPoint& cp)
  {
    return *getCheckedPointLoc(cp) == loc;
  });
}

void
BaggageItinAnalyzer::determineFurthestTicketedPoint()
{
  TravelSegPtrVecCI segI;
  const std::vector<TravelSeg*>& travelSeg = _itin.travelSeg();
  AirSeg* startSeg, *currSeg;
  uint32_t highestMiles = 0;

  startSeg = static_cast<AirSeg*>(travelSeg.front());
  _furthestTicketedPointIter = segI =
      std::find_if(travelSeg.begin(),
                   travelSeg.end(),
                   std::not1(std::mem_fun(&TravelSeg::isNonAirTransportation)));

  for (; segI != travelSeg.end(); ++segI)
  {
    if ((*segI)->isNonAirTransportation())
      continue;

    currSeg = static_cast<AirSeg*>((*segI));

    if (isCheckedPoint(*(currSeg->destination())))
      continue;

    const GlobalDirection gd = determineGlobalDir({startSeg, currSeg});
    const uint32_t currentMiles = getMileage(*startSeg->origin(), *currSeg->destination(), {gd});

    if (currentMiles > highestMiles)
    {
      highestMiles = currentMiles;
      _furthestTicketedPointIter = segI;
    }
  }
}

GlobalDirection
BaggageItinAnalyzer::determineGlobalDir(const std::vector<TravelSeg*>& tsvec) const
{
  RtwUtil::ScopedRtwDisabler rtwDisabler(_trx);
  GlobalDirection gd = XX;
  GlobalDirectionFinderV2Adapter::getGlobalDirection(&_trx, _itin.travelDate(), tsvec, gd);
  return gd;
}

BaggageItinAnalyzer::ApplicableGDs
BaggageItinAnalyzer::determineGlobalDirs(TravelSegPtrVecCI beg,
                                         TravelSegPtrVecCI turnaround,
                                         TravelSegPtrVecCI end) const
{
  ApplicableGDs gdirs;
  const GlobalDirection outGd = determineGlobalDir(std::vector<TravelSeg*>(beg, turnaround));

  if (outGd != XX)
    gdirs.insert(outGd);

  // in case of RT/CT check also the inbound part
  if (turnaround != end && (**beg).boardMultiCity() == (**(end - 1)).offMultiCity())
  {
    const GlobalDirection inGd = determineGlobalDir(std::vector<TravelSeg*>(turnaround, end));

    if (inGd != XX)
      gdirs.insert(inGd);
  }

  return gdirs;
}

uint32_t
BaggageItinAnalyzer::getMileage(const Loc& originLoc,
                                const Loc& destinationLoc,
                                const ApplicableGDs& gdirs) const
{
  const LocCode& originLocCode = originLoc.city().empty() ? originLoc.loc() : originLoc.city();
  const LocCode& destinationLocCode =
      destinationLoc.city().empty() ? destinationLoc.loc() : destinationLoc.city();

  uint32_t miles = getMileage(originLocCode, destinationLocCode, gdirs, TPM);

  if (miles)
    return miles;

  miles = getMileage(originLocCode, destinationLocCode, gdirs, MPM);

  if (miles)
    return TseUtil::getTPMFromMPM(miles);

  return TseUtil::greatCircleMiles(originLoc, destinationLoc);
}

uint32_t
BaggageItinAnalyzer::getMileage(const LocCode& origin,
                                const LocCode& destination,
                                const ApplicableGDs& gdirs,
                                Indicator mileageType) const
{
  const std::vector<Mileage*>& mileages =
      _trx.dataHandle().getMileage(origin, destination, _itin.travelDate(), mileageType);

  printMileage(origin, destination, mileages, gdirs, mileageType);

  bool isMaxMatchingGd = false;
  uint32_t max = 0;

  for (const Mileage* mileage : mileages)
  {
    const bool isCurMatchingGd = gdirs.count(mileage->globaldir());

    if (isMaxMatchingGd == isCurMatchingGd)
    {
      max = std::max(max, mileage->mileage());
      continue;
    }

    // mileage records for applicable GDs have higher priority
    if (isCurMatchingGd)
    {
      max = mileage->mileage();
      isMaxMatchingGd = true;
    }
  }

  return max;
}

void
BaggageItinAnalyzer::printMileage(const LocCode& origin,
                                  const LocCode& destination,
                                  const std::vector<Mileage*>& mil,
                                  const ApplicableGDs& gdirs,
                                  Indicator mileageType) const
{
  if (UNLIKELY(_diag852 && _trx.diagnostic().diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "GI"))
  {
    _diag852->printMileage(origin, destination, mil, gdirs, mileageType);
  }
}

void
BaggageItinAnalyzer::determineFurthestCheckedPoint()
{
  const std::vector<TravelSeg*>& travelSegs = _itin.travelSeg();

  if (travelSegs.size() == 1)
  {
    _furthestCheckedPoint = _checkedPoints.front();
    return;
  }

  const Loc& origin = *travelSegs.front()->origin();
  constexpr uint32_t NO_MILES = std::numeric_limits<uint32_t>::max();
  uint32_t maxMiles = NO_MILES;

  for (const CheckedPoint& checkedPoint : _checkedPoints)
  {
    const Loc& cpLoc = *getCheckedPointLoc(checkedPoint);
    const ApplicableGDs gdirs = determineGlobalDirs(
        travelSegs.begin(), getSegAfterCheckedPoint(checkedPoint), travelSegs.end());
    const uint32_t currentMiles = getMileage(origin, cpLoc, gdirs);

    if (currentMiles > maxMiles || maxMiles == NO_MILES ||
        (currentMiles == maxMiles && *(checkedPoint.first) == _itin.lastTravelSeg()))
    {
      maxMiles = currentMiles;
      _furthestCheckedPoint = checkedPoint;
    }
  }
}

void
BaggageItinAnalyzer::determineBaggageTravelsForNonUsDot()
{
  determineCheckedPointsForAirOnly();
  if (_checkedPoints.empty())
    return;

  determineFurthestCheckedPoint();

  // make a fake checked point at first check-in to unify the processing
  CheckedPoint fakeCheckedPoint(_itin.travelSeg().begin(), CP_AT_ORIGIN);
  // make the first baggage travel starting at journey's origin
  // but skip non air transportation segments
  for (auto tsIt = _itin.travelSeg().begin(); tsIt != _itin.travelSeg().end(); ++tsIt)
  {
    if (!(*tsIt)->isNonAirTransportation())
    {
      fakeCheckedPoint.first = tsIt;
      break;
    }
  }
  makeBaggageTravel(fakeCheckedPoint, _checkedPoints.front());

  CheckedPointVector::const_iterator cpIt = _checkedPoints.begin();
  for (; cpIt != _checkedPoints.end() - 1; ++cpIt)
    makeBaggageTravel(*cpIt, *(cpIt + 1));
}

bool
BaggageItinAnalyzer::removeRetransitPointsFromBaggageTravels()
{
  std::vector<BaggageTravel*> baggageTravels;
  for (BaggageTravel* baggageTravel : _baggageTravels)
  {
    if (retransitPointExist(*baggageTravel))
    {
      redefineBaggageTravel(baggageTravels, *baggageTravel);
    }
    else
      baggageTravels.push_back(baggageTravel);
  }
  if (baggageTravels.size() > _baggageTravels.size())
  {
    _retransitPointsExist = true;
    _baggageTravels.clear();
    _baggageTravels.swap(baggageTravels);
    return true;
  }
  return false;
}

bool
BaggageItinAnalyzer::retransitPointExist(const BaggageTravel& baggageTravel) const
{
  std::set<LocCode> locs;
  locs.insert((*(baggageTravel.getTravelSegBegin()))->boardMultiCity());
  for (TravelSeg* travelSeg :
       makeIteratorRange(baggageTravel.getTravelSegBegin(), baggageTravel.getTravelSegEnd()))
  {
    if (!isSameCity(travelSeg->origAirport(), travelSeg->destAirport()) &&
        !travelSeg->isNonAirTransportation())
    {
      if (!locs.insert(travelSeg->offMultiCity()).second)
        return true;
    }
  }
  return false;
}

void
BaggageItinAnalyzer::redefineBaggageTravel(std::vector<BaggageTravel*>& baggageTravels,
                                           const BaggageTravel& baggageTravel) const
{
  const Loc& origin = *(*baggageTravel.getTravelSegBegin())->origin();
  const GlobalDirection gd = determineGlobalDir(
      std::vector<TravelSeg*>(baggageTravel.getTravelSegBegin(), baggageTravel.getTravelSegEnd()));
  int32_t maxMiles = -1;

  std::vector<TravelSeg*>::const_iterator furthestTsIt = baggageTravel.getTravelSegEnd();
  std::vector<TravelSeg*>::const_iterator tvlSegIter = baggageTravel.getTravelSegBegin();
  for (; tvlSegIter != baggageTravel.getTravelSegEnd() - 1; ++tvlSegIter)
  {
    TravelSeg* travelSeg = *tvlSegIter;
    if (travelSeg->isNonAirTransportation())
      continue;

    const Loc& dest = *travelSeg->destination();
    const int32_t currentMiles = getMileage(origin, dest, {gd});

    if (currentMiles > maxMiles)
    {
      maxMiles = currentMiles;
      furthestTsIt = tvlSegIter;
    }
  }

  if (furthestTsIt == baggageTravel.getTravelSegEnd())
    return;

  if ((*furthestTsIt)->isNonAirTransportation())
    return;
  BaggageTravel* bt1 = createBaggageTravel();
  BaggageTravel* bt2 = createBaggageTravel();

  furthestTsIt++;
  bt1->updateSegmentsRange(baggageTravel.getTravelSegBegin(), furthestTsIt);

  while (furthestTsIt != baggageTravel.getTravelSegEnd() - 1 &&
         (*furthestTsIt)->isNonAirTransportation())
    ++furthestTsIt;

  if ((*furthestTsIt)->isNonAirTransportation())
    return;

  bt2->updateSegmentsRange(furthestTsIt, baggageTravel.getTravelSegEnd());

  baggageTravels.push_back(bt1);
  baggageTravels.push_back(bt2);
}

void
BaggageItinAnalyzer::makeBaggageTravel(const CheckedPoint& current, const CheckedPoint& next)
{
  TravelSegPtrVecCI travelSegBeginIt;
  if (current.second == CP_AT_ORIGIN)
  {
    travelSegBeginIt = current.first;
  }
  else
  {
    travelSegBeginIt = current.first + 1;
    while (travelSegBeginIt != _itin.travelSeg().end() &&
           (*travelSegBeginIt)->isNonAirTransportation())
      ++travelSegBeginIt;
  }

  if (travelSegBeginIt == _itin.travelSeg().end() || !(*travelSegBeginIt)->isAir() ||
      (current.second == CP_AT_DESTINATION && next.second == CP_AT_ORIGIN &&
       travelSegBeginIt == next.first))
    return;

  BaggageTravel* baggageTravel = createBaggageTravel();

  if (!baggageTravel)
  {
    _baggageTravels.clear();
    return;
  }

  baggageTravel->updateSegmentsRange(travelSegBeginIt, getSegAfterCheckedPoint(next));
  _baggageTravels.push_back(baggageTravel);
}

inline static bool
IsAirTsPredicate(const TravelSeg* ts)
{
  return ts->isAir();
}

void
BaggageItinAnalyzer::addBaggageTravelForUsDot(TravelSegPtrVecCI segBegin, TravelSegPtrVecCI segEnd)
{
  // trim arunks at the beggining
  segBegin = std::find_if(segBegin, segEnd, IsAirTsPredicate);

  // trim arunk at the end
  while (segBegin != segEnd && !IsAirTsPredicate(*(segEnd - 1)))
    --segEnd;

  if (segBegin == segEnd)
    return;

  BaggageTravel* baggageTravel = createBaggageTravel();
  if (baggageTravel)
  {
    baggageTravel->updateSegmentsRange(segBegin, segEnd);
    _baggageTravels.push_back(baggageTravel);
  }
}

void
BaggageItinAnalyzer::determineBaggageTravelsForUsDot()
{
  if (_checkedPoints.empty())
    return;

  const auto inboundBtBegin = getSegAfterCheckedPoint(_furthestCheckedPoint);
  addBaggageTravelForUsDot(_itin.travelSeg().begin(), inboundBtBegin);
  addBaggageTravelForUsDot(inboundBtBegin, _itin.travelSeg().end());
}

BaggageTravel*
BaggageItinAnalyzer::createBaggageTravel() const
{
  BaggageTravel* baggageTravel = nullptr;
  _trx.dataHandle().get(baggageTravel);
  if (LIKELY(baggageTravel))
  {
    baggageTravel->_trx = &_trx;
    if (!_farePath.empty())
      baggageTravel->setupTravelData(*_farePath.front());
    else
    {
      baggageTravel->setItin(_itin);
      TSE_ASSERT(!_trx.paxType().empty());
      baggageTravel->setPaxType(*_trx.paxType().front());
    }
    baggageTravel->_stopOverLength = _stopOverLength;
  }
  else
    LOG4CXX_ERROR(logger, "Unable to create BaggageTravel object");
  return baggageTravel;
}

namespace
{
struct BaggageTravelWithFakeTsOnly
{
  bool operator()(const BaggageTravel* bt) const
  {
    if (bt->getNumTravelSegs() != 1u)
      return false;

    const AirSeg* as = (*bt->getTravelSegBegin())->toAirSeg();
    return as && as->isFake();
  }
};
}

void
BaggageItinAnalyzer::removeDummyBaggageTravels()
{
  if (LIKELY(_trx.getTrxType() != PricingTrx::MIP_TRX || !_trx.getRequest()->originBasedRTPricing()))
    return;

  _baggageTravels.erase(
      std::remove_if(_baggageTravels.begin(), _baggageTravels.end(), BaggageTravelWithFakeTsOnly()),
      _baggageTravels.end());
}

void
BaggageItinAnalyzer::cloneBaggageTravelsForAllFarePaths()
{
  for (BaggageTravel* baggageTravel : _baggageTravels)
    addToFarePath(baggageTravel);

  uint32_t farePathIndex = 0;

  _farePathIndex2baggageTravels[farePathIndex++] = _baggageTravels;

  if (_farePath.size() > 1)
  {
    // we need to ensure that no reallocation will occur so that the original
    // _baggageTravels.end() iterator remains valid
    _baggageTravels.reserve(_baggageTravels.size() * _farePath.size());
    const std::vector<BaggageTravel*>::const_iterator btOrigIterBegin = _baggageTravels.begin();
    const std::vector<BaggageTravel*>::const_iterator btOrigIterEnd = _baggageTravels.end();

    for (FarePath* farePath :
         makeIteratorRange(_farePath.begin() + 1, _farePath.end()))
    {
      std::vector<BaggageTravel*>& singleFarePathBagTravels =
          _farePathIndex2baggageTravels[farePathIndex++];

      for (const BaggageTravel* originalBaggageTravel :
           makeIteratorRange(btOrigIterBegin, btOrigIterEnd))
      {
        BaggageTravel* baggageTravel = createBaggageTravel();
        if (baggageTravel)
        {
          baggageTravel->clone(*originalBaggageTravel, farePath);
          addToFarePath(baggageTravel);
          _baggageTravels.push_back(baggageTravel);

          singleFarePathBagTravels.push_back(baggageTravel);
        }
        else
          return;
      }
    }
  }
}

void
BaggageItinAnalyzer::determineBaggageTravels()
{
  if (_itin.getBaggageTripType().isUsDot())
    determineBaggageTravelsForUsDot();
  else
  {
    determineBaggageTravelsForNonUsDot();
    while (removeRetransitPointsFromBaggageTravels())
      ;
  }
}

void
BaggageItinAnalyzer::setUsDot()
{
  const Loc* furthestPointLoc = getCheckedPointLoc(_furthestCheckedPoint);

  if (TrxUtil::isBaggage302NewUSDotMethodActivated(_trx))
  {
    if (_wholeTravelInUS)
    {
      _itin.setBaggageTripType(BaggageTripType::WHOLLY_WITHIN_US);
      return;
    }

    const bool toFromUS = LocUtil::isBaggageUSTerritory(*(_itin.firstTravelSeg()->origin())) ||
                          LocUtil::isBaggageUSTerritory(*(_itin.lastTravelSeg()->destination())) ||
                          LocUtil::isBaggageUSTerritory(*furthestPointLoc);

    _itin.setBaggageTripType(toFromUS ? BaggageTripType::TO_FROM_US : BaggageTripType::OTHER);
  }
  else
  {
    const bool toFromUS =
        !_wholeTravelInUS &&
        (LocUtil::isBaggageUSTerritory(*(_itin.firstTravelSeg()->origin())) ||
         LocUtil::isBaggageUSTerritory(*(_itin.lastTravelSeg()->destination())) ||
         LocUtil::isBaggageUSTerritory(*furthestPointLoc));

    _itin.setBaggageTripType(toFromUS ? BaggageTripType::TO_FROM_US : BaggageTripType::OTHER);
  }
}

void
BaggageItinAnalyzer::setBaggageTripType()
{
  if (_wholeTravelInUS)
  {
    _itin.setBaggageTripType(BaggageTripType::WHOLLY_WITHIN_US);
    return;
  }

  if (_itin.tripCharacteristics().isSet(Itin::CanadaOnly))
  {
    _itin.setBaggageTripType(BaggageTripType::WHOLLY_WITHIN_CA);
    return;
  }

  bool toFromUS = false;
  bool toFromCA = false;

  const auto locToCheck
      = { _itin.firstTravelSeg()->origin(),
          getCheckedPointLoc(_furthestCheckedPoint),
          _itin.lastTravelSeg()->destination() };

  for (const Loc* loc : locToCheck)
  {
    toFromUS = toFromUS || LocUtil::isBaggageUSTerritory(*loc);
    toFromCA = toFromCA || loc->nation() == CANADA;
  }

  if (!fallback::apo40993UsDotBaggageIfStopoverInUs(&_trx))
  {
    toFromUS = toFromUS || isStopoverWithCondition(LocUtil::isBaggageUSTerritory);
    toFromCA = toFromCA ||
        isStopoverWithCondition([](const Loc& loc) { return loc.nation() == NATION_CA; });
  }

  if (toFromUS && toFromCA)
    _itin.setBaggageTripType(BaggageTripType::BETWEEN_US_CA);
  else if (toFromUS)
    _itin.setBaggageTripType(BaggageTripType::TO_FROM_US);
  else if (toFromCA)
    _itin.setBaggageTripType(BaggageTripType::TO_FROM_CA);
  else
    _itin.setBaggageTripType(BaggageTripType::OTHER);
}

bool
BaggageItinAnalyzer::isStopoverWithCondition(bool (*condition) (const Loc& loc)) const
{
  const std::vector<TravelSeg*>& travelSegs = _itin.travelSeg();

  if (travelSegs.size() > 1)
  {
    auto tvlSegIter = travelSegs.cbegin();
    auto tvlSegEnd = travelSegs.cend() - 1;
    for (; tvlSegIter != tvlSegEnd; ++tvlSegIter)
    {
      const TravelSeg* tvlSeg = *tvlSegIter;
      const TravelSeg* nextTvlSeg = *(tvlSegIter + 1);

      if (condition(*tvlSeg->destination()) &&
          nextTvlSeg->isStopOver(tvlSeg, _itin.geoTravelType(),
                                 TravelSeg::BAGGAGE, _stopOverLength))
      {
        return true;
      }
    }
  }

  return false;
}

void
BaggageItinAnalyzer::determineMss()
{
  if (_itin.getBaggageTripType().isUsDot())
    determineMssForUsDot();
  else
    determineMssForNonUsDot();
}

void
BaggageItinAnalyzer::determineMssForUsDot()
{
  // basic algorithm is the same as for non US DOT
  determineMssForNonUsDot();

  const bool whollyWithinUsOrCa = _itin.getBaggageTripType().isWhollyWithinUsOrCa();

  if (_baggageTravels.size() == 2)
  {
    // for US DOT we also need to find MSS of the entire journey
    BaggageTravel* outboundBT = _baggageTravels.front();
    BaggageTravel* inboundBT = _baggageTravels.back();
    TravelSegPtrVecCI mssJourney =
        whollyWithinUsOrCa
            ? outboundBT->getTravelSegBegin()
            : determineMss(outboundBT->getTravelSegBegin(), inboundBT->getTravelSegEnd());
    outboundBT->_MSSJourney = mssJourney;
    inboundBT->_MSSJourney = mssJourney;
  }
  else if (whollyWithinUsOrCa)
  {
    BaggageTravel* outboundBT = _baggageTravels.front();
    outboundBT->_MSSJourney = outboundBT->getTravelSegBegin();
  }
}

void
BaggageItinAnalyzer::determineMssForNonUsDot()
{
  for (BaggageTravel* baggageTravel : _baggageTravels)
  {
    baggageTravel->_MSS =
        determineMss(baggageTravel->getTravelSegBegin(), baggageTravel->getTravelSegEnd());
    // MSS for journey does not make sense for non US DOT,
    // we just set the iterator to point to some valid location
    baggageTravel->_MSSJourney = baggageTravel->_MSS;
  }
}

TravelSegPtrVecCI
BaggageItinAnalyzer::determineMss(TravelSegPtrVecCI tvlSegIter,
                                  TravelSegPtrVecCI tvlSegEndIter) const
{
  bool areaChanged = false;
  bool subAreaChanged = false;
  bool nationChanged = false;
  TravelSegPtrVecCI mss = tvlSegIter;
  for (; tvlSegIter != tvlSegEndIter; ++tvlSegIter)
  {
    if ((*tvlSegIter)->isAir())
    {
      const Loc* origin = (*tvlSegIter)->origin();
      const Loc* destination = (*tvlSegIter)->destination();
      if (origin->area() != destination->area())
      {
        if (!areaChanged)
        {
          mss = tvlSegIter;
          if (origin->area() != IATA_AREA3 || destination->area() != IATA_AREA2)
            break;
          areaChanged = true;
        }
        else
        {
          if (origin->area() == IATA_AREA2 && destination->area() == IATA_AREA1)
            mss = tvlSegIter;
          break;
        }
      }
      else if (!areaChanged && !subAreaChanged && origin->subarea() != destination->subarea())
      {
        subAreaChanged = true;
        mss = tvlSegIter;
      }
      else if (!areaChanged && !subAreaChanged && !nationChanged &&
               origin->nation() != destination->nation())
      {
        nationChanged = true;
        mss = tvlSegIter;
      }
    }
  }
  return mss;
}

void
BaggageItinAnalyzer::printItinAnalysisResults() const
{
  if (_diag852)
  {
    _diag852->printItinAnalysisResults(
        _trx,
        _checkedPoints,
        _furthestCheckedPoint,
        (_furthestTicketedPointIter == _itin.travelSeg().end() ? nullptr : *_furthestTicketedPointIter),
        _itin,
        _retransitPointsExist);
  }
}

void
BaggageItinAnalyzer::printBaggageTravels() const
{
  if (_farePath.empty())
    return;

  if (_diag852)
    _diag852->printBaggageTravels(_farePath.front()->baggageTravels(),
                                  _itin.getBaggageTripType().isUsDot());
}
void
BaggageItinAnalyzer::printUsTariffCarriers() const
{
  if (_itin.getBaggageTripType().isUsDot() && _diag852)
    _diag852->printTariffCarriers(_trx, _itin);
}

void
BaggageItinAnalyzer::addToFarePath(BaggageTravel* baggageTravel) const
{
  baggageTravel->farePath()->baggageTravels().push_back(baggageTravel);
}
} // tse

