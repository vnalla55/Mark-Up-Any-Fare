#include "Routing/TicketedPointDeduction.h"

#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "Common/Vendor.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TpdPsr.h"
#include "Routing/FareCalcVisitor.h"
#include "Routing/IsInLoc.h"
#include "Routing/MileageRoute.h"

namespace tse
{
static Logger
logger("atseintl.Routing.TicketedPointDeduction");

/**
 * Unary predicate returning true if given TpdPsr record has globalDir NOT equal to
 * stored _globalDir NOR blank (which means applicable to anything)
 */
class GDNotEqualNorBlank : public std::unary_function<TpdPsr, bool>
{
public:
  GDNotEqualNorBlank(GlobalDirection globalDir) : _globalDir(globalDir) {}
  bool operator()(const TpdPsr* tpdPsr) const
  {
    return tpdPsr->globalDir() != _globalDir && tpdPsr->globalDir() != GlobalDirection::ZZ;
  }

private:
  const GlobalDirection _globalDir;

  // lint --e{1509}
};

// not needed as we do not filter by fareTypeAppl
// left commented out in case requirements change
/*
class FareTypeApplNotEqualNorBlank: public std::unary_function<TpdPsr, bool>
{
public:
   FareTypeApplNotEqualNorBlank(Indicator fareTypeAppl): _fareTypeAppl(fareTypeAppl) {}
   bool operator()(const TpdPsr *tpdPsr) const
   {
      return tpdPsr->fareTypeAppl() != ' ' && tpdPsr->fareTypeAppl() != _fareTypeAppl;
   }
private:
   const Indicator _fareTypeAppl;
};
*/

bool
TicketedPointDeduction::apply(MileageRoute& mileageRoute) const
{
  return apply(mileageRoute, nullptr);
}

bool
TicketedPointDeduction::apply(MileageRoute& mileageRoute, FareCalcVisitor* visitor) const
{
  LOG4CXX_DEBUG(logger, "Entered TicketedPointDeduction::apply()");
  if (UNLIKELY(mileageRoute.mileageRouteItems().empty()))
  {
    LOG4CXX_DEBUG(logger, "Leaving TicketedPointDeduction::apply(): not applied");
    return false;
  }
  MileageRouteItem& board(mileageRoute.mileageRouteItems().front());
  MileageRouteItem& off(mileageRoute.mileageRouteItems().back());
  // retrieve all TpdPsr records matching given criteria
  const std::vector<TpdPsr*>& allGDTpdList = getData(*mileageRoute.dataHandle(),
                                                     TPD,
                                                     mileageRoute.governingCarrier(),
                                                     board.city1()->area()[0],
                                                     off.city2()->area()[0],
                                                     board.travelDate());
  // this will store the final vector of TpdPsr records to process
  std::vector<TpdPsr*> tpdList;
  // if the route global direction is well defined, filter the record vector
  // to have only the records with matching globalDir or blank
  if (LIKELY(mileageRoute.globalDirection() != GlobalDirection::ZZ))
  {
    std::remove_copy_if(allGDTpdList.begin(),
                        allGDTpdList.end(),
                        std::back_inserter(tpdList),
                        GDNotEqualNorBlank(mileageRoute.globalDirection()));
  }
  else
  {
    tpdList = allGDTpdList;
  }
  // not needed - we do not filter by fareTypeAppl
  // tpdList.remove_if(FareTypeApplNotEqualNorBlank(mileageRoute.fareTypeAppl()));
  if (tpdList.empty())
  {
    LOG4CXX_DEBUG(logger, "Leaving TicketedPointDeduction::apply(): not applied");
    return false;
  }
  const CarrierCode& governingCarrier(mileageRoute.governingCarrier());
  // try to match the records by all matching criteria
  // if successful, the route destination contains highest TPD from all matched records -
  // apply that as the route TPD
  //
  //@note This is enough for WP. WN will pass incremental routes, thus collecting
  //	   TPDs for all items.
  if (processSubRoute(mileageRoute.mileageRouteItems(),
                      tpdList,
                      governingCarrier,
                      *mileageRoute.dataHandle(),
                      visitor))
  {

    // lint --e{530}
    MileageRouteItem& back(mileageRoute.mileageRouteItems().back());
    mileageRoute.tpd() = back.tpd();
    LOG4CXX_DEBUG(logger, "Leaving TicketedPointDeduction::apply(): applied");
    return true;
  }
  // TPD not applied
  LOG4CXX_DEBUG(logger, "Leaving TicketedPointDeduction::apply(): not applied");
  return false;
}

/**
 * Method separating data retrieval to make the class testable independently of DataHandle.
 */
const std::vector<TpdPsr*>&
TicketedPointDeduction::getData(DataHandle& dataHandle,
                                Indicator application,
                                const CarrierCode& governingCarrier,
                                Indicator area1,
                                Indicator area2,
                                const DateTime& travelDate) const
{
  CarrierCode carrier = governingCarrier;
  if (carrier == INDUSTRY_CARRIER)
  {
    carrier = "";
  }
  return dataHandle.getTpdPsr(application, carrier, area1, area2, travelDate);
}

/**
 * Main method matching the route through all retrieved TpdPsr records by all criteria
 * For Jal/Axess TPD project
 */
bool
TicketedPointDeduction::processSubRoute(MileageRouteItems& subRoute,
                                        const std::vector<TpdPsr*>& tpdList,
                                        const CarrierCode& governingCarrier,
                                        DataHandle& dataHandle,
                                        FareCalcVisitor* visitor) const
{
  // if there are no records to process, return "none applied"
  if (UNLIKELY(tpdList.empty()))
    return false;

  uint16_t grandTPD(0);
  FareCalcMatching matchedViaGeoLocs;
  MatchedTPD matchedTPDsegs;

  const MileageRouteItem& firstItem = subRoute.front();
  const MileageRouteItem& lastItem = subRoute.back();

  // loop through all TpdPsr records trying to match them with the route
  // highest matched TPD apply to route destination, i.e. whole route
  std::vector<TpdPsr*>::const_iterator itr(tpdList.begin());
  std::vector<TpdPsr*>::const_iterator end(tpdList.end());

  for (; itr != end; ++itr)
  {
    TpdPsr& tpd(**itr);

    const bool origLoc1 =
        (firstItem.isOriginInLoc(tpd.loc1()) && lastItem.isDestinationInLoc(tpd.loc2()));
    if (!origLoc1)
    {
      const bool origLoc2 =
          (firstItem.isOriginInLoc(tpd.loc2()) && lastItem.isDestinationInLoc(tpd.loc1()));

      if (!origLoc2)
        continue;
    }

    if (visitor != nullptr)
    {
      visitor->resetMatching();
      visitor->resetMatchedTPD(); // TPD project
    }

    // match via geo locations or discard record
    if (!processViaGeoLocs(subRoute, tpd, dataHandle, visitor, origLoc1))
      continue;

    // match Thru Via Market Only
    if (!processThruViaMktOnly(subRoute, tpd))
      continue;

    // match thru market carriers or discard record
    if (!processThruMktCxrs(subRoute, tpd, governingCarrier))
      continue;

    // match via carrier locations or discard record
    if (!processViaCxrLocs(subRoute, tpd, governingCarrier))
      continue;

    // match via exceptions or discard record
    if (!processViaExcepts(subRoute, tpd, governingCarrier))
      continue;

    // if all matched and TPD is higher than highest so far, store that TPD
    if (tpd.tpmDeduction() > grandTPD)
    {
      grandTPD = tpd.tpmDeduction();
      if (visitor != nullptr)
      {
        matchedViaGeoLocs = visitor->getMatching();
        matchedTPDsegs = visitor->getMatchedTPD();
      }
    }
  }
  // if any TpdPsr record passed all matching, apply highest TPD to route destination
  if (grandTPD > 0)
  {
    subRoute.back().tpd() = grandTPD;
    if (visitor != nullptr)
    {
      visitor->resetMatching(matchedViaGeoLocs);
      visitor->resetMatchedTPD(matchedTPDsegs);
    }
    return true;
  }
  // none TpdPsr record matched all criteria
  return false;
}

/**
 * Binary predicate returning true if viaGeoLocs passed as arguments have equal setNo.
 */
struct EqualBySetNo
{
  bool operator()(const TpdPsrViaGeoLoc* l, const TpdPsrViaGeoLoc* r) const
  {
    return l->setNo() == r->setNo();
  }
} equalBySetNo;

/**
 * Binary predicate ordering viaGeoLocs only by setNo.
 */
struct LessBySetNo
{
  bool operator()(const TpdPsrViaGeoLoc* l, const TpdPsrViaGeoLoc* r) const
  {
    return l->setNo() < r->setNo();
  }
} lessBySetNo;

/**
 * Unary predicate returning true if given viaGeoLoc has loc equal to the stored _loc.
 */
class EqualLocCode
{
public:
  EqualLocCode(const LocCode& loc) : _loc(loc) {}
  bool operator()(const TpdPsrViaGeoLoc* l) const { return l->loc().loc() == _loc; }

private:
  const LocCode& _loc;
};

bool
TicketedPointDeduction::processViaGeoLocs(MileageRouteItems& subRoute,
                                          TpdPsr& tpd,
                                          DataHandle& dataHandle,
                                          FareCalcVisitor* visitor,
                                          bool origLoc1) const
{
  if (tpd.viaGeoLocs().empty())
    return true;

  // create a vector of viaGeoLocs consisting only of first locs in their sets
  std::vector<TpdPsrViaGeoLoc*> uniqueGeoLocs;
  std::unique_copy(tpd.viaGeoLocs().begin(),
                   tpd.viaGeoLocs().end(),
                   std::back_inserter(uniqueGeoLocs),
                   equalBySetNo);
  std::vector<TpdPsrViaGeoLoc*>::const_iterator itr(uniqueGeoLocs.begin());
  std::vector<TpdPsrViaGeoLoc*>::const_iterator end(uniqueGeoLocs.end());
  std::vector<const TpdPsrViaGeoLoc*>& locs(subRoute.back().condTpdViaGeoLocs());

  bool geoLocFound(false);

  MileageRouteItems::const_iterator ii;
  // iterate through first viaGeoLocs in sets to process each set separately
  // sets are combined with OR, i.e. match found (geoLocFound variable) in any set makes the record
  // pass
  for (; itr != end && !geoLocFound; ++itr)
  {
    const TpdPsrViaGeoLoc* geoLoc(*itr);

    // determine bounderies of current set in viaGeoLoc vector
    std::pair<std::vector<TpdPsrViaGeoLoc*>::const_iterator,
              std::vector<TpdPsrViaGeoLoc*>::const_iterator> sameSetNo =
        std::equal_range(tpd.viaGeoLocs().begin(), tpd.viaGeoLocs().end(), geoLoc, lessBySetNo);
    std::vector<TpdPsrViaGeoLoc*>::const_iterator i(sameSetNo.first);
    bool localFound(false); // current viaGeoLoc found on the route

    // iterate through viaGeoLocs belonging to current set
    for (; i != sameSetNo.second; ++i)
    {
      const TpdPsrViaGeoLoc& loc(**i);
      bool cond(false);

      // at least one viaGeoLoc must be found in the route
      if (loc.relationalInd() == VIAGEOLOCREL_ANDOR)
      {
        // if previous viaGeoLoc was found in the route, we are done
        if (localFound)
          break;
        // in all other cases we need to examine viaGeoLoc vector further
        ii = std::find_if(subRoute.begin(), subRoute.end() - 1, IsInLoc(loc, cond, origLoc1));
        localFound = ii != subRoute.end() - 1;
        // store matched via geo locs fo fare calc
        if (localFound && visitor != nullptr)
        {
          visitor->recordMatch(*ii, loc);
          visitor->recordMatchedTPD((*ii).pnrSegment()); // TPD project
        }
        // if match not found only because of stopover and this loc was not yet stored as
        // potentially matching, record it so
        if (cond &&
            std::find_if(locs.begin(), locs.end(), EqualLocCode(loc.loc().loc())) == locs.end())
          locs.push_back(&loc);
      }
      // exactly one viaGeoLoc must be found in the route
      else if (loc.relationalInd() == VIAGEOLOCREL_OR)
      {
        if (localFound)
          break;

        // in all other cases we need to examine viaGeoLoc vector further
        ii = std::find_if(subRoute.begin(), subRoute.end() - 1, IsInLoc(loc, cond, origLoc1));
        localFound = (ii != subRoute.end() - 1);
        // store matched via geo locs fo fare calc
        if (localFound && visitor != nullptr)
        {
          visitor->recordMatch(*ii, loc);
          visitor->recordMatchedTPD((*ii).pnrSegment()); // TPD project
        }
        // if match not found only because of stopover and this loc was not yet stored as
        // potentially matching, record it so
        if (!localFound && cond &&
            std::find_if(locs.begin(), locs.end(), EqualLocCode(loc.loc().loc())) == locs.end())
          locs.push_back(&loc);
      }
      // all viaGeoLocs in this set must appear in the route
      else if (loc.relationalInd() == VIAGEOLOCREL_AND)
      {
        // since we are not in the first viaGeoLoc (logical operator is present) and the previous
        // one
        // was not found, then this conjunction of viaGeoLocs is false
        if (!localFound)
          continue;
        // if the current viaGeoLoc is not found, this conjunction is also false
        ii = std::find_if(subRoute.begin(), subRoute.end() - 1, IsInLoc(loc, cond, origLoc1));
        localFound = ii != subRoute.end() - 1;
        if (!localFound)
        {
          // if match not found only because of stopover and this loc was not yet stored as
          // potentially matching, record it so
          if (cond &&
              std::find_if(locs.begin(), locs.end(), EqualLocCode(loc.loc().loc())) == locs.end())
            locs.push_back(&loc);
          continue;
        }
        // store matched via geo locs fo fare calc
        if (localFound && visitor != nullptr)
        {
          visitor->recordMatch(*ii, loc);
          visitor->recordMatchedTPD((*ii).pnrSegment()); // TPD project
        }
        // in any case we need to proceed to evaluate the vector further
      }
      // if there is no operator then we are at the first viaGeoLoc in set
      // therefore we just store current matching result and go on because
      // we can tell nothing about global matching yet
      else
      {
        ii = std::find_if(subRoute.begin(), subRoute.end() - 1, IsInLoc(loc, cond, origLoc1));
        localFound = ii != subRoute.end() - 1;
        // store matched via geo locs fo fare calc
        if (localFound && visitor != nullptr)
        {
          visitor->recordMatch(*ii, loc);
          visitor->recordMatchedTPD((*ii).pnrSegment()); // TPD project
        }
        // if match not found only because of stopover and this loc was not yet stored as
        // potentially matching, record it so
        else if (UNLIKELY(cond && std::find_if(locs.begin(), locs.end(), EqualLocCode(loc.loc().loc())) ==
                             locs.end()))
        {
          TpdPsrViaGeoLoc* locCopy = nullptr;

          // lint --e{413}
          dataHandle.get(locCopy);
          *locCopy = loc;
          // since this viaGeoLoc has no logical operator (first in set) and separate sets are
          // combinde with OR, then set this operator to OR for reporting purpose
          locCopy->relationalInd() = VIAGEOLOCREL_OR;
          locs.push_back(locCopy);
        }
      }
    }

    geoLocFound = localFound;
  }

  return geoLocFound;
}

class IsOffViaLoc
{
public:
  IsOffViaLoc(const MileageRouteItem& item) : _routeItem(item) {}

  bool operator()(TpdPsrViaGeoLoc* viaGeoLoc) const
  {
    return _routeItem.isDestinationInLoc(viaGeoLoc->loc());
  }

private:
  const MileageRouteItem& _routeItem;
};

bool
TicketedPointDeduction::processThruViaMktOnly(const MileageRouteItems& subRoute, const TpdPsr& tpd)
    const
{
  if ((tpd.tpdThruViaMktOnlyInd() != THRUVIAMKTONLY_YES) || tpd.viaGeoLocs().empty() ||
      (subRoute.size() < 2))
    return true;

  MileageRouteItems::const_iterator routeItr(subRoute.begin());
  MileageRouteItems::const_iterator routeEnd(subRoute.end() - 1);
  for (; routeItr != routeEnd; ++routeItr)
  {
    if (std::find_if(tpd.viaGeoLocs().begin(), tpd.viaGeoLocs().end(), IsOffViaLoc(*routeItr)) ==
        tpd.viaGeoLocs().end())
      return false;
  }

  return true;
}

bool
TicketedPointDeduction::processThruMktCxrs(const MileageRouteItems& subRoute,
                                           const TpdPsr& tpd,
                                           const CarrierCode& governingCarrier) const
{
  // we need to check this condition only if the carrier of TpdPsr record is empty
  // otherwise the carrier field would be already matched with governing carrier of the route
  if (!tpd.thruMktCxrs().empty() && tpd.carrier().empty())
  {
    bool found = std::find(tpd.thruMktCxrs().begin(), tpd.thruMktCxrs().end(), governingCarrier) !=
                 tpd.thruMktCxrs().end();
    // if result whether governing carrier was found on the vector of carriers or not, does not
    // conform to
    // whether it should be there (thruMktCarrierExcept indicator), the record fails
    if (found != (tpd.thruMktCarrierExcept() == THRUMKTCXREXCEPT_NO))
    {
      return false;
    }
  }
  // if the accordance exists, check if the whole route must be via the same carrier
  if (tpd.thruViaMktSameCxr() == THRUVIAMKTSAMECXR_YES ||
      (!tpd.carrier().empty() && tpd.thisCarrierRestr() == 'Y'))
  {
    MileageRouteItems::const_iterator itr(subRoute.begin());
    MileageRouteItems::const_iterator end(subRoute.end());
    for (; itr != end; ++itr)
    {
      if (itr->isSurface())
        continue;

      if (governingCarrier == INDUSTRY_CARRIER)
      {
        if (!itr->segmentCarrier().empty() && itr->segmentCarrier() != governingCarrier)
        {
          return false;
        }
      }
      // if the whole route must be via the same carrier, and another carrier is found in the route,
      // fail this record
      else if (itr->segmentCarrier() != governingCarrier)
      {
        return false;
      }
    }
  }
  return true;
}

/**
 * Unary predicate returning true if origin of given item is in the stored location
 */
class IsBoardInLocs
{
public:
  IsBoardInLocs(const LocKey& loc1, const LocKey& loc2, bool& first)
    : _loc1(loc1), _loc2(loc2), _first(first)
  {
  }
  bool operator()(const MileageRouteItem& item) const
  {
    _first = item.isOriginInLoc(_loc1);
    return _first || item.isOriginInLoc(_loc2);
  }

private:
  const LocKey& _loc1, _loc2;
  bool& _first;
};

/**
 * Unary predicate returning true if destination of given item is in the stored location
 */
class IsOffInLoc
{
public:
  IsOffInLoc(const LocKey& loc) : _loc(loc) {}
  bool operator()(const MileageRouteItem& item) const { return item.isDestinationInLoc(_loc); }

private:
  const LocKey& _loc;
};

/**
 * Unary predicate returning true if given item is via stored carrier
 */
class IsViaCxr : public std::unary_function<MileageRouteItem, bool>
{
public:
  IsViaCxr(const CarrierCode& cxr, const CarrierCode& govCxr)
    : _cxr(cxr), _cxrSet(nullptr), _govCxr(govCxr)
  {
  }
  IsViaCxr(const std::set<CarrierCode>& cxrSet, const CarrierCode& govCxr)
    : _cxr(""), _cxrSet(&cxrSet), _govCxr(govCxr)
  {
  }
  bool operator()(const MileageRouteItem& item) const
  {
    return ((item.isSurface() &&
             ((_govCxr == _cxr) ||
              ((_cxrSet != nullptr) && (_cxrSet->find(_govCxr) != _cxrSet->end())))) || // For surface,
                                                                                  // use governing
                                                                                  // carrier to
                                                                                  // compare
            ((item.segmentCarrier() == _cxr) ||
             ((_cxrSet != nullptr) && (_cxrSet->find(_govCxr) != _cxrSet->end())))); // For Non-surface,
                                                                               // use real carrier
  }

private:
  const CarrierCode& _cxr;
  const std::set<CarrierCode>* _cxrSet;
  const CarrierCode& _govCxr;

  // lint --e{1509}
};

bool
TicketedPointDeduction::processViaCxrLocs(const MileageRouteItems& subRoute,
                                          const TpdPsr& tpd,
                                          const CarrierCode& govCxr) const
{
  // if there is no condition, the record passes
  if (tpd.viaCxrLocs().empty())
    return true;
  std::vector<TpdPsrViaCxrLoc*>::const_iterator itr(tpd.viaCxrLocs().begin());
  std::vector<TpdPsrViaCxrLoc*>::const_iterator end(tpd.viaCxrLocs().end());

  std::set<CarrierCode> viaCxrSet;

  bool matchedLocs = false;
  bool matchedLocsViaCxr = false;

  for (; (itr != end) && !matchedLocsViaCxr; ++itr)
  {
    const TpdPsrViaCxrLoc& cxrLoc(**itr);

    if (cxrLoc.loc1().loc().empty() || // No Loc1 and Loc2 specified, only carrier
        ((cxrLoc.loc1().locType() == tpd.loc1().locType()) && // Get around content GUI restriction
         (cxrLoc.loc1().loc() == tpd.loc1().loc()) &&
         (cxrLoc.loc2().locType() == tpd.loc2().locType()) &&
         (cxrLoc.loc2().loc() == tpd.loc2().loc())))
    {
      // This item is for all travel via Cxr
      viaCxrSet.insert(cxrLoc.viaCarrier());
      continue;
    }

    // The following check is for item that has different Loc1 and Loc2 than the through Loc1 and
    // Loc2

    bool firstLoc(false);
    // find the beginning of the part of route of interest
    MileageRouteItems::const_iterator first = std::find_if(
        subRoute.begin(), subRoute.end(), IsBoardInLocs(cxrLoc.loc1(), cxrLoc.loc2(), firstLoc));
    // if neither loc exists in the route, proceed to next viaCxrLoc
    if (first == subRoute.end())
      continue;
    // find the end of the part of route of interest
    MileageRouteItems::const_reverse_iterator last;
    if (firstLoc)
      last = std::find_if(subRoute.rbegin(), subRoute.rend(), IsOffInLoc(cxrLoc.loc2()));
    else
      last = std::find_if(subRoute.rbegin(), subRoute.rend(), IsOffInLoc(cxrLoc.loc1()));

    // if the second loc does not exist in the route, proceed to next viaCxrLoc
    if (last == subRoute.rend())
      continue;

    matchedLocs = true;

    // if in the part of route of interest there appears no carrier different from the one specified
    if (std::find_if(first, last.base(), std::not1(IsViaCxr(cxrLoc.viaCarrier(), govCxr))) ==
        last.base())
      matchedLocsViaCxr = true;
  }

  if (matchedLocs && !matchedLocsViaCxr) // Matched an item by locs but failed match Cxr
    return false;

  // Check all travel via carrier
  if (!viaCxrSet.empty())
  {
    if (std::find_if(subRoute.begin(), subRoute.end(), std::not1(IsViaCxr(viaCxrSet, govCxr))) !=
        subRoute.end())
      return false;
  }

  return true;
}

bool
TicketedPointDeduction::processViaExcepts(const MileageRouteItems& subRoute,
                                          const TpdPsr& tpd,
                                          const CarrierCode& govCxr) const
{
  // if there is no condition, the record passes
  if (tpd.viaExcepts().empty())
    return true;
  std::vector<TpdPsrViaExcept*>::const_iterator itr(tpd.viaExcepts().begin());
  std::vector<TpdPsrViaExcept*>::const_iterator end(tpd.viaExcepts().end());
  for (; itr != end; ++itr)
  {
    const TpdPsrViaExcept& exc(**itr);
    bool firstLoc(false);
    // find the beginning of the part of route of interest
    MileageRouteItems::const_iterator first = std::find_if(
        subRoute.begin(), subRoute.end(), IsBoardInLocs(exc.loc1(), exc.loc2(), firstLoc));
    // if neither loc exists in the route, proceed to next viaCxrLoc
    if (first == subRoute.end())
      continue;
    // find the end of the part of route of interest
    MileageRouteItems::const_reverse_iterator last;
    if (firstLoc)
      last = std::find_if(subRoute.rbegin(), subRoute.rend(), IsOffInLoc(exc.loc2()));
    else
      last = std::find_if(subRoute.rbegin(), subRoute.rend(), IsOffInLoc(exc.loc1()));
    // if the second loc does not exist in the route, proceed to next viaCxrLoc
    if (last == subRoute.rend())
      continue;
    // if in the part of route of interest specified carrier appears, record fails
    if (std::find_if(first, last.base(), IsViaCxr(exc.viaCarrier(), govCxr)) != last.base())
    {
      return false;
    }
  }
  return true;
}
}
