#include "MinFares/test/TestUtil.h"

#include "DataModel/AirSeg.h"
#include "DBAccess/Loc.h"
#include "Common/LocUtil.h"
#include "Common/TseCodeTypes.h"

using namespace tse;

AirSeg*
TestUtil::createAirSeg(int segNumber, const Loc* orig, const Loc* dest, const CarrierCode& carrier)
{
  AirSeg* seg = new AirSeg();
  seg->segmentOrder() = segNumber;
  seg->origin() = orig;
  seg->destination() = dest;
  seg->boardMultiCity() = orig->loc();
  seg->offMultiCity() = dest->loc();

  if (LocUtil::isInternational(*orig, *dest))
    seg->geoTravelType() = GeoTravelType::International;
  else if (LocUtil::isDomestic(*orig, *dest))
    seg->geoTravelType() = GeoTravelType::Domestic;
  else if (LocUtil::isForeignDomestic(*orig, *dest))
    seg->geoTravelType() = GeoTravelType::ForeignDomestic;
  else if (LocUtil::isTransBorder(*orig, *dest))
    seg->geoTravelType() = GeoTravelType::Transborder;
  else
    seg->geoTravelType() = GeoTravelType::UnknownGeoTravelType;
  seg->carrier() = carrier;

  return seg;
}
