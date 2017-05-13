#pragma once

#include "Routing/RoutingConsts.h"

#include <map>

namespace tse
{

class MileageRouteItem;
class TpdPsrViaGeoLoc;

typedef std::map<LocCode, TpdViaGeoLocMatching> FareCalcMatching;

// this vector contains travel segment numbers where the matched VIA GEO locs belong to
typedef std::vector<int16_t> MatchedTPD;

class FareCalcVisitor
{
public:
  void recordMatch(const MileageRouteItem&, const TpdPsrViaGeoLoc&);
  const FareCalcMatching& getMatching() const;
  void resetMatching(FareCalcMatching& matching);
  void resetMatching();

  void recordMatchedTPD(int16_t segNumber);
  const MatchedTPD& getMatchedTPD() const;
  void resetMatchedTPD(MatchedTPD& matchedTPD);
  void resetMatchedTPD();

private:
  TpdViaGeoLocMatching getMatchType(const TpdPsrViaGeoLoc&) const;

  FareCalcMatching _matchedTpdViaGeoLocs;
  MatchedTPD _matchedTPDseg;
  bool _failedDirService;
};

} // namespace tse

