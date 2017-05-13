//----------------------------------------------------------------------------
//  Copyright Sabre 2003
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

#include "Common/GoverningCarrier.h"

#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/LocPredicates.h"
#include "Common/Logger.h"
#include "Common/RtwUtil.h"
#include "Common/TravelSegAnalysis.h"
#include "Common/TravelSegUtil.h"
#include "Common/TrxUtil.h"
#include "Common/TseConsts.h"
#include "Common/TseCodeTypes.h"
#include "Common/TseStringTypes.h"
#include "Common/TseUtil.h"
#include "DataModel/AirSeg.h"
#include "DataModel/ArunkSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/Itin.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/CarrierPreference.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/Mileage.h"

#include <boost/iterator/filter_iterator.hpp>

#include <algorithm>
#include <iostream>
#include <vector>

namespace tse
{
static Logger
logger("atseintl.Common.GoverningCarrier");

/** @class GvoerningCarrier
 * This class selects the governing carrier for each FareMarket.
 * Each fare market calls a method to select a governing carrier.
 * A FareMarket can have only one governing carrier.
 * Governing Carrier selection logic is varies with the travel
 * boundary of the FareMaket. Three Major variants are
 *   - Travel within one country.
 *   - Travel within one sub area.(e.g. within Europe)
 *   - Travel within one/multiple areas.( e.g IATA area 1)
 * @todo Major Steps in Governing Carrier Selection
 *    -# Identify the travel boundary.
 *    -# Invoke appropriate business logic.
 *    -# Process business logic to select the carrier/carriers.
 *
 */

class SubAreasAreIATA1AndNotEqual
{
public:
  bool operator()(const TravelSeg* tvlSeg) const
  {
    const IATAAreaCode& origArea = tvlSeg->origin()->area();
    const IATAAreaCode& destArea = tvlSeg->destination()->area();
    return (origArea == IATA_AREA1 || destArea == IATA_AREA1) && origArea != destArea;
  }
};

class InternationalSegment
{
public:
  bool operator()(const TravelSeg* tvlSeg) const
  {
    return TravelSegUtil::isNotUSCanadaOrCanadaUS(tvlSeg);
  }
};

class IsAirSegment
{
public:
  bool operator()(const TravelSeg* tvlSeg) const
  {
    return (tvlSeg->segmentType() == Air || tvlSeg->segmentType() == Open);
  }
};

class RtwSegmentFilter
{
public:
  bool operator()(const TravelSeg* tvlSeg) const { return tvlSeg->isAir(); }
};


template<class LocPred>
struct LocToTsPred : public LocPred
{
  bool operator()(const TravelSeg* ts) const
  {
    return LocPred::operator()(ts->origin(), ts->destination());
  }
};

GoverningCarrier::GoverningCarrier()
  : _trx(nullptr),
    _isRtw(false),
    _isIataFareSelectionApplicable(false),
    _itin(nullptr)
{
}

GoverningCarrier::GoverningCarrier(PricingTrx* trx)
  : _trx(trx),
    _isRtw(false),
    _isIataFareSelectionApplicable(trx->isIataFareSelectionApplicable() &&
                                   !TrxUtil::isFareSelectionForSubIata21Only(*trx)),
    _itin(nullptr)
{
  setupRtw(*trx);
}

void
GoverningCarrier::setupRtw(PricingTrx& trx)
{
  if (UNLIKELY(RtwUtil::isRtw(trx) && !trx.itin().empty()))
  {
    _isRtw = true;
    _itin = trx.itin().front();
  }
}

//----------------------------------------------------------------------------
// process()
//----------------------------------------------------------------------------
/** Invokes appropriate method to determine GoverningCarrier.
 * Each FareMarket has at most one TravelBoundary. GoverningCarrier
 * selection process is selected on the basis of this TravelBoundary.
 */
bool
GoverningCarrier::process(FareMarket& fareMarket)
{
  if (UNLIKELY(_isRtw && fareMarket.travelSeg().size() == _itin->travelSeg().size()))
  {
    return processRtw(fareMarket);
  }

  if(_trx && _isIataFareSelectionApplicable)
    return setUpGovCxrBasedOnFirstCrossing(fareMarket);

  LOG4CXX_INFO(logger, "Entered GoverningCarrier::process()");

  SmallBitSet<uint8_t, FMTravelBoundary>& boundary = fareMarket.travelBoundary();

  std::set<CarrierCode> govCxrSet;

  bool rc = false;
  if (boundary.isSet(FMTravelBoundary::TravelWithinUSCA))
  {
    rc = selectWithinUSCA(
        fareMarket.travelSeg(), govCxrSet, fareMarket.direction(), &(fareMarket.primarySector()));
    if (LIKELY(!govCxrSet.empty() && rc))
    {
      std::set<CarrierCode>::iterator itr = govCxrSet.begin();
      fareMarket.governingCarrier() = *itr;
    }
  }
  else if (boundary.isSet(FMTravelBoundary::TravelWithinOneIATA))
  {
    rc = selectWithinSameIATA(
        fareMarket.travelSeg(), govCxrSet, fareMarket.direction(), &(fareMarket.primarySector()));
    if (LIKELY(!govCxrSet.empty() && rc))
    {
      std::set<CarrierCode>::iterator itr = govCxrSet.begin();
      fareMarket.governingCarrier() = *itr;
    }
  }
  else if (boundary.isSet(FMTravelBoundary::TravelWithinTwoIATA))
  {
    rc = selectWithinMultiIATA(
        fareMarket.travelSeg(), govCxrSet, fareMarket.direction(), &(fareMarket.primarySector()));
    if (LIKELY(!govCxrSet.empty() && rc))
    {
      std::set<CarrierCode>::iterator itr = govCxrSet.begin();
      fareMarket.governingCarrier() = *itr;
    }
  }
  else if (boundary.isSet(FMTravelBoundary::TravelWithinAllIATA))
  {
    rc = selectWithinAllIATA(
        fareMarket.travelSeg(), govCxrSet, fareMarket.direction(), &(fareMarket.primarySector()));

    if (LIKELY(!govCxrSet.empty() && rc))
    {
      std::set<CarrierCode>::iterator itr = govCxrSet.begin();
      fareMarket.governingCarrier() = *itr;
    }
  }
  else if (boundary.isSet(FMTravelBoundary::TravelWithinSubIATA11))
  {
    rc = selectWithinSubIATA11(
        fareMarket.travelSeg(), govCxrSet, fareMarket.direction(), &(fareMarket.primarySector()));

    if (LIKELY(!govCxrSet.empty() && rc))
    {
      std::set<CarrierCode>::iterator itr = govCxrSet.begin();
      fareMarket.governingCarrier() = *itr;
    }
  }
  else if (boundary.isSet(FMTravelBoundary::TravelWithinSameSubIATAExcept21And11))
  {
    rc = selectWithinSameSubIATA(
        fareMarket.travelSeg(), govCxrSet, fareMarket.direction(), &(fareMarket.primarySector()));

    if (LIKELY(!govCxrSet.empty() && rc))
    {
      std::set<CarrierCode>::iterator itr = govCxrSet.begin();
      fareMarket.governingCarrier() = *itr;
    }
  }
  else if (boundary.isSet(FMTravelBoundary::TravelWithinSameCountryExceptUSCA))
  {
    rc = true;
  }
  else if (LIKELY(boundary.isSet(FMTravelBoundary::TravelWithinSubIATA21)))
  {
    rc = true;
  }
  else
  {
    LOG4CXX_ERROR(logger, "GoverningCarrier() Unhandled Travel boundary");
  }

  if (LIKELY(rc))
  {
    LOG4CXX_INFO(logger,
                 "GoverningCarrier::process() was successful, GovCxr = ["
                     << fareMarket.governingCarrier() << "]");
  }
  else
  {
    LOG4CXX_ERROR(logger, "GoverningCarrier::process() failed");
  }

  LOG4CXX_INFO(logger, "Leaving GoverningCarrier::process()");
  return rc;
}

//----------------------------------------------------------------------------
// setUpGovCxrBasedOnFirstCrossing()
//----------------------------------------------------------------------------
/**This method instantiated for the IATA FareSelection project to invoke an
 * appropriate method to determine the first Crossing GoverningCarrier, except
 * domestic US/Ca and foreign domestic travels.
 * Each FareMarket has at most one TravelBoundary. GoverningCarrier
 * selection process is selected on the basis of this TravelBoundary.
 * It's called from the ItinAnalyzerServiceWrapper.
 */
bool
GoverningCarrier::setUpGovCxrBasedOnFirstCrossing(FareMarket& fareMarket)
{
  LOG4CXX_INFO(logger, "Entered GoverningCarrier::setUpGovCxrBasedOnFirstCrossing()");

  SmallBitSet<uint8_t, FMTravelBoundary>& boundary = fareMarket.travelBoundary();
  std::vector<TravelSeg*>& travelSegs = fareMarket.travelSeg();

  std::set<CarrierCode> govCxrSet;

  if(selectFirstCrossingGovCxr(fareMarket.travelSeg(),
                               govCxrSet,
                               fareMarket.direction(),
                               fareMarket.primarySector()))
  {
    if (!govCxrSet.empty())
    {
      std::set<CarrierCode>::iterator itr = govCxrSet.begin();
      fareMarket.governingCarrier() = *itr;

      if (boundary.isSet(FMTravelBoundary::TravelWithinSameCountryExceptUSCA))
      {
        fareMarket.primarySector() = travelSegs.front();
      }

      LOG4CXX_INFO(logger,
                 "GoverningCarrier::process() was successful, GovCxr = ["
                     << fareMarket.governingCarrier() << "]");
    }
  }
  else
  {
    LOG4CXX_ERROR(logger, "GoverningCarrier::setUpGovCxrBasedOnFirstCrossing() failed");
  }

  LOG4CXX_INFO(logger, "Leaving GoverningCarrier::setUpGovCxrBasedOnFirstCrossing()");
  return (!fareMarket.governingCarrier().empty());
}

bool
GoverningCarrier::selectFirstCrossingGovCxr(
                       const std::vector<TravelSeg*>& tvlsegs,
                             std::set<CarrierCode>& govCxrSet,
                             FMDirection fmDirection,
                             TravelSeg*& primarySector)
{
  Boundary boundary = TravelSegAnalysis::selectTravelBoundary(tvlsegs);

  if (boundary == Boundary::USCA)
  {
    selectWithinUSCA(tvlsegs, govCxrSet, fmDirection, &primarySector);
  }
  else if (boundary == Boundary::EXCEPT_USCA)
  {
    AirSeg* airSeg = dynamic_cast<AirSeg*>(tvlsegs.front());
    if (airSeg == nullptr || airSeg->carrier().empty())
      govCxrSet.insert(INDUSTRY_CARRIER);
    else
      govCxrSet.insert(airSeg->carrier());
  }
  else if (boundary == Boundary::ONE_IATA)
  {
    setFirstSubAreaCrossing(tvlsegs, govCxrSet, fmDirection, primarySector);
  }
  else if (boundary == Boundary::TWO_IATA || boundary == Boundary::ALL_IATA)
  {
    setFirstAreaCrossing(tvlsegs, govCxrSet, fmDirection, primarySector);
  }
  else if (boundary == Boundary::AREA_11 || boundary == Boundary::AREA_21 ||
           boundary == Boundary::OTHER_SUB_IATA)
  {
    setFirstInternationalCrossing(tvlsegs, govCxrSet, fmDirection, primarySector);
  }
  else
  {
    LOG4CXX_ERROR(logger, "GoverningCarrier() Unhandled Travel boundary");
  }

  return !govCxrSet.empty();
}

bool
GoverningCarrier::setFirstSubAreaCrossing(
                             const std::vector<TravelSeg*>& tvlsegs,
                             std::set<CarrierCode>& govCxrSet,
                             FMDirection fmDirection,
                             TravelSeg*& primarySector)
{
  TravelSeg* travelSeg = nullptr;
  if (fmDirection == FMDirection::INBOUND)
  {
    std::vector<TravelSeg*>::const_reverse_iterator tvlSegItr =
          find_if(tvlsegs.rbegin(), tvlsegs.rend(), LocToTsPred<SubAreasNotEqual>());

    if (tvlSegItr != tvlsegs.rend())
    {
      travelSeg = *tvlSegItr;
      setGoverningCarrier(travelSeg, govCxrSet, &primarySector);
    }
  }
  else
  {
    std::vector<TravelSeg*>::const_iterator tvlSegItr =
          std::find_if(tvlsegs.begin(), tvlsegs.end(), LocToTsPred<SubAreasNotEqual>());
    if (tvlSegItr != tvlsegs.end())
    {
      travelSeg = *tvlSegItr;
      setGoverningCarrier(travelSeg, govCxrSet, &primarySector);
    }
  }
  return true;
}

bool
GoverningCarrier::setFirstAreaCrossing(
                             const std::vector<TravelSeg*>& tvlsegs,
                             std::set<CarrierCode>& govCxrSet,
                             FMDirection fmDirection,
                             TravelSeg*& primarySector)
{
  TravelSeg* travelSeg = nullptr;
  if (fmDirection == FMDirection::INBOUND)
  {
    std::vector<TravelSeg*>::const_reverse_iterator tvlSegItr =
              find_if(tvlsegs.rbegin(), tvlsegs.rend(), LocToTsPred<AreasNotEqual>());

    if (tvlSegItr != tvlsegs.rend())
    {
      travelSeg = *tvlSegItr;
      setGoverningCarrier(travelSeg, govCxrSet, &primarySector);
    }
  }
  else
  {
    std::vector<TravelSeg*>::const_iterator tvlSegItr =
              std::find_if(tvlsegs.begin(), tvlsegs.end(), LocToTsPred<AreasNotEqual>());
    if (tvlSegItr != tvlsegs.end())
    {
      travelSeg = *tvlSegItr;
      setGoverningCarrier(travelSeg, govCxrSet, &primarySector);
    }
  }
  return true;
}

bool
GoverningCarrier::setFirstInternationalCrossing(
                             const std::vector<TravelSeg*>& tvlsegs,
                             std::set<CarrierCode>& govCxrSet,
                             FMDirection fmDirection,
                             TravelSeg*& primarySector)
{
  TravelSeg* travelSeg = nullptr;
  if (fmDirection == FMDirection::INBOUND)
  {
    std::vector<TravelSeg*>::const_reverse_iterator tvlSegItr =
            find_if(tvlsegs.rbegin(), tvlsegs.rend(), InternationalSegment());

    if (tvlSegItr != tvlsegs.rend())
    {
      travelSeg = *tvlSegItr;
      setGoverningCarrier(travelSeg, govCxrSet, &primarySector);
    }
  }
  else
  {
    std::vector<TravelSeg*>::const_iterator tvlSegItr =
            std::find_if(tvlsegs.begin(), tvlsegs.end(), InternationalSegment());
    if (tvlSegItr != tvlsegs.end())
    {
      travelSeg = *tvlSegItr;
      setGoverningCarrier(travelSeg, govCxrSet, &primarySector);
    }
  }
  return true;
}

bool
GoverningCarrier::processRtw(FareMarket& fareMarket)
{
  LOG4CXX_INFO(logger, "Entered GoverningCarrier::processRtw()");

  const TravelBoundarySet& boundary = fareMarket.travelBoundary();

  bool transoceanic = false;
  bool areaCrossing = false;
  if(_trx && !_isIataFareSelectionApplicable)
  {
    transoceanic = boundary.isSet(FMTravelBoundary::TravelWithinAllIATA);
    areaCrossing = transoceanic || boundary.isSet(FMTravelBoundary::TravelWithinTwoIATA);
  }
  else
  {
    areaCrossing = (boundary.isSet(FMTravelBoundary::TravelWithinAllIATA) ||
                    boundary.isSet(FMTravelBoundary::TravelWithinTwoIATA)   );
  }
  bool subareaCrossing = areaCrossing || boundary.isSet(FMTravelBoundary::TravelWithinOneIATA);
  bool international = subareaCrossing || boundary.isSet(FMTravelBoundary::TravelWithinSubIATA11) ||
                       boundary.isSet(FMTravelBoundary::TravelWithinSubIATA21) ||
                       boundary.isSet(FMTravelBoundary::TravelWithinSameSubIATAExcept21And11);

  RtwSteps steps;
  steps.set(TRANSOCEANIC, transoceanic);
  steps.set(AREA_CROSSING, areaCrossing);
  steps.set(SUBAREA_CROSSING, subareaCrossing);
  steps.set(INTERNATIONAL, international);

  std::set<CarrierCode> govCxrSet;
  bool rc = processRtwSteps(fareMarket.travelSeg(), steps, govCxrSet, &fareMarket.primarySector());

  if (rc)
  {
    fareMarket.governingCarrier() = *govCxrSet.begin();
    LOG4CXX_INFO(logger,
                 "GoverningCarrier::processRtw() was successful,"
                 "GovCxr = ["
                     << fareMarket.governingCarrier() << "]");
  }
  else
  {
    LOG4CXX_ERROR(logger, "GoverningCarrier::processRtw() failed");
  }

  LOG4CXX_INFO(logger, "Leaving GoverningCarrier::processRtw()");
  return rc;
}

bool
GoverningCarrier::getGoverningCarrier(const std::vector<TravelSeg*>& segments,
                                      std::set<CarrierCode>& result,
                                      FMDirection direction,
                                      Boundary boundary)
{
  if (UNLIKELY(_isRtw && segments.size() == _itin->travelSeg().size()))
  {
    return getGoverningCarrierRtw(segments, boundary, result);
  }

  LOG4CXX_INFO(logger, " Entered GoverningCarrier::getGoverningCarrier()");

  bool rc = false;
  if (boundary == Boundary::USCA)
  {
    rc = selectWithinUSCA(segments, result, direction);
  }
  else if (boundary == Boundary::EXCEPT_USCA)
  {
    rc = selectWithinCountryExceptUSCA(segments, result);
  }
  else if (boundary == Boundary::ONE_IATA)
  {
    rc = selectWithinSameIATA(segments, result, direction);
  }
  else if (boundary == Boundary::TWO_IATA)
  {
    rc = selectWithinMultiIATA(segments, result, direction);
  }
  else if (boundary == Boundary::ALL_IATA)
  {
    if(_trx && _isIataFareSelectionApplicable)
    {
      rc = selectWithinMultiIATA(segments, result, direction);
    }
    else
    {
      rc = selectWithinAllIATA(segments, result, direction);
    }
  }
  else if (boundary == Boundary::AREA_11)
  {
    if(_trx && _isIataFareSelectionApplicable)
      rc = selectWithinSameSubIATA(segments, result, direction);
    else
      rc = selectWithinSubIATA11(segments, result, direction);
  }
  else if (boundary == Boundary::AREA_21)
  {
    if(_trx && _isIataFareSelectionApplicable)
      rc = selectWithinSameSubIATA(segments, result, direction);
    else
      rc = selectWithinSubIATA21(segments, result, direction);
  }
  else if (LIKELY(boundary == Boundary::OTHER_SUB_IATA))
  {
    rc = selectWithinSameSubIATA(segments, result, direction);
  }
  else
  {
    LOG4CXX_ERROR(logger, " GoverningCarrier::getGoverningCarrier() Failed");
  }

  LOG4CXX_INFO(logger, "Leaving GoverningCarrier::getGoverningCarrier()");
  return rc;
}

bool
GoverningCarrier::getGoverningCarrierRtw(const std::vector<TravelSeg*>& segments,
                                         const Boundary boundary,
                                         std::set<CarrierCode>& govCxrSet)
{
  LOG4CXX_INFO(logger, "Entered GoverningCarrier::getGoverningCarrierRtw()");

  bool allSteps = false;
  RtwSteps steps;
  if(_trx && _isIataFareSelectionApplicable)
  {
    allSteps =
        steps.set(AREA_CROSSING, boundary == Boundary::ALL_IATA || boundary == Boundary::TWO_IATA);
  }
  else
  {
    allSteps = steps.set(TRANSOCEANIC, boundary == Boundary::ALL_IATA);
    allSteps = steps.set(AREA_CROSSING, allSteps || boundary == Boundary::TWO_IATA);
  }
  allSteps = steps.set(SUBAREA_CROSSING, allSteps || boundary == Boundary::ONE_IATA);
  allSteps = steps.set(INTERNATIONAL,
                       allSteps || boundary == Boundary::AREA_11 || boundary == Boundary::AREA_21 ||
                           boundary == Boundary::OTHER_SUB_IATA);

  bool rc = processRtwSteps(segments, steps, govCxrSet);

  if (!rc)
  {
    LOG4CXX_ERROR(logger, "GoverningCarrier::getGoverningCarrierRtw() failed");
  }

  LOG4CXX_INFO(logger, "Leaving GoverningCarrier::getGoverningCarrierRtw()");
  return rc;
}

//---------------------------------------------------------------------------
bool
GoverningCarrier::getGoverningCarrier(const std::vector<TravelSeg*>& tvlSegs,
                                      std::set<CarrierCode>& govCxrSet,
                                      FMDirection fmDirection)
{
  Boundary boundary = TravelSegAnalysis::selectTravelBoundary(tvlSegs);
  return getGoverningCarrier(tvlSegs, govCxrSet, fmDirection, boundary);
}

bool
GoverningCarrier::processRtwSteps(const std::vector<TravelSeg*>& travelSegs,
                                  const RtwSteps& steps,
                                  std::set<CarrierCode>& govCxrSet,
                                  TravelSeg** primarySector)
{
  typedef boost::filter_iterator<RtwSegmentFilter, std::vector<TravelSeg*>::const_iterator> TsIt;

  TsIt tsBegin(RtwSegmentFilter(), travelSegs.begin(), travelSegs.end());
  TsIt tsEnd(RtwSegmentFilter(), travelSegs.end(), travelSegs.end());

  if (steps.isSet(TRANSOCEANIC) &&
      selectFirstSatisfying(
          SubAreasAreIATA1AndNotEqual(), tsBegin, tsEnd, govCxrSet, primarySector))
  {
    return true;
  }

  if (steps.isSet(AREA_CROSSING) &&
      selectFirstSatisfying(LocToTsPred<AreasNotEqual>(), tsBegin, tsEnd, govCxrSet, primarySector))
  {
    return true;
  }

  if (steps.isSet(SUBAREA_CROSSING) &&
      selectFirstSatisfying(
          LocToTsPred<SubAreasNotEqual>(), tsBegin, tsEnd, govCxrSet, primarySector))
  {
    return true;
  }

  if (steps.isSet(INTERNATIONAL) &&
      selectFirstSatisfying(InternationalSegment(), tsBegin, tsEnd, govCxrSet, primarySector))
  {
    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
// selectWithinUSCA()
//----------------------------------------------------------------------------
/** Selects Carrier of the first air segment as the Governing Carrier.*/

bool
GoverningCarrier::selectWithinUSCA(const std::vector<TravelSeg*>& tvlSegs,
                                   std::set<CarrierCode>& govCxrSet,
                                   FMDirection fmDirection,
                                   TravelSeg** primarySector)
{

  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::selectGovCxrWithinUSCA()");
  TravelSeg* travelSeg = nullptr;
  if (fmDirection == FMDirection::INBOUND)
  {
    std::vector<TravelSeg*>::const_reverse_iterator tvlSegItr =
        find_if(tvlSegs.rbegin(), tvlSegs.rend(), IsAirSegment());
    if (LIKELY(tvlSegItr != tvlSegs.rend()))
      travelSeg = *tvlSegItr;
  }
  else
  {
    std::vector<TravelSeg*>::const_iterator tvlSegItr =
        std::find_if(tvlSegs.begin(), tvlSegs.end(), IsAirSegment());
    if (tvlSegItr != tvlSegs.end())
      travelSeg = *tvlSegItr;
  }

  return setGoverningCarrier(travelSeg, govCxrSet, primarySector);
}

bool
GoverningCarrier::getSetOfGoverningCarriers(const std::vector<TravelSeg*>& tvlSegs,
                                            TravelSeg* travelSeg,
                                            std::set<CarrierCode>& govCxrSet,
                                            FMDirection fmDirection,
                                            TravelSeg** primarySector)
{
   setGoverningCarrier(travelSeg, govCxrSet, primarySector);
   CarrierCode govCxr = getHighestTPMCarrier(tvlSegs, fmDirection, *primarySector);

   if (!govCxr.empty())
   {
      govCxrSet.insert(govCxr);
   }
   return !govCxrSet.empty();
}

//----------------------------------------------------------------------------
// selectWithinSameIATA()
//----------------------------------------------------------------------------
/**The carrier providing the first flight that changes sub-areas is the governing carrier.
 * Iterate through the vector of air segments and compare the sub-IATA area of origin
 * and destination of each air segment. If they are not same, carrier of this air segment is
 * the governing carrier.
 */
bool
GoverningCarrier::selectWithinSameIATA(const std::vector<TravelSeg*>& tvlSegs,
                                       std::set<CarrierCode>& govCxrSet,
                                       FMDirection fmDirection,
                                       TravelSeg** primarySector)
{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::selectGovCxrWithinSameIATA()");

  TravelSeg* travelSeg = nullptr;
  if (fmDirection == FMDirection::INBOUND)
  {
    std::vector<TravelSeg*>::const_reverse_iterator tvlSegItr =
        find_if(tvlSegs.rbegin(), tvlSegs.rend(), LocToTsPred<SubAreasNotEqual>());

    if (LIKELY(tvlSegItr != tvlSegs.rend()))
    {
      travelSeg = *tvlSegItr;
      // get next candidate for GovCxr
      if(_trx && _isIataFareSelectionApplicable)
      {
        return getSetOfGoverningCarriers(tvlSegs, travelSeg, govCxrSet, fmDirection, primarySector);
      }
      return setGoverningCarrier(travelSeg, govCxrSet, primarySector);
    }
  }
  else
  {
    std::vector<TravelSeg*>::const_iterator tvlSegItr =
        std::find_if(tvlSegs.begin(), tvlSegs.end(), LocToTsPred<SubAreasNotEqual>());
    if (LIKELY(tvlSegItr != tvlSegs.end()))
    {
      travelSeg = *tvlSegItr;
      if(_trx && _isIataFareSelectionApplicable)
      {
        return getSetOfGoverningCarriers(tvlSegs, travelSeg, govCxrSet, fmDirection, primarySector);
      }
      return setGoverningCarrier(travelSeg, govCxrSet, primarySector);
    }
  }

  LOG4CXX_DEBUG(logger, "Leaving GoverningCarrier::selectGovCxrWithinSameIATA() - FAILED");
  return false;
}

//----------------------------------------------------------------------------
// selectWithinMultiIATA()
//----------------------------------------------------------------------------
/**The carrier providing the first flight that changes areas is the governing carrier.
 * Iterate through the vector of air segments and compare the IATA area of origin
 * and destination of each air segment. If they are not same, carrier of this air segment is
 * the governing carrier.
 */
bool
GoverningCarrier::selectWithinMultiIATA(const std::vector<TravelSeg*>& tvlSegs,
                                       std::set<CarrierCode>& govCxrSet,
                                       FMDirection fmDirection,
                                       TravelSeg** primarySector)
{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::selectGovCxrWithinTwoIATA()");
  CarrierCode govCxr("XX");

  TravelSeg* travelSeg = nullptr;
  if (fmDirection == FMDirection::INBOUND)
  {
    std::vector<TravelSeg*>::const_reverse_iterator tvlSegItr =
        find_if(tvlSegs.rbegin(), tvlSegs.rend(), LocToTsPred<AreasNotEqual>());

    if (LIKELY(tvlSegItr != tvlSegs.rend()))
    {
      travelSeg = *tvlSegItr;
      if(_trx && _isIataFareSelectionApplicable)
      {
        return getSetOfGoverningCarriers(tvlSegs, travelSeg, govCxrSet, fmDirection, primarySector);
      }
      return setGoverningCarrier(travelSeg, govCxrSet, primarySector);
    }
  }
  else
  {
    std::vector<TravelSeg*>::const_iterator tvlSegItr =
        std::find_if(tvlSegs.begin(), tvlSegs.end(), LocToTsPred<AreasNotEqual>());
    if (LIKELY(tvlSegItr != tvlSegs.end()))
    {
      travelSeg = *tvlSegItr;
      if(_trx && _isIataFareSelectionApplicable)
      {
        return getSetOfGoverningCarriers(tvlSegs, travelSeg, govCxrSet, fmDirection, primarySector);
      }
      return setGoverningCarrier(travelSeg, govCxrSet, primarySector);
    }
  }

  LOG4CXX_DEBUG(logger, "Leaving GoverningCarrier::selectGovCxrWithinTwoIATA()");
  return false;
}

//----------------------------------------------------------------------------
// selectWithinAllIATA()
//----------------------------------------------------------------------------
/**The carrier providing the first flight from/into IATA area 1 is the governing carrier.
 * check the first air segment for IATA area 1 in origin and destination. If found, carrier
 * of this travel segment is the Governing Carrier. Elese if,
 * check the last air segment for IATA area 1 in origin and destination. If found, carrier
 * of this travel segment is the Governing Carrier. Else,
 * Iterate through the vector of air segments( except firs and last one) and compare the
 * IATA area of origin and destination to find the first carrier that provides service to/from
 * IATA area-1 and select that as the governing carrier.
 */
bool
GoverningCarrier::selectWithinAllIATA(const std::vector<TravelSeg*>& tvlSegs,
                                      std::set<CarrierCode>& govCxrSet,
                                      FMDirection fmDirection,
                                      TravelSeg** primarySector)
{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::selectGovCxrWithinAllIATA()");

  TravelSeg* travelSeg = findTravelSegInAreaOne(tvlSegs, fmDirection);
  if (LIKELY(travelSeg))
  {
    bool setRC = setGoverningCarrier(travelSeg, govCxrSet, primarySector);

    LOG4CXX_DEBUG(logger, "setGoverningCarrier returned: " << setRC);
    return setRC;
  }

  return false;
}

//----------------------------------------------------------------------------
// selectWithinSubIATA11()
//----------------------------------------------------------------------------
/**The carrier providing the first flight to/from US/CA is the governing carrier.*/
bool
GoverningCarrier::selectWithinSubIATA11(const std::vector<TravelSeg*>& tvlSegs,
                                        std::set<CarrierCode>& govCxrSet,
                                        FMDirection fmDirection,
                                        TravelSeg** primarySector)
{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::selectGovCxrWithinSubIATA11()");
  CarrierCode govCxr;

  if (fmDirection == FMDirection::INBOUND)
  {
    std::vector<TravelSeg*> rTvlSegs(tvlSegs.size());
    std::copy(tvlSegs.begin(), tvlSegs.end(), rTvlSegs.begin());
    std::reverse(rTvlSegs.begin(), rTvlSegs.end());

    govCxr = getFirstIntlFlt(rTvlSegs, primarySector);
  }
  else
  {
    govCxr = getFirstIntlFlt(tvlSegs, primarySector);
  }

  if (UNLIKELY(govCxr.equalToConst("XX")))
  {
    return false;
  }

  govCxrSet.insert(govCxr);

  // get next candidate for GovCxr
  if(_trx && _isIataFareSelectionApplicable)
  {
    govCxr = getHighestTPMCarrier(tvlSegs, fmDirection, *primarySector);

    if (!govCxr.empty())
    {
      govCxrSet.insert(govCxr);
    }
  }
  return true;
}

//----------------------------------------------------------------------------
// selectWithinSameSubIATA()
//----------------------------------------------------------------------------
/**The carrier providing the first international flight is the governing carrier.
 * This method doesnt process governing carrier logic if it is only within
 * sub-IATA21(Europe).
 */
bool
GoverningCarrier::selectWithinSameSubIATA(const std::vector<TravelSeg*>& tvlSegs,
                                          std::set<CarrierCode>& govCxrSet,
                                          FMDirection fmDirection,
                                          TravelSeg** primarySector)

{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::selectGovCxrWithinSameSubIATA()");
  CarrierCode govCxr("XX");
  if (fmDirection == FMDirection::INBOUND)
  {
    std::vector<TravelSeg*> rTvlSegs(tvlSegs.size());
    std::copy(tvlSegs.begin(), tvlSegs.end(), rTvlSegs.begin());
    std::reverse(rTvlSegs.begin(), rTvlSegs.end());
    govCxr = getFirstIntlFlt(rTvlSegs, primarySector);
  }
  else
  {
    govCxr = getFirstIntlFlt(tvlSegs, primarySector);
  }

  if (UNLIKELY(govCxr.equalToConst("XX")))
  {
    return false;
  }

  govCxrSet.insert(govCxr);

  // get next candidate for GovCxr
  if(_trx && _isIataFareSelectionApplicable)
  {
    govCxr = getHighestTPMCarrier(tvlSegs, fmDirection, *primarySector);

    if (!govCxr.empty())
    {
      govCxrSet.insert(govCxr);
    }
  }
  return true;
}

//----------------------------------------------------------------------------
// selectWithinSubIATA21()
//----------------------------------------------------------------------------
/**The carrier with the lower applicable  fare between first internationl flight and
 * carrier with highest TPM(Ticketed point Mileage) is the governing carrier.
 * @todo
 *           - select first itnernational flight.
 *            - select air segment with highest TPM
 *           - Call farefinder() and get the fare.
 *           - Carrier with the lowest fare is the governing carrier.
 *                  -if multiple carriers have same fare amont.
 *                            - check the booking code. Carrier with higher priority booking
 * code(e.g Y over C)
 *                               is the Governing Carrier.
 */
bool
GoverningCarrier::selectWithinSubIATA21(const std::vector<TravelSeg*>& tvlSegs,
                                        std::set<CarrierCode>& govCxrSet,
                                        FMDirection direction,
                                        TravelSeg** primarySector)
{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::selectGovCxrWithinSubIATA21()");
  CarrierCode govCxr("XX");

  if(_trx && _isIataFareSelectionApplicable)
  {
    if (direction == FMDirection::INBOUND)
    {
      std::vector<TravelSeg*> rTvlSegs(tvlSegs.size());
      std::copy(tvlSegs.begin(), tvlSegs.end(), rTvlSegs.begin());
      std::reverse(rTvlSegs.begin(), rTvlSegs.end());
      govCxr = getFirstIntlFlt(rTvlSegs, primarySector);
    }
    else
    {
      govCxr = getFirstIntlFlt(tvlSegs, primarySector);
    }
  }
  else
    govCxr = getFirstIntlFlt(tvlSegs, primarySector);

  if (govCxr.equalToConst("XX"))
  {
    LOG4CXX_DEBUG(logger, "Leaving GoverningCarrier::selectGovCxrWithinSubIATA21() Failed");
    return false;
  }
  govCxrSet.insert(govCxr);

  // get next candidate for GovCxr
  if(_trx && _isIataFareSelectionApplicable)
  {
    govCxr = getHighestTPMCarrier(tvlSegs, direction, *primarySector);
  }
  else
    govCxr = getHighestTPMCarrierOld(tvlSegs);

  if (govCxr.equalToConst("XX"))
  {
    LOG4CXX_DEBUG(logger, "Leaving GoverningCarrier::selectGovCxrWithinSubIATA21() Failed");
    return false;
  }
  govCxrSet.insert(govCxr);
  LOG4CXX_DEBUG(logger, "Leaving GoverningCarrier::selectGovCxrWithinSubIATA21()");
  return true;
}

//----------------------------------------------------------------------------
// selectWithinCountryExceptUSCA()
//----------------------------------------------------------------------------
/**The carrier with the lowest applicable fare for the entire component is the governing carrier.
 * @todo
 *           - select carriers that  have pbulished through fare for the FareMarket.
 *           - Call farefinder() and get the fare.
 *           - Carrier with the lowest fare is the governing carrier.
 *                  -if multiple carriers have same fare amont.
 *                            - check the booking code. Carrier with higher priority booking code
 * (e.g Y over C)
 *                               is the Governing Carrier.
 */
bool
GoverningCarrier::selectWithinCountryExceptUSCA(const std::vector<TravelSeg*>& tvlSegs,
                                                std::set<CarrierCode>& govCxrSet,
                                                TravelSeg** primarySector)

{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::selectWithinCountryExceptUSCA()");

  std::vector<TravelSeg*>::const_iterator tvlSegItr = tvlSegs.begin();
  for (; tvlSegItr != tvlSegs.end(); tvlSegItr++)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*tvlSegItr);
    CarrierCode govCxr;
    if (airSeg != nullptr)
    {
      govCxr = airSeg->carrier().empty() == false ? airSeg->carrier() : INDUSTRY_CARRIER;
    }
    else
    {
      govCxr = INDUSTRY_CARRIER;
    }
    govCxrSet.insert(govCxr);
  }

  if (LIKELY(!(govCxrSet.empty())))
  {
    LOG4CXX_DEBUG(logger, "Leaving GoverningCarrier::selectWithinCountryExceptUSCA()");
    return true;
  }
  else
  {
    LOG4CXX_DEBUG(logger, "Leaving GoverningCarrier::selectWithinCountryExceptUSCA() Failed");
    return false;
  }
}

//---------------------------------------------------------------------------
// getFirstIntlFlt()
//---------------------------------------------------------------------------
/** Returns the carrier of the first international flight.*/
CarrierCode
GoverningCarrier::getFirstIntlFlt(const std::vector<TravelSeg*>& tvlSegs, TravelSeg** primarySector)
{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::getFirstIntlFlt()");

  std::vector<TravelSeg*>::const_iterator tvlSegItr = tvlSegs.begin();
  for (; tvlSegItr != tvlSegs.end(); tvlSegItr++)
  {
    if (TravelSegUtil::isNotUSCanadaOrCanadaUS(*tvlSegItr))
    {
      if (primarySector)
      {
        *primarySector = (*tvlSegItr);
      }

      const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*tvlSegItr);
      if (airSeg != nullptr) // && (*tvlSegItr)->segmentType() != Arunk)
      {
        return (airSeg->carrier().empty() == true ? INDUSTRY_CARRIER : airSeg->carrier());
      }
      else
      {
        return INDUSTRY_CARRIER;
      }
    }
  }

  LOG4CXX_DEBUG(logger, "leaving GoverningCarrier::getFirstIntlFlt --Failed");
  return "XX";
}

uint32_t
GoverningCarrier::getTPMOld(const AirSeg& airSeg)
{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::getTPMOld");

  const Loc& orig = *airSeg.origin();
  const Loc& dest = *airSeg.destination();

  DataHandle dataHandle;
  const Mileage* mileage =
      dataHandle.getMileage(orig.loc(), dest.loc(), TPM, GlobalDirection::EH, airSeg.departureDT());

  if (!mileage)
  {
    LOG4CXX_DEBUG(logger, "TPM Not Found in Database");

    return TseUtil::greatCircleMiles(orig, dest);
  }
  else
  {
    return mileage->mileage();
  }
}

bool
GoverningCarrier::isDomesticOld(const AirSeg& airSeg)
{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::isDomesticOld()");

  if (airSeg.origin()->nation() == airSeg.destination()->nation())
  {
    return true;
  }
  return false;
}

bool
GoverningCarrier::isDomestic(const AirSeg& airSeg, const bool withinScandinavia)
{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::isDomestic()");

  if (airSeg.origin()->nation() == airSeg.destination()->nation())
  {
    return true;
  }
  if ((LocUtil::isRussia(*(airSeg.origin())) &&
       LocUtil::isRussia(*(airSeg.destination()))) ||
      (LocUtil::isDomesticUSCA(*(airSeg.origin())) &&
       LocUtil::isDomesticUSCA(*(airSeg.destination()))) ||
      (!withinScandinavia && 
       LocUtil::isScandinavia(*(airSeg.origin())) &&
       LocUtil::isScandinavia(*(airSeg.destination()))) )
    return true;

  return false;
}

CarrierCode
GoverningCarrier::getHighestTPMCarrierOld(const std::vector<TravelSeg*>& tvlSegs)
{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::getHighestTPMCarrierOld()");

  uint32_t currentTPM = 0;
  CarrierCode currentCarrier = "";
  uint32_t highestTPM = 0;
  CarrierCode highestTPMCarrier = "";
  bool intlFlight = false; // domestic or international

  std::vector<TravelSeg*>::const_iterator itr = tvlSegs.begin();
  for (; itr != tvlSegs.end(); itr++)
  {
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(*itr);
    if (!airSeg)
    {
      continue;
    }

    const uint32_t TPM = getTPMOld(*airSeg);
    if (currentCarrier.empty())
    {
      currentTPM = TPM;
      currentCarrier = airSeg->carrier().empty() == true ? INDUSTRY_CARRIER : airSeg->carrier();
      if (!isDomesticOld(*airSeg))
        intlFlight = true;
      continue;
    }

    if (airSeg->carrier() == currentCarrier)
    {
      currentTPM += TPM;

      if (!isDomesticOld(*airSeg))
        intlFlight = true;
    }
    else
    {
      if (intlFlight)
      {
        if (highestTPM < currentTPM)
        {
          highestTPM = currentTPM;
          highestTPMCarrier = currentCarrier;
        }
      }

      currentTPM = TPM;
      currentCarrier = airSeg->carrier().empty() == true ? INDUSTRY_CARRIER : airSeg->carrier();
      intlFlight = false;

      if (!isDomesticOld(*airSeg))
        intlFlight = true;
    }
  }

  if (intlFlight)
  {
    if (highestTPM < currentTPM)
    {
      highestTPM = currentTPM;
      highestTPMCarrier = currentCarrier;
    }
  }

  return highestTPMCarrier;
}

CarrierCode
GoverningCarrier::getHighestTPMCarrier(const std::vector<TravelSeg*>& tvlSegs,
                                             FMDirection direction,
                                             TravelSeg*& primarySector  )
{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::getHighestTPMCarrier()");
  uint32_t currentTPM = 0;
  CarrierCode currentCarrier = CarrierCode();
  uint32_t highestTPM = 0;
  CarrierCode highestTPMCarrier = CarrierCode();

  uint32_t currentCxrHighTPMSector = 0;
  TravelSeg* currentCxrHighTPMprimeSector = nullptr;

  bool intlFlight = false; // domestic or international
  // if all flights withIn Scandinavia,
  // then travel between Nations is International
  bool withinScandinavia = LocUtil::isAllFlightsWithInScandinavia(tvlSegs);

  std::vector<TravelSeg*>::const_iterator itr = tvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator eitr = tvlSegs.end();

  for (; itr != eitr; ++itr)
  {
    const AirSeg* airSeg = dynamic_cast<AirSeg*>(*itr);
    if (!airSeg)
    {
      if (intlFlight && 
          ((currentTPM > highestTPM) ||
           ((currentTPM >= highestTPM) && direction == FMDirection::INBOUND)))
      {
        highestTPM = currentTPM;
        highestTPMCarrier = currentCarrier;
        if (&primarySector)
          primarySector = currentCxrHighTPMprimeSector;
        intlFlight = false;
      }
      currentCarrier.clear();
      continue;
    }
    // gets TPM (even thru MPM or GCM)
    const uint32_t TPM = getTPM(*airSeg);

    if (currentCarrier.empty())
    {
      currentTPM = TPM;
      currentCarrier = airSeg->carrier();
      if (!isDomestic(*airSeg, withinScandinavia))
      {
        intlFlight = true;
        currentCxrHighTPMSector = currentTPM;
        currentCxrHighTPMprimeSector = *itr;
      }
      continue;
    }

    if (airSeg->carrier() == currentCarrier)
    {
      currentTPM += TPM;
      if (!isDomestic(*airSeg, withinScandinavia))
      {
        intlFlight = true;
        if((currentCxrHighTPMSector < TPM) ||
           ((currentCxrHighTPMSector == TPM) && direction == FMDirection::INBOUND))
        {
          currentCxrHighTPMSector = TPM;
          currentCxrHighTPMprimeSector = *itr;
        }
      }
    }
    else
    {
      if (intlFlight &&
          ((currentTPM > highestTPM) ||
           ((currentTPM >= highestTPM) && direction == FMDirection::INBOUND)))
      {
        highestTPM = currentTPM;
        highestTPMCarrier = currentCarrier;
        if (&primarySector)
           primarySector = currentCxrHighTPMprimeSector;
      }
      if (!isDomestic(*airSeg, withinScandinavia))
      {
        intlFlight = true;
        currentCxrHighTPMSector = TPM;
        currentCxrHighTPMprimeSector = *itr;
      }
      else
        intlFlight = false;

      currentTPM = TPM;
      currentCarrier = airSeg->carrier();
    }
  }
  if(intlFlight &&
     ((currentTPM > highestTPM) ||
      ((currentTPM >= highestTPM) && direction == FMDirection::INBOUND)))
  {
     highestTPM = currentTPM;
     highestTPMCarrier = currentCarrier;
     if (&primarySector)
       primarySector = currentCxrHighTPMprimeSector;
  }

  return highestTPMCarrier;
}

uint32_t
GoverningCarrier::getTPM(const AirSeg& airSeg)
{
  if(!trx())
    return getTPMWn(airSeg); // support WN request

  GlobalDirection gd;

  std::vector<TravelSeg*> tvlSegs = {const_cast<AirSeg*>(&airSeg)};
  // find the global direction
  GlobalDirectionFinderV2Adapter::getGlobalDirection(trx(), airSeg.departureDT(), tvlSegs, gd);

  const Loc& loc1 = *(airSeg.origin());
  const Loc& loc2 = *(airSeg.destination());

  return LocUtil::getTPM(loc1, loc2, gd, airSeg.departureDT(), trx()->dataHandle());
}

uint32_t
GoverningCarrier::getTPMWn(const AirSeg& airSeg)
{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::getTPMOld");

  const Loc& orig = *airSeg.origin();
  const Loc& dest = *airSeg.destination();

  DataHandle dataHandle;
  const Mileage* mileage =
      dataHandle.getMileage(orig.loc(), dest.loc(), TPM, GlobalDirection::ZZ, airSeg.departureDT());

  if (!mileage)
  {
    LOG4CXX_DEBUG(logger, "TPM Not Found in Database");

    return TseUtil::greatCircleMiles(orig, dest);
  }
  else
  {
    return mileage->mileage();
  }
}

bool
GoverningCarrier::processCarrierPreference(FareMarket& fareMarket)
{
  LOG4CXX_INFO(logger, "Entered GoverningCarrier::processCarrierPreference");

  const CarrierCode& carrier = fareMarket.governingCarrier();
  const DateTime& travelDate = fareMarket.travelDate();
  const CarrierPreference* cp = trx()->dataHandle().getCarrierPreference(carrier, travelDate);

  // If there isnt one specified get the default
  if (UNLIKELY(cp == nullptr))
    cp = trx()->dataHandle().getCarrierPreference(EMPTY_STRING(), travelDate);

  fareMarket.governingCarrierPref() = cp;

  if (UNLIKELY(cp == nullptr))
    return false;

  return true;
}

bool
GoverningCarrier::setGoverningCarrier(TravelSeg* travelSeg,
                                      std::set<CarrierCode>& govCxrSet,
                                      TravelSeg** primarySector)
{
  if (travelSeg)
  {
    CarrierCode govCxr;
    const AirSeg* airSeg = dynamic_cast<const AirSeg*>(travelSeg);
    if (airSeg != nullptr)
    {
      // Open or Air
      govCxr = (airSeg->carrier().empty() == true ? INDUSTRY_CARRIER : airSeg->carrier());
    }
    else
    {
      govCxr = INDUSTRY_CARRIER; // surface segmet
    }

    govCxrSet.insert(govCxr);

    if (primarySector)
    {
      *primarySector = travelSeg;
    }
    return true;
  }

  LOG4CXX_DEBUG(logger, "Leaving GoverningCarrier::setGoverningCarrier() - FAILED");
  return false;
}

TravelSeg*
GoverningCarrier::findTravelSegInAreaOne(const std::vector<TravelSeg*>& tvlSegs,
                                         FMDirection fmDirection)
{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::findTravelSegInAreaOne()");

  if (fmDirection == FMDirection::INBOUND)
  {
    LOG4CXX_DEBUG(logger, "FareMarket direction INBOUND");

    std::vector<TravelSeg*>::const_reverse_iterator tvlSegItr =
        find_if(tvlSegs.rbegin(), tvlSegs.rend(), SubAreasAreIATA1AndNotEqual());

    if (LIKELY(tvlSegItr != tvlSegs.rend()))
      return *tvlSegItr;
  }
  else
  {
    LOG4CXX_DEBUG(logger, "FareMarket direction OUTBOUND/UNKNOWN");

    std::vector<TravelSeg*>::const_iterator tvlSegItr =
        std::find_if(tvlSegs.begin(), tvlSegs.end(), SubAreasAreIATA1AndNotEqual());

    if (LIKELY(tvlSegItr != tvlSegs.end()))
      return *tvlSegItr;
  }

  LOG4CXX_DEBUG(logger, "Leaving GoverningCarrier::findTravelSegInAreaOne()");

  // Travel Seg not found
  return nullptr;
}

//----------------------------------------------------------------------------
bool
GoverningCarrier::getGovCxrSpecialCases(FareMarket& fareMarketRef)
{
  if (UNLIKELY(_isRtw && fareMarketRef.travelSeg().size() == _itin->travelSeg().size()))
    return false;

  SmallBitSet<uint8_t, FMTravelBoundary>& boundary = fareMarketRef.travelBoundary();

  if(_trx && _isIataFareSelectionApplicable)
  {
    if (getGovCxrSpecialDomesticNonUSCA(fareMarketRef, boundary))
      return true;

    if (getGovCxrSpecialDomesticUSCA(fareMarketRef, boundary))
      return true;

    if (getGovCxrExceptAllDomestic(fareMarketRef, boundary))
      return true;
  }
  else
  {
    if (getGovCxrSpecialEuropeOnly(fareMarketRef, boundary))
      return true;

    if (LIKELY(getGovCxrSpecialDomesticNonUSCA(fareMarketRef, boundary)))
      return true;

    if (getGovCxrSpecialDomesticUSCA(fareMarketRef, boundary))
      return true;
  }
    return false;
}

bool
GoverningCarrier::getGovCxrSpecialEuropeOnly(FareMarket& fareMarketRef, TravelBoundarySet& boundary)

{
  if (boundary.isSet(FMTravelBoundary::TravelWithinSubIATA21))
  {
    // Set the governing carrier to the first carrier in the
    // fare market that crosses an international boundary
    fareMarketRef.governingCarrier() = TravelSegAnalysis::getFirstIntlFlt(
        fareMarketRef.travelSeg(), fareMarketRef.primarySector());
    return true;
  }

  return false;
}

bool
GoverningCarrier::getGovCxrExceptAllDomestic(FareMarket& fareMarketRef, TravelBoundarySet& boundary)

{
  if (!boundary.isSet(FMTravelBoundary::TravelWithinUSCA) && 
      !boundary.isSet(FMTravelBoundary::TravelWithinSameCountryExceptUSCA))
  {
    // Set the governing carrier to the first carrier in the
    // fare market that crosses an international boundary
    fareMarketRef.governingCarrier() = TravelSegAnalysis::getFirstIntlFlt(
        fareMarketRef.travelSeg(), fareMarketRef.primarySector());
    return true;
  }

  return false;
}

bool
GoverningCarrier::getGovCxrSpecialDomesticNonUSCA(FareMarket& fareMarketRef,
                                                  TravelBoundarySet& boundary)
{

  if (LIKELY(boundary.isSet(FMTravelBoundary::TravelWithinSameCountryExceptUSCA)))
  {
    // Case 2
    // Get the first segment in the market
    AirSeg* airSegPtr = dynamic_cast<AirSeg*>(fareMarketRef.travelSeg().front());
    if (LIKELY(airSegPtr != nullptr))
    {
      // Set the governing carrier to the carrier of the first
      // segment in the flight
      fareMarketRef.governingCarrier() = airSegPtr->carrier();
      fareMarketRef.primarySector() = fareMarketRef.travelSeg().front();

      return true;
    }
  }

  return false;
}

bool
GoverningCarrier::getGovCxrSpecialDomesticUSCA(FareMarket& fareMarketRef,
                                               TravelBoundarySet& boundary)
{
  if (boundary.isSet(FMTravelBoundary::TravelWithinUSCA))
  {
    // First get the origin
    const Loc* orig = fareMarketRef.origin();
    bool foundOrigin = false;
    // Find the air segment that is the origin
    std::vector<TravelSeg*>::iterator trvSegIter = fareMarketRef.travelSeg().begin();
    std::vector<TravelSeg*>::iterator trvSegEndIter = fareMarketRef.travelSeg().end();

    for (; trvSegIter != trvSegEndIter; ++trvSegIter)
    {
      AirSeg* curAirSeg = dynamic_cast<AirSeg*>(*trvSegIter);
      if (curAirSeg != nullptr)
      {
        // If the cur air segment origin is equal to the fare
        // market origin, then we must have the origin
        // Set the governing carrier to the carrier that is
        // flying the origin segment
        if (curAirSeg->origin()->loc().compare(orig->loc()) == 0)
        {
          fareMarketRef.governingCarrier() = curAirSeg->carrier();
          fareMarketRef.primarySector() = *trvSegIter;

          foundOrigin = true;
          break;
        }
      }
    }

    if (!foundOrigin)
    {
      // If we cannot find the origin, use the first carrier
      AirSeg* aSegPtr = dynamic_cast<AirSeg*>(fareMarketRef.travelSeg().front());
      if (aSegPtr != nullptr)
      {
        fareMarketRef.governingCarrier() = aSegPtr->carrier();
        fareMarketRef.primarySector() = fareMarketRef.travelSeg().front();

        return true;
      }
    }
  }

  return false;
}

uint32_t
GoverningCarrier::getHighestTPMByCarrier(const CarrierCode& cxr,
                                         const std::vector<TravelSeg*>& tvlSegs)
{
  uint32_t lastTPM = 0;
  uint32_t highestTPM = 0;

  // if all flights withIn Scandinavia,
  // then travel between Nations is International
  bool withinScandinavia = LocUtil::isAllFlightsWithInScandinavia(tvlSegs);
  bool hasInternationalSegment = false;
  std::vector<TravelSeg*>::const_iterator itr = tvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator itrE = tvlSegs.end();

  for (; itr != itrE; ++itr)
  {
    const AirSeg* airSeg = (*itr)->toAirSeg();
    if (!airSeg || (cxr != airSeg->carrier()))
    {
      if (hasInternationalSegment && (lastTPM >= highestTPM))
      {
        highestTPM = lastTPM;
        lastTPM = 0;
      }
      continue;
    }

    // If it gets here, the travel segment's carrier is the
    // same as the cxr requested.
    uint32_t currentTPM = airSeg->mileage();
    if (currentTPM==0)
      currentTPM = getTPM(*airSeg);

    lastTPM += currentTPM;
    if (!hasInternationalSegment)
      hasInternationalSegment = (isDomestic(*airSeg, withinScandinavia)==false);
  }

  if (hasInternationalSegment && (lastTPM >= highestTPM))
  {
    highestTPM = lastTPM;
    lastTPM = 0;
  }

  return highestTPM;
}

//Returns highest Governing carrier for Foreign Domestic Fare Markets
CarrierCode
GoverningCarrier::getForeignDomHighestTPMCarrier(const std::vector<TravelSeg*>& tvlSegs,
                                             FMDirection direction,
                                             TravelSeg*& primarySector)
{
  LOG4CXX_DEBUG(logger, "Entered GoverningCarrier::getForeignDomHighestTPMCarrier()");
  uint32_t currentTPM = 0;
  CarrierCode currentCarrier = CarrierCode();
  uint32_t highestTPM = 0;
  CarrierCode highestTPMCarrier = CarrierCode();

  uint32_t currentCxrHighTPMSector = 0;
  TravelSeg* currentCxrHighTPMprimeSector = nullptr;

  std::vector<TravelSeg*>::const_iterator itr = tvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator eitr = tvlSegs.end();

  for (; itr != eitr; ++itr)
  {
    const AirSeg* airSeg = dynamic_cast<AirSeg*>(*itr);
    if (!airSeg) continue;

    const uint32_t TPM = getTPM(*airSeg);
    if (currentCarrier.empty())
    {
      currentTPM = TPM;
      currentCarrier = airSeg->carrier();
      currentCxrHighTPMSector = currentTPM;
      currentCxrHighTPMprimeSector = *itr;
      continue;
    }

    if (airSeg->carrier() == currentCarrier)
    {
      currentTPM += TPM;
      if((currentCxrHighTPMSector < TPM) ||
           ((currentCxrHighTPMSector == TPM) && direction  == FMDirection::INBOUND))
      {
        currentCxrHighTPMSector = TPM;
        currentCxrHighTPMprimeSector = *itr;
      }
    }
    else
    {
      if((currentTPM > highestTPM) ||
         ((currentTPM >= highestTPM) && direction == FMDirection::INBOUND))
      {
        highestTPM = currentTPM;
        highestTPMCarrier = currentCarrier;
        if (&primarySector)
          primarySector = currentCxrHighTPMprimeSector;
      }
      currentTPM = TPM;
      currentCarrier = airSeg->carrier();
      currentCxrHighTPMSector = TPM;
      currentCxrHighTPMprimeSector = *itr;
    }
  }

  if((currentTPM > highestTPM) ||((currentTPM >= highestTPM) && direction == FMDirection::INBOUND))
  {
     highestTPM = currentTPM;
     highestTPMCarrier = currentCarrier;
     if (&primarySector)
       primarySector = currentCxrHighTPMprimeSector;
  }

  return highestTPMCarrier;
}

//Return highest mileage for Foreign Domestic Fare Markets
uint32_t
GoverningCarrier::getForeignDomHighestTPMByCarrier(const CarrierCode& cxr,
                                         const std::vector<TravelSeg*>& tvlSegs)
{
  uint32_t lastTPM = 0;
  uint32_t highestTPM = 0;

  std::vector<TravelSeg*>::const_iterator itr = tvlSegs.begin();
  std::vector<TravelSeg*>::const_iterator itrE = tvlSegs.end();

  for (; itr != itrE; ++itr)
  {
    const AirSeg* airSeg = (*itr)->toAirSeg();
    if (!airSeg || (cxr != airSeg->carrier()))
    {
      if (lastTPM >= highestTPM)
      {
        highestTPM = lastTPM;
        lastTPM = 0;
      }
      continue;
    }

    uint32_t currentTPM = airSeg->mileage();
    if (currentTPM==0)
      currentTPM = getTPM(*airSeg);

    lastTPM += currentTPM;
  }

  if (lastTPM >= highestTPM)
  {
    highestTPM = lastTPM;
    lastTPM = 0;
  }

  return highestTPM;
}

} // tse
