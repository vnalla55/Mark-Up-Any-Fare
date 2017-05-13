

#pragma once

#include "Routing/RestrictionInfo.h"
#include "Routing/RtgKey.h"
#include "Routing/TravelRoute.h"

#include <map>
#include <vector>

namespace tse
{

class Routing;
class DRVInfo;

typedef std::vector<std::string> RoutingMapStrings;
typedef std::vector<DRVInfo*> DRVInfos;

class MapInfo : public ValidationInfo
{

public:
  MapInfo()
    : _routeStrings(nullptr),
      _drvStatus(false),
      _missingCityIndex(0),
      _postDRVmissingCityIndex(0),
      _missingCarrier(false)
  {
  }

  RoutingMapStrings* routeStrings() const { return _routeStrings; }
  RoutingMapStrings*& routeStrings() { return _routeStrings; }
  const DRVInfos& drvInfos() const { return _drvInfos; }
  DRVInfos& drvInfos() { return _drvInfos; }
  bool drvStatus() const { return _drvStatus; }
  bool& drvStatus() { return _drvStatus; }
  int16_t missingCityIndex() const { return _missingCityIndex; }
  int16_t& missingCityIndex() { return _missingCityIndex; }
  int16_t postDRVmissingCityIndex() const { return _postDRVmissingCityIndex; }
  int16_t& postDRVmissingCityIndex() { return _postDRVmissingCityIndex; }
  bool missingCarrier() const { return _missingCarrier; }
  bool& missingCarrier() { return _missingCarrier; }

private:
  RoutingMapStrings* _routeStrings;
  DRVInfos _drvInfos;
  bool _drvStatus;
  int16_t _missingCityIndex;
  int16_t _postDRVmissingCityIndex;
  bool _missingCarrier;
};
}
