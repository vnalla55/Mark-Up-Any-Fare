#include "Routing/MPMCollectorWN.h"

#include "DBAccess/DataHandle.h"
#include "Routing/GlobalDirectionRetriever.h"
#include "Routing/MileageDataRetriever.h"
#include "Routing/MileageRoute.h"
#include "Routing/MPMRetriever.h"
#include "Routing/PermissibleSpecifiedRouting.h"
#include "Routing/Retriever.h"

#include <set>

namespace tse
{

static const uint16_t
southAmericaCountriesNo(14);
static const NationCode southAmericaCountries[] = { "AR", "BO", "BR", "CL", "CO", "EC", "GF",
                                                    "GY", "PA", "PE", "PY", "SR", "VE", "UY" };
static const NationCode
SouthPacific("34");
static const NationCode
NorthAmerica("11");

MPMCollectorWN::~MPMCollectorWN() {}

bool
MPMCollectorWN::collectMileage(MileageRoute& mileageRoute) const
{
  DataHandle& dataHandle(*mileageRoute.dataHandle());
  const GlobalDirectionRetriever& gdRetriever(tse::Singleton<GlobalDirectionRetriever>::instance());

  MileageRoute tempRoute;
  tempRoute.partialInitialize(mileageRoute);
  tempRoute.globalDirection() = GlobalDirection::ZZ;

  Loc* origin(mileageRoute.mileageRouteItems().front().city1());
  Loc* multiTransportOrigin(mileageRoute.mileageRouteItems().front().multiTransportOrigin());
  MileageRouteItems::iterator itr(mileageRoute.mileageRouteItems().begin());
  MileageRouteItems::const_iterator beg(itr);
  MileageRouteItems::const_iterator end(mileageRoute.mileageRouteItems().end());
  bool qualifiesForGDTable(false);
  // set origin of each city pair to global origin,
  // retrieve MPM for each city pair and
  // copy the result into original mileageRoute
  for (; itr != end; ++itr)
  {
    bool rc(false);
    MileageRouteItem& item(*itr);
    if (itr == beg)
    {
      item.mpmGlobalDirection() = item.tpmGlobalDirection();
      rc = true;
    }
    else if (qualifiesForGDTable || item.city1()->area() == item.city2()->area())
    {
      MileageRouteItems::const_iterator prv(itr);
      --prv;
      item.mpmGlobalDirection() = prv->mpmGlobalDirection();
      rc = true;
    }
    else if (isGDPossible(beg, itr, qualifiesForGDTable))
    {
      item.mpmGlobalDirection() = item.tpmGlobalDirection();
      rc = true;
    }
    if (rc)
    {
      // copy global origin to current segment's for MPM retrieval purpose
      Loc* loc(item.city1());
      Loc* mloc(item.multiTransportOrigin());
      item.city1() = origin;
      item.multiTransportOrigin() = multiTransportOrigin;

      tempRoute.mileageRouteItems().push_back(item);
      tempRoute.mileageRouteItems().back().city1() = loc;
      if (qualifiesForGDTable || !(rc = getMPM(item, dataHandle)))
      {
        if (gdRetriever.retrieve(tempRoute.mileageRouteItems(), dataHandle))
        {
          item.mpmGlobalDirection() = tempRoute.mileageRouteItems().back().mpmGlobalDirection();
          rc = getMPM(item, dataHandle);
        }
      }
      // restore original segment's origin
      item.city1() = loc;
      item.multiTransportOrigin() = mloc;
    }
    if (rc)
    {
      // set tempRoute globalDirection to current segment's
      tempRoute.globalDirection() = item.mpmGlobalDirection();

      if (getPSR(tempRoute))
      {
        item.psrApplies() = true;
      }
      else if (tempRoute.psrMayApply())
      {
        item.psrMayApply() = true;
        tempRoute.psrMayApply() = false;
      }
    }
  }

  mileageRoute.mileageRouteMPM() = mileageRoute.mileageRouteItems().back().mpm();
  return true;
}

template <GlobalDirection gd>
struct EqualsByGD : public std::unary_function<MileageRouteItem, bool>
{
  bool operator()(const MileageRouteItem& item) const { return item.tpmGlobalDirection() == gd; }
};

class AreaCollector
{
public:
  AreaCollector(const std::set<std::string>& initial) : _uniqueAreas(initial) {}
  void operator()(const MileageRouteItem& item)
  {
    _uniqueAreas.insert(item.city2()->area());
    for (const Loc* loc : item.hiddenLocs())
    {
      _uniqueAreas.insert(loc->area());
    }
  }
  operator std::set<std::string>() const { return _uniqueAreas; }

private:
  std::set<std::string> _uniqueAreas;
};

class GDTableQualifier
{
public:
  GDTableQualifier() : _pa(false), _northAmerica(false), _begin(true) {}
  void operator()(const MileageRouteItem& item)
  {
    if (!_pa && item.tpmGlobalDirection() == GlobalDirection::PA)
      _pa = true;
    if (!_begin && !_northAmerica && item.city1()->subarea() == NorthAmerica)
      _northAmerica = true;
    if (_begin)
      _begin = false;
  }
  operator bool() const { return _pa && _northAmerica; }

private:
  bool _pa, _northAmerica, _begin;
};

//@pre { end is a valid random access iterator pointing to an existing MileageRouteItem }
bool
MPMCollectorWN::isGDPossible(MileageRouteItems::const_iterator beg,
                             MileageRouteItems::const_iterator end,
                             bool& qualifiesForGDTable) const
{
  bool result(true);
  const std::string& origArea(beg->city1()->area());
  const std::string& destArea(end->city2()->area());
  MileageRouteItems::const_iterator postEnd(end + 1);
  if ((origArea == IATA_AREA1 && (destArea == IATA_AREA2 || destArea == IATA_AREA3)) ||
      (destArea == IATA_AREA1 && (origArea == IATA_AREA2 || origArea == IATA_AREA3)))
  { // if there is AP TPM global between (area 2 or area 3) and area 1, no MPM global is possible
    if ((result = std::find_if(beg, postEnd, EqualsByGD<GlobalDirection::AP>()) == postEnd) &&
        !qualifiesForGDTable)
    { // if travel between South America and South Pacific includes PA TPM global,
      // GlobaDirection table needs to be consulted for MPM global determination
      const Loc* area1loc = origArea == IATA_AREA1 ? beg->city1() : end->city2();
      const Loc* area3loc =
          origArea == IATA_AREA3 ? beg->city1() : destArea == IATA_AREA3 ? end->city2() : nullptr;
      if (area3loc != nullptr && isSouthAmerica(area1loc) && isSouthPacific(area3loc))
        qualifiesForGDTable = std::for_each(beg, postEnd, GDTableQualifier());
    }
  }
  else if ((origArea == IATA_AREA2 && destArea == IATA_AREA3) ||
           (origArea == IATA_AREA3 && destArea == IATA_AREA2))
    // if there is more than 1 AT TPM global between area 2 and area 3, no MPM global is possible
    result = std::count_if(beg, postEnd, EqualsByGD<GlobalDirection::AT>()) <= 1;
  else if (origArea == destArea)
  { // if travel traverses all areas and returns to the original, no MPM global is possible
    std::set<std::string> uniqueAreas;
    uniqueAreas.insert(origArea);
    uniqueAreas = std::for_each(beg, postEnd, AreaCollector(uniqueAreas));
    result = uniqueAreas.size() < 3;
  }
  return result;
}

bool
MPMCollectorWN::isSouthAmerica(const Loc* loc) const
{
  const NationCode* end(southAmericaCountries + southAmericaCountriesNo);
  return std::find_if(southAmericaCountries,
                      end,
                      std::bind2nd(std::equal_to<NationCode>(), loc->nation())) != end;
}

bool
MPMCollectorWN::isSouthPacific(const Loc* loc) const
{
  return loc->subarea() == SouthPacific;
}

bool
MPMCollectorWN::getMPM(MileageRouteItem& item, DataHandle& dataHandle) const
{
  const Retriever<MPMRetriever>& mpmRetriever(tse::Singleton<Retriever<MPMRetriever> >::instance());
  return mpmRetriever.retrieve(item, dataHandle);
}

bool
MPMCollectorWN::getPSR(MileageRoute& route) const
{
  const MileageExclusion& psr(tse::Singleton<PermissibleSpecifiedRouting>::instance());
  return psr.apply(route);
}

}
