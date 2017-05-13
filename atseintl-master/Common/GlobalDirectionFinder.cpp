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

#include "Common/GlobalDirectionFinder.h"

#include "Common/Global.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "Common/Vendor.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/RexPricingTrx.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/GlobalDirSeg.h"
#include "DBAccess/Loc.h"

#include <iostream>
#include <vector>

namespace tse
{
using std::set;
using std::transform;
using std::inserter;

namespace
{
Logger
logger("atseintl.Fares.GlobalDirectionFinder");
}

const char GlobalDirectionFinder::WITHIN_ONE_AREA = 'W';
const char GlobalDirectionFinder::BETWEEN_TWO_AREAS = 'B';
const char GlobalDirectionFinder::FROM_ONE_AREA = 'F';
const std::string GlobalDirectionFinder::ATLANTIC_PACIFIC = "AP";
const std::string GlobalDirectionFinder::ATLANTIC = "AT";

bool
GlobalDirectionFinder::setGlobalDirectionForRW(const PricingTrx* trx, GlobalDirection& globalDir)
    const
{
  if (trx->itin().front()->tripCharacteristics().isSet(Itin::RW_SFC))
    globalDir = GlobalDirection::RW;

  else if (trx->itin().front()->tripCharacteristics().isSet(Itin::CT_SFC))
    globalDir = GlobalDirection::CT;

  else
    return false;
  // too early call, tripCharacteristics not yet set

  logGlobalDirection(globalDir);
  return true;
}

bool
GlobalDirectionFinder::isRWProcessing(const PricingTrx* trx) const
{
  if (UNLIKELY(!trx))
    return false;

  if (LIKELY(!trx->getOptions() || !trx->getOptions()->isRtw()))
    return false;

  size_t locationCounter = 0;
  for (const Location& location : _locations)
  {
    if (!location.isHidden())
      locationCounter++;
  }

  return locationCounter > 2;
}

void
GlobalDirectionFinder::logGlobalDirection(const GlobalDirection& globalDir) const
{
  if (UNLIKELY(IS_DEBUG_ENABLED(logger)))
  {
    std::string gd;
    globalDirectionToStr(gd, globalDir);

    LOG4CXX_DEBUG(logger, "Final GlobalDirection was [" << gd << "]");
    LOG4CXX_DEBUG(logger, "Leaving GlobalDirectionFinder::getGlobalDirection()");
  }

  LOG4CXX_INFO(logger, "Leaving GlobalDirectionFinder::process() was Successful");
}

//----------------------------------------------------------------------------
// getGlobalDirection()
//----------------------------------------------------------------------------
bool
GlobalDirectionFinder::getGlobalDirection(const PricingTrx* trx,
                                          DateTime travelDate,
                                          const std::set<CarrierCode>& carriers,
                                          GlobalDirection& globalDir) const
{
  LOG4CXX_DEBUG(logger, "Entered GlobalDirectionFinder::getGlobalDirection()");
  if (UNLIKELY(isRWProcessing(trx)))
    return setGlobalDirectionForRW(trx, globalDir);

  globalDir = GlobalDirection::XX;
  if (UNLIKELY(_locations.size() < 2))
  {
    LOG4CXX_ERROR(
        logger,
        "GlobalDirectionFinder - must have at least 2 locations to find a global direction");
    return false;
  }

  DataHandle dataHandle(trx ? trx->ticketingDate() : DateTime::localTime());
  dataHandle.setParentDataHandle(trx ? &trx->dataHandle() : nullptr);
  const GlobalDirSegVector& globalDirectionList(
      dataHandle.getGlobalDirSeg(trx ? trx->adjustedTravelDate(travelDate) : travelDate));

  if (globalDirectionList.empty())
  {
    LOG4CXX_DEBUG(logger, "getGlobalDirSeg returned an empty vector");
    return false;
  }

  if (!findTheBestMatchedGlobalDirectionForJourney(
          trx, globalDir, carriers, globalDirectionList, travelDate))
    LOG4CXX_DEBUG(logger, "The best matched GlobalDirection not found. Universal will be apply");

  logGlobalDirection(globalDir);
  return true;
}

bool
GlobalDirectionFinder::findTheBestMatchedGlobalDirectionForJourney(
    const PricingTrx* trx,
    GlobalDirection& globalDir,
    const std::set<CarrierCode>& carriers,
    const GlobalDirSegVector& globalDirectionList,
    const DateTime& travelDate) const
{
  const Location& origin = _locations.front();
  const Location& dest = _locations.back();
  bool oneArea = withinOneArea();
  bool oneAreaWithStops = withinOneArea(true);
  const bool applyRestrictionsforAT = applyRestrictionsforATglobal();
  const bool applyRestrictionsforAP = applyRestrictionsforAPglobal();

  for (const GlobalDirSeg* globalDirSeg : globalDirectionList)
  {
    if (UNLIKELY(globalDirSeg == nullptr))
      continue;

    const std::string& currentGlobalDir = globalDirSeg->globalDir();
    LOG4CXX_DEBUG(logger, "validating global direction [" << currentGlobalDir << "]");

    if (!(globalDirSeg->saleEffDate() <= travelDate && travelDate <= globalDirSeg->saleDiscDate()))
      continue;

    bool within = oneArea;

    if (globalDirSeg->flightTrackingInd() != 'T')
      within = oneAreaWithStops;

    if (!validateGlobalDir(origin, dest, *globalDirSeg, within))
    {
      continue;
    }

    if (validateAllTrvOnCxrLogic(carriers, *globalDirSeg))
    {
      LOG4CXX_DEBUG(logger, "validateAllTrvOnCxrLogic passed");
      strToGlobalDirection(globalDir, globalDirSeg->globalDir());
      return true;
    }

    if (currentGlobalDir == ATLANTIC_PACIFIC && !applyRestrictionsforAP)
    {
      LOG4CXX_DEBUG(logger, "GlobalDirectionFinder::Invalid AP global");
      continue;
    }

    if (currentGlobalDir == ATLANTIC && !applyRestrictionsforAT)
    {
      LOG4CXX_DEBUG(logger, "GlobalDirectionFinder::Invalid AT global");
      continue;
    }

    if (!validateMustBeViaLoc(*globalDirSeg))
    {
      LOG4CXX_DEBUG(logger, "validateMustBeViaLoc failed");
      continue;
    }

    if (!validateMustNotBeViaLoc(*globalDirSeg))
    {
      LOG4CXX_DEBUG(logger, "validateMustNotBeViaLoc failed");
      continue;
    }

    if (!validateMustBeViaIntermediateLoc(*globalDirSeg))
    {
      LOG4CXX_DEBUG(logger, "validateMustBeViaIntermediateLoc failed");
      continue;
    }

    if (!validateMustNotBeViaIntermediateLoc(*globalDirSeg))
    {
      LOG4CXX_DEBUG(logger, "validateMustNotBeViaIntermediateLoc failed");
      continue;
    }

    strToGlobalDirection(globalDir, globalDirSeg->globalDir());
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
// validateGlobalDir()
//----------------------------------------------------------------------------
bool
GlobalDirectionFinder::validateGlobalDir(const Location& origin,
                                         const Location& dest,
                                         const GlobalDirSeg& globalDirSeg,
                                         const bool withinOneAreaOnly) const
{
  LOG4CXX_DEBUG(logger, "Entered GlobalDirectionFinder::validateGlobalDirList()");

  // Fix within/between the same area()/loc
  if (withinOneAreaOnly && globalDirSeg.directionality() == BETWEEN_TWO_AREAS &&
      globalDirSeg.loc1Type() == globalDirSeg.loc2Type() &&
      globalDirSeg.loc1() == globalDirSeg.loc2() &&
      LocUtil::isInLoc(
          origin.loc(), globalDirSeg.loc1Type(), globalDirSeg.loc1(), Vendor::SABRE, MANUAL))
  {
    return true;
  }

  if (withinOneAreaOnly && (globalDirSeg.directionality() != WITHIN_ONE_AREA))
  {
    return false;
  }

  if (withinOneAreaOnly && globalDirSeg.directionality() == WITHIN_ONE_AREA)
  {
    if (LocUtil::isInLoc(
            origin.loc(), globalDirSeg.loc1Type(), globalDirSeg.loc1(), Vendor::SABRE, MANUAL))
      return true;
    else
      return false;
  }

  if (LocUtil::isInLoc(
          origin.loc(), globalDirSeg.loc1Type(), globalDirSeg.loc1(), Vendor::SABRE, MANUAL) &&
      LocUtil::isInLoc(
          dest.loc(), globalDirSeg.loc2Type(), globalDirSeg.loc2(), Vendor::SABRE, MANUAL))
  {
    return true;
  }

  // Try the 'flipped' version since its a between global dir
  if (globalDirSeg.directionality() == BETWEEN_TWO_AREAS)
  {
    if (LocUtil::isInLoc(
            origin.loc(), globalDirSeg.loc2Type(), globalDirSeg.loc2(), Vendor::SABRE, MANUAL) &&
        LocUtil::isInLoc(
            dest.loc(), globalDirSeg.loc1Type(), globalDirSeg.loc1(), Vendor::SABRE, MANUAL))
    {
      return true;
    }
  }

  LOG4CXX_DEBUG(logger, "Leaving GlobalDirectionFinder::validateGlobalDirList()");
  return false;
}

//----------------------------------------------------------------------------
// validateAllTrvOnCxrLogic()
//
// If allTrvOnCxr filed is populated with any specific carrier
// then, we can use the GlobalDirection when all our AirSeg
// has the same carrier.
//
//----------------------------------------------------------------------------
bool
GlobalDirectionFinder::validateAllTrvOnCxrLogic(const std::set<CarrierCode>& carriers,
                                                const GlobalDirSeg& globalDirSeg) const
{
  const CarrierCode carrier = globalDirSeg.allTvlOnCarrier();
  if (carrier.empty() || carrier == " ")
    return false;

  return carriers.size() == 1 && *carriers.begin() == carrier;
}

//----------------------------------------------------------------------------
// validateMustBeViaLoc()
//
//   -Case 1 : _fltTrackingInd is 'T'
//      - find must be via location in ticketed points vector.
//      - if Match, return true, false otherwise.
//   -Case 2 : _fltTrackingInd is 'B'
//      - Match must be via location against ticketed
//        points and also in the vector of hidden stops.
//        - if either one Matches, return ture
//          false otherwise.
//
//
//----------------------------------------------------------------------------
bool
GlobalDirectionFinder::validateMustBeViaLoc(const GlobalDirSeg& globalDirSeg) const
{
  LOG4CXX_DEBUG(logger, "Entered GlobalDirectionFinder::validateMustBeViaLoc()");

  const LocType& matchLocType = globalDirSeg.mustBeViaLocType();
  const LocCode& matchLoc = globalDirSeg.mustBeViaLoc();

  const bool checkHidden = globalDirSeg.flightTrackingInd() != 'T';
  if (!matchLoc.empty())
  {
    return validateLoc(matchLocType, matchLoc, checkHidden, true);
  }

  LOG4CXX_DEBUG(logger, "Leaving GlobalDirectionFinder::validateMustBeViaLoc()");
  return true;
}

//----------------------------------------------------------------------------
// validateMustNotBeViaLoc()
//   -Case 1 : _fltTrackingInd is 'T'
//            - find must be via loc in ticketed points vector.
//                  - if not Match, return ture, false otherwise.
//  -Case 2 : _fltTrackingInd is 'B'
//            - Match must be via loc against ticketed
//              points and also in the vector of hidden stops.
//            - if both dont Matches, return ture,
//              false otherwise.
//
//
//----------------------------------------------------------------------------
bool
GlobalDirectionFinder::validateMustNotBeViaLoc(const GlobalDirSeg& globalDirSeg) const
{
  LOG4CXX_DEBUG(logger, "Entered GlobalDirectionFinder::validateMustNotBeViaLoc()");

  const LocType& matchLocType = globalDirSeg.mustNotBeViaLocType();
  const LocCode& matchLoc = globalDirSeg.mustNotBeViaLoc();

  const bool checkHidden = globalDirSeg.flightTrackingInd() != 'T';
  if (!matchLoc.empty())
  {
    return validateLoc(matchLocType, matchLoc, checkHidden, false);
  }

  LOG4CXX_DEBUG(logger, "Leaving GlobalDirectionFinder::validateMustNotBeViaLoc()");
  return true;
}

//----------------------------------------------------------------------------
// validateMustBeViaIntermediateLoc()
//
// - Match the Intermeditate Location1 and intermediate
//   location2 for a valid travel segment.
//    - if Match found validate for non-stop service.
//           -If both of the above condition is true
//            then return ture.

//           -False otherwise.
//
//----------------------------------------------------------------------------
bool
GlobalDirectionFinder::validateMustBeViaIntermediateLoc(const GlobalDirSeg& globalDirSeg) const
{
  LOG4CXX_DEBUG(logger, "Entered GlobalDirectionFinder::validateMustBeViaIntermediateLoc()");

  const LocType& matchLocType1 = globalDirSeg.viaInterLoc1Type();
  const LocCode& matchLoc1 = globalDirSeg.viaInterLoc1();

  const LocType& matchLocType2 = globalDirSeg.viaInterLoc2Type();
  const LocCode& matchLoc2 = globalDirSeg.viaInterLoc2();

  if (!matchLoc1.empty() && !matchLoc2.empty())
  {
    return validateLoc(matchLocType1, matchLoc1, matchLocType2, matchLoc2, true);
  }

  LOG4CXX_DEBUG(logger, "Leaving GlobalDirectionFinder::validateMustBeViaIntermediateLoc()");
  return true;
}

//----------------------------------------------------------------------------
// validateMustNotBeViaIntermediateLoc()
//
// - Match the Intermeditate Location1 and intermediate
//   location2 for a valid travel segment.
//    - if Match found validate for non-stop service.
//           -If both of the above condition is true
//            then return false.
//           -True otherwise.
//
//----------------------------------------------------------------------------

bool
GlobalDirectionFinder::validateMustNotBeViaIntermediateLoc(const GlobalDirSeg& globalDirSeg) const
{
  LOG4CXX_DEBUG(logger, "Entered GlobalDirectionFinder::validateMustNotBeViaIntermediateLoc()");

  const LocType& matchLocType1 = globalDirSeg.notViaInterLoc1Type();
  const LocCode& matchLoc1 = globalDirSeg.notViaInterLoc1();

  const LocType& matchLocType2 = globalDirSeg.notViaInterLoc2Type();
  const LocCode& matchLoc2 = globalDirSeg.notViaInterLoc2();

  if (!matchLoc1.empty() && !matchLoc2.empty())
  {
    return validateLoc(matchLocType1, matchLoc1, matchLocType2, matchLoc2, false);
  }
  LOG4CXX_DEBUG(logger, "Leaving GlobalDirectionFinder::validateMustNotBeViaIntermediateLoc()");
  return true;
}

//----------------------------------------------------------------------------
// validateLoc()
//----------------------------------------------------------------------------
bool
GlobalDirectionFinder::validateLoc(const LocType& matchLocType,
                                   const LocCode& matchLoc,
                                   const bool checkHiddenStops,
                                   const bool matchRC) const
{
  for (size_t i = 1; i < _locations.size() - 1; ++i)
  {
    if (!_locations[i].isHidden() || checkHiddenStops)
      if (LocUtil::isInLoc(_locations[i].loc(), matchLocType, matchLoc, Vendor::SABRE, MANUAL))
        return matchRC;
  }

  return !matchRC;
}

//----------------------------------------------------------------------------
// validateLoc()
//----------------------------------------------------------------------------
bool
GlobalDirectionFinder::validateLoc(const LocType& matchLocType1,
                                   const LocCode& matchLoc1,
                                   const LocType& matchLocType2,
                                   const LocCode& matchLoc2,
                                   const bool matchRC) const
{
  LOG4CXX_DEBUG(logger, "Entered GlobalDirectionFinder::validateLoc() for Intermediate Points");
  bool isPrevInLoc1 =
      LocUtil::isInLoc(_locations.front().loc(), matchLocType1, matchLoc1, Vendor::SABRE, MANUAL);
  bool isPrevInLoc2 =
      LocUtil::isInLoc(_locations.front().loc(), matchLocType2, matchLoc2, Vendor::SABRE, MANUAL);
  bool isCurentInLoc1 = false;
  bool isCurentInLoc2 = false;

  for (const Location& location : _locations)
  {
    if (location == _locations.front())
      continue;

    isCurentInLoc1 =
        LocUtil::isInLoc(location.loc(), matchLocType1, matchLoc1, Vendor::SABRE, MANUAL);
    isCurentInLoc2 =
        LocUtil::isInLoc(location.loc(), matchLocType2, matchLoc2, Vendor::SABRE, MANUAL);

    if ((isPrevInLoc1 && isCurentInLoc2) || (isPrevInLoc2 && isCurentInLoc1))
      return matchRC;

    isPrevInLoc1 = isCurentInLoc1;
    isPrevInLoc2 = isCurentInLoc2;
  }

  LOG4CXX_DEBUG(logger, " Leaving GlobalDirectionFinder::validateLoc() for Intermediate Points");
  return !matchRC;
}

//----------------------------------------------------------------------------
// withinOneArea()
//----------------------------------------------------------------------------
bool
GlobalDirectionFinder::withinOneArea(const bool checkHiddenStops) const
{
  if (UNLIKELY(_locations.empty()))
    return true;

  IATAAreaCode area = _locations.front().area();
  for (const Location& location : _locations)
  {
    if (location.isHidden() && !checkHiddenStops)
      continue;

    if (location.area() != area)
      return false;
  }

  return true;
}

namespace
{
class LocationVisitor
{
  typedef GlobalDirectionFinder::Location Location;

public:
  LocationVisitor(const IATAAreaCode& firstArea, const IATAAreaCode& secondArea)
    : _transfersCount(0u),
      _visitedFirstArea(false),
      _visitedSecondArea(false),
      _firstArea(firstArea),
      _secondArea(secondArea)
  {
  }

  void visit(const Location& location)
  {
    _visitedFirstArea = _visitedFirstArea || location.area() == _firstArea;
    _visitedSecondArea = _visitedSecondArea || location.area() == _secondArea;

    if (location.isHidden())
      return;

    _transfersCount += (_visitedFirstArea && _visitedSecondArea);

    _visitedFirstArea = location.area() == _firstArea;
    _visitedSecondArea = location.area() == _secondArea;
  }

  size_t getTransfersCount() const { return _transfersCount; }

private:
  size_t _transfersCount;
  bool _visitedFirstArea;
  bool _visitedSecondArea;
  const IATAAreaCode& _firstArea;
  const IATAAreaCode& _secondArea;
};
}

size_t
GlobalDirectionFinder::countTransfers(const IATAAreaCode& firstArea, const IATAAreaCode& secondArea)
    const
{
  LocationVisitor visitor(firstArea, secondArea);
  for (const Location& location : _locations)
    visitor.visit(location);

  return visitor.getTransfersCount();
}

bool
GlobalDirectionFinder::applyRestrictionsforAPglobal() const
{
  return (countTransfers(IATA_AREA1, IATA_AREA2) == 1) &&
         (countTransfers(IATA_AREA1, IATA_AREA3) == 1);
  // Tobe a valid AP global the change in area() should be exactly 2
  // This is fix for problem log pl#6396 for further clarification
}

bool
GlobalDirectionFinder::applyRestrictionsforATglobal() const
{
  if (traverses3areas())
  {
    const size_t locationsCount(_locations.size());
    const Location* firstArrival = nullptr;
    const Location* lastDeparture = nullptr;
    size_t i = 1;
    while (i < locationsCount && _locations[i].isHidden())
      ++i;
    firstArrival = &_locations[i];

    i = locationsCount - 2;
    while (i > 0 && _locations[i].isHidden())
      --i;
    lastDeparture = &_locations[i];

    if ((_locations[0].area() != IATA_AREA1 || firstArrival->subArea() != IATA_SUB_ARE_33()) &&
        (_locations[locationsCount - 1].area() != IATA_AREA1 ||
         lastDeparture->subArea() != IATA_SUB_ARE_33()))
      return countTransfers(IATA_AREA1, IATA_AREA2) == 1;
  }
  return true;
}

bool
GlobalDirectionFinder::traverses3areas() const
{
  set<IATAAreaCode> uniqueAreas;

  for (const Location& location : _locations)
    uniqueAreas.insert(location.area());
  return uniqueAreas.size() == 3;
}
}
