#include "ItinAnalyzer/CodeShareValidatorSOL.h"

#include "Common/Config/ConfigurableValue.h"
#include "Common/GoverningCarrier.h"
#include "Common/ItinUtil.h"
#include "Common/LocUtil.h"
#include "Common/ShoppingUtil.h"
#include "Common/TravelSegAnalysis.h"
#include "DataModel/AirSeg.h"
#include "DataModel/FareMarket.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/ShoppingTrx.h"

#include <boost/optional.hpp>

namespace tse
{
FALLBACK_DECL(fallbackExcludeCodeSharingFix);

namespace
{
ConfigurableValue<bool>
excludeCodeShareSOP("SHOPPING_DIVERSITY", "EXCLUDE_CODE_SHARE_SOP", false);
ConfigurableValue<bool>
disableShareForEurope("SHOPPING_DIVERSITY", "DISABLE_CODE_SHARE_FOR_EUROPE", false);
}

bool
shouldExcludeCodeSharingSOL()
{
  static boost::optional<bool> excludeCodeSharing;

  if (!excludeCodeSharing)
  {
    bool result = false;
    std::string configVal;
    if (Global::config().getValue("EXCLUDE_CODE_SHARE_SOP", configVal, "SHOPPING_DIVERSITY"))
      result = ("Y" == configVal || "T" == configVal);
    excludeCodeSharing = result;
  }

  return excludeCodeSharing.get();
}

bool
shouldDisableCodeSharingForEurope()
{
  static boost::optional<bool> disableCodeShareForEurope;

  if (UNLIKELY(!disableCodeShareForEurope))
  {
    bool result = false;
    std::string configVal;
    if (Global::config().getValue("DISABLE_CODE_SHARE_FOR_EUROPE", configVal, "SHOPPING_DIVERSITY"))
      result = ("Y" == configVal || "T" == configVal);
    disableCodeShareForEurope = result;
  }

  return disableCodeShareForEurope.get();
}

CodeShareValidatorSOL::CodeShareValidatorSOL(ShoppingTrx& trx,
                                             const Itin* itin,
                                             int32_t legIndex,
                                             GeoTravelType geoTvlTypeForThruFM)
  : _trx(trx),
    _itin(itin),
    _legIndex(legIndex),
    _geoTvlTypeForThruFM(geoTvlTypeForThruFM)
{
  _codeShareValidationEnabled = isCodeShareValidationEnabled(geoTvlTypeForThruFM);
  if (_codeShareValidationEnabled)
    gatherGoverningCarriers();
}

void
CodeShareValidatorSOL::gatherGoverningCarriers()
{
  std::vector<CarrierCode> govCxrs;
  if (LIKELY(ShoppingUtil::retrieveCarrierList(_trx, _legIndex - 1, govCxrs)))
  {
    _governingCarriers.insert(govCxrs.begin(), govCxrs.end());
  }
}

bool
CodeShareValidatorSOL::shouldCreateLocalFareMarkets()
{
  if (!_codeShareValidationEnabled)
    return true;

  if (fallback::fallbackExcludeCodeSharingFix(&_trx))
  { // old code
    if (isCodeSharedTwoSegmentMarket())
      return false;
  }

  _governingSegment = findItinGoverningSegment();

  if (fallback::fallbackExcludeCodeSharingFix(&_trx))
  { // old code
    if (!_governingSegment || isCodeSharedSegment(_governingSegment))
      return false;
  }
  else
  {
    // JIRA SCI-245: Check if governingSegment exist and it has code share.
    //              If so, this local FM should not be created. IT should be skipped.
    if (_governingSegment && isCodeSharedSegment(_governingSegment))
      return false;
  }

  return true;
}

bool
CodeShareValidatorSOL::shouldSkipLocalFareMarketPair(const std::vector<TravelSeg*>& fareMarketSegments1,
                                                     const std::vector<TravelSeg*>& fareMarketSegments2)
    const
{
  if (UNLIKELY(
          (fareMarketSegments1.empty() &&
           fareMarketSegments2.size() == _itin->travelSeg().size()) ||
          (fareMarketSegments2.empty() && fareMarketSegments1.size() == _itin->travelSeg().size())))
    return false;

  // Only skip creating the FM when it's international and it has
  // code-sharing segment.
  if ((_geoTvlTypeForThruFM == GeoTravelType::International) &&
      (isLocalForeignDomesticHasCodeShare(fareMarketSegments1) ||
       isLocalForeignDomesticHasCodeShare(fareMarketSegments2)))
    return true;

  if (!_codeShareValidationEnabled || !_governingSegment)
    return false;

  if (!isAcceptableFareMarket(fareMarketSegments1) || !isAcceptableFareMarket(fareMarketSegments2))
    return true;

  return false;
}

bool
CodeShareValidatorSOL::isLocalForeignDomesticHasCodeShare(const std::vector<TravelSeg*>& segs) const
{
  bool isLocalForeignDomestic =
      LocUtil::isForeignDomestic(*segs.front()->origin(), *segs.back()->destination());

  if (isLocalForeignDomestic)
  {
    for (const TravelSeg* segment : segs)
    {
      if (UNLIKELY(!segment->isAir()))
        continue;

      const AirSeg* airSeg(static_cast<const AirSeg*>(segment));
      if (airSeg->marketingCarrierCode() != airSeg->operatingCarrierCode())
        return true;
    }
  }

  return false;
}

bool
CodeShareValidatorSOL::hasCodeSharedGoverningSegment(const FareMarket* fareMarket) const
{
  if (!_codeShareValidationEnabled || !fareMarket->primarySector() ||
      (fareMarket->primarySector() && !fareMarket->primarySector()->isAir()))
    return false;

  const AirSeg* airSeg(static_cast<const AirSeg*>(fareMarket->primarySector()));
  if (isCodeSharedSegment(airSeg))
    return true;

  return false;
}

bool
CodeShareValidatorSOL::isGovernigSegmentFound() const
{
  return _governingSegment != nullptr;
}

bool
CodeShareValidatorSOL::isAcceptableFareMarket(const std::vector<TravelSeg*>& fareMarketSegments) const
{
  bool hasGoverningSegment = false;
  bool hasCodeSharedSegment = false;
  for (const TravelSeg* segment : fareMarketSegments)
  {
    if (UNLIKELY(!segment->isAir()))
      continue;

    if (segment == _governingSegment)
    {
      hasGoverningSegment = true;
      continue;
    }

    const AirSeg* airSeg(static_cast<const AirSeg*>(segment));
    if (isCodeSharedSegment(airSeg))
      hasCodeSharedSegment = true;
  }

  if (hasCodeSharedSegment && !hasGoverningSegment)
    return false;

  return true;
}

bool
CodeShareValidatorSOL::isCodeShareValidationEnabled(GeoTravelType geoTvlTypeForThruFM) const
{
  if (UNLIKELY(!excludeCodeShareSOP.getValue()))
    return false;

  if (geoTvlTypeForThruFM == GeoTravelType::Domestic || geoTvlTypeForThruFM == GeoTravelType::ForeignDomestic)
    return false;

  const Loc& origin(*_itin->travelSeg().front()->origin());
  const Loc& destination(*_itin->travelSeg().back()->destination());

  if (LocUtil::isDomestic(origin, destination) || LocUtil::isTransBorder(origin, destination) ||
      LocUtil::isForeignDomestic(origin, destination))
  {
    return false;
  }

  Boundary boundary(TravelSegAnalysis::selectTravelBoundary(_itin->travelSeg()));

  if (boundary == Boundary::AREA_21 && disableShareForEurope.getValue())
    return false;

  return true;
}

bool
CodeShareValidatorSOL::isCodeSharedSegment(const AirSeg* segment) const
{
  if (segment->marketingCarrierCode() == segment->operatingCarrierCode())
    return false;

  if (_governingCarriers.count(segment->operatingCarrierCode()))
    return false;

  return true;
}

bool
CodeShareValidatorSOL::isCodeSharedTwoSegmentMarket() const
{
  if (_itin->travelSeg().size() > 2)
    return false;

  for (const TravelSeg* segment : _itin->travelSeg())
  {
    if (UNLIKELY(!segment->isAir()))
      continue;

    const AirSeg* airSeg(static_cast<const AirSeg*>(segment));
    if (isCodeSharedSegment(airSeg))
      return true;
  }

  return false;
}

AirSeg*
CodeShareValidatorSOL::findItinGoverningSegment() const
{
  const std::vector<TravelSeg*>& segments(_itin->travelSeg());

  const LocCode& board(segments.front()->boardMultiCity());
  const LocCode& off(segments.back()->offMultiCity());
  const Loc* origin(segments.front()->origin());
  const Loc* destination(segments.back()->destination());

  FareMarket fareMarket;

  fareMarket.origin() = origin;
  fareMarket.destination() = destination;
  fareMarket.boardMultiCity() = board;
  fareMarket.offMultiCity() = off;
  fareMarket.travelDate() = _itin->travelDate();
  fareMarket.legIndex() = _legIndex - 1;
  fareMarket.travelSeg() = segments;

  if (_legIndex == 1)
    fareMarket.direction() = FMDirection::OUTBOUND;
  else
    fareMarket.direction() = FMDirection::INBOUND;

  Boundary boundary(TravelSegAnalysis::selectTravelBoundary(segments));
  ItinUtil::setGeoTravelType(boundary, fareMarket);

  GoverningCarrier govCarrier(&_trx);
  govCarrier.process(fareMarket);
  if (UNLIKELY(!fareMarket.primarySector() && boundary == Boundary::AREA_21))
  {
    if (fareMarket.direction() == FMDirection::OUTBOUND)
      TravelSegAnalysis::getFirstIntlFlt(segments, fareMarket.primarySector());
    else
      TravelSegAnalysis::getLastIntlFlt(segments, fareMarket.primarySector());
  }

  AirSeg* result = nullptr;
  if (LIKELY(fareMarket.primarySector() && fareMarket.primarySector()->isAir()))
    result = static_cast<AirSeg*>(fareMarket.primarySector());

  return result;
}
}
