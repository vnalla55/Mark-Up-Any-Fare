#pragma once

namespace tse
{

class MileageRouteItem;
class TpdPsrViaGeoLoc;

class IsInLoc
{
public:
  IsInLoc(const TpdPsrViaGeoLoc& loc, bool& cond, bool origLoc1);
  IsInLoc(const TpdPsrViaGeoLoc& loc, bool& cond);
  bool operator()(MileageRouteItem& item);
  bool conditional();

private:
  const TpdPsrViaGeoLoc& _loc;
  bool& _cond;
  bool _origLoc1;
};
}

