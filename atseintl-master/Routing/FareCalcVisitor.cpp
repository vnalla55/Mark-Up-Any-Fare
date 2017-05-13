#include "Routing/FareCalcVisitor.h"

#include "DBAccess/TpdPsrViaGeoLoc.h"
#include "Routing/MileageRouteItem.h"

namespace tse
{
void
FareCalcVisitor::recordMatch(const MileageRouteItem& item, const TpdPsrViaGeoLoc& loc)
{
  _matchedTpdViaGeoLocs.insert(std::make_pair(item.destCityOrAirport()->loc(), getMatchType(loc)));
}

const FareCalcMatching&
FareCalcVisitor::getMatching() const
{
  return _matchedTpdViaGeoLocs;
}

void
FareCalcVisitor::resetMatching(FareCalcMatching& matching)
{
  _matchedTpdViaGeoLocs = matching;
}

void
FareCalcVisitor::resetMatching()
{
  _matchedTpdViaGeoLocs.clear();
}

TpdViaGeoLocMatching
FareCalcVisitor::getMatchType(const TpdPsrViaGeoLoc& loc) const
{
  return loc.loc().locType() == LOCTYPE_CITY ? CITY_MATCH : GENERAL_MATCH;
}
////NEW
void
FareCalcVisitor::recordMatchedTPD(int16_t segNumber)
{
  _matchedTPDseg.push_back(segNumber);
}

const MatchedTPD&
FareCalcVisitor::getMatchedTPD() const
{
  return _matchedTPDseg;
}

void
FareCalcVisitor::resetMatchedTPD(MatchedTPD& matchedTPD)
{
  _matchedTPDseg = matchedTPD;
}

void
FareCalcVisitor::resetMatchedTPD()
{
  _matchedTPDseg.clear();
}
}
