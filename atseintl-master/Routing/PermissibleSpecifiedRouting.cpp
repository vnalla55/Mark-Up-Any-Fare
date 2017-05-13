#include "Routing/PermissibleSpecifiedRouting.h"

#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/Vendor.h"
#include "DBAccess/Loc.h"
#include "Routing/MileageRoute.h"
#include "Routing/MileageRouteItem.h"
#include "Routing/PSRRetriever.h"
#include "Routing/RoutingConsts.h"

#include <set>
#include <vector>

namespace tse
{
namespace
{
Logger
_logger("atseintl.Routing.PermissibleSpecifiedRouting");
}

bool
PermissibleSpecifiedRouting::apply(MileageRoute& mileageRoute) const
{
  LOG4CXX_DEBUG(_logger, "Entered PermissibleSpecifiedRouting::apply()");

  if (mileageRoute.mileageRouteItems().empty() || mileageRoute.mileageRouteItems().size() <= 1)
  {
    return false;
  }

  PSRRetriever& psrRetriever(tse::Singleton<PSRRetriever>::instance());
  const std::vector<TpdPsr*> psrList = psrRetriever.getpsrData(mileageRoute);

  if (UNLIKELY(psrList.empty()))
  {
    return false;
  }

  if (validate(mileageRoute, psrList))
  {
    return true;
  }

  //----------------------------------------------
  // Reverse Itinerary and validate PSRList again
  //----------------------------------------------
  MileageRoute reverseMileageRoute;
  reverseRoute(mileageRoute, reverseMileageRoute);

  if (validate(reverseMileageRoute, psrList))
  {
    mileageRoute.applicablePSR() = reverseMileageRoute.applicablePSR();
    mileageRoute.psrSetNumber() = reverseMileageRoute.psrSetNumber();
    mileageRoute.hipExempt() = reverseMileageRoute.hipExempt();

    mileageRoute.psrMayApply() |= reverseMileageRoute.psrMayApply();

    return true;
  }
  else
  {
    mileageRoute.psrMayApply() |= reverseMileageRoute.psrMayApply();
  }

  LOG4CXX_DEBUG(_logger, "Leaving PermissibleSpecifiedRouting::apply()");
  return false;
}

//------------------------------------------------------------------------
//  Validate the PSR items
//------------------------------------------------------------------------
bool
PermissibleSpecifiedRouting::validate(MileageRoute& mileageRoute,
                                      const std::vector<TpdPsr*>& psrList) const
{
  const MileageRouteItem& firstItem = mileageRoute.mileageRouteItems().front();
  const MileageRouteItem& lastItem = mileageRoute.mileageRouteItems().back();

  std::vector<TpdPsr*>::const_iterator itr(psrList.begin());
  std::vector<TpdPsr*>::const_iterator end(psrList.end());
  for (; itr != end; ++itr)
  {
    TpdPsr& psr(**itr);

    if (!firstItem.isOriginInLoc(psr.loc1()) || !lastItem.isDestinationInLoc(psr.loc2()))
      continue;

    if (psr.globalDir() != GlobalDirection::ZZ && psr.globalDir() != mileageRoute.globalDirection())
    {
      continue;
    }

    if (!processThruMktCxrs(mileageRoute, psr))
    {
      continue;
    }

    if (!processViaMktSameCarrier(mileageRoute, psr))
    {
      continue;
    }

    if (!processViaCxrLocs(mileageRoute, psr))
    {
      continue;
    }

    if (!processViaCxrLocExceptions(mileageRoute, psr))
    {
      continue;
    }

    if (!processFareType(mileageRoute, psr))
    {
      continue;
    }

    if (!processViaGeoLocs(mileageRoute, psr))
    {
      continue;
    }

    if (!processGeoLocStopoverConditions(mileageRoute, psr))
    {
      continue;
    }

    if (psr.psrHip() == YES)
    {
      mileageRoute.hipExempt() = true;
    }

    return true;
  }

  return false;
}

//----------------------------------------------------------------------------
//  PermissibleSpecifiedRouting::processViaGeoLocs()
//----------------------------------------------------------------------------
bool
PermissibleSpecifiedRouting::processViaGeoLocs(MileageRoute& mileageRoute, TpdPsr& psr) const
{

  std::vector<TpdPsrViaGeoLoc*>::const_iterator geoLocItr = psr.viaGeoLocs().begin();
  std::vector<TpdPsrViaGeoLoc*>::const_iterator geoLocEnd = psr.viaGeoLocs().end();

  while (geoLocItr != geoLocEnd)
  {

    if (validateGeoLocSet(psr, mileageRoute, geoLocItr, geoLocEnd))
    {
      return true;
    }
  }

  return false; // no geoLocSets passed
}

//-----------------------------------------------------------
// Validate geoLocs within a Set
//-----------------------------------------------------------
bool
PermissibleSpecifiedRouting::validateGeoLocSet(
    TpdPsr& psr,
    MileageRoute& mileageRoute,
    std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocItr,
    const std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocEnd) const
{

  MileageRouteItems::iterator mItr(mileageRoute.mileageRouteItems().begin());
  MileageRouteItems::iterator mEnd(mileageRoute.mileageRouteItems().end() - 1);

  int32_t setNumber = (*geoLocItr)->setNo();

  Indicator groupRelationalInd = BLANK;

  for (; mItr != mEnd; ++mItr)
  {
    if (!matchGeoLoc(mileageRoute, *mItr, geoLocItr, geoLocEnd, groupRelationalInd, setNumber))
    {
      return false;
    }
  }

  //---------------------------------------------------------------------------------------
  // All itin items have been validated.  Make sure there are no other required points to
  // validate on the psr
  //---------------------------------------------------------------------------------------
  if (checkGeoLocs(mileageRoute, setNumber, geoLocItr, geoLocEnd))
  {
    mileageRoute.applicablePSR() = &psr;
    mileageRoute.psrSetNumber() = setNumber;
    return true;
  }

  return false;
}

//---------------------------------------------------------------------------------
//  MatchGeoLoc
//  Match the mileageRouteItem to the psr geo location.
//
//  Return only when the mileageRouteItem is matched or the geoLoc Set has failed.
//---------------------------------------------------------------------------------
bool
PermissibleSpecifiedRouting::matchGeoLoc(
    MileageRoute& mileageRoute,
    MileageRouteItem& mRouteItem,
    std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocItr,
    const std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocEnd,
    Indicator& groupRelationalInd,
    int32_t setNumber) const
{
  while (geoLocItr != geoLocEnd && setNumber == (*geoLocItr)->setNo())
  {
    if (checkLoc(mileageRoute, mRouteItem, setNumber, geoLocItr, geoLocEnd, groupRelationalInd))
    {
      return true;
    }

    else
    {
      if (!setGeoLocIterator(mileageRoute, geoLocItr, geoLocEnd, groupRelationalInd, setNumber))
      {
        return false;
      }
    }
  }

  return false;
}

//--------------------------------------------------------------------------------------------
// setGeoLocIterator
//
// Set the geoLocIterator when the location in the mileageRouteItem was not matched.
//--------------------------------------------------------------------------------------------

bool
PermissibleSpecifiedRouting::setGeoLocIterator(
    MileageRoute& mileageRoute,
    std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocItr,
    const std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocEnd,
    Indicator& groupRelationalInd,
    int32_t setNumber) const
{
  //-------------------------------------------------------------------
  // Fail if this is the last geoLoc in the last set
  //-------------------------------------------------------------------

  ++geoLocItr;
  if (geoLocItr == geoLocEnd || setNumber != (*geoLocItr)->setNo())
  {
    return false;
  }

  bool advanceToNextSet = false;

  switch (groupRelationalInd)
  {
  case BLANK:

    //---------------------------------------------------------------
    // The relational indicator is BLANK
    // (First group of geoLoc items in Set)
    //---------------------------------------------------------------

    advanceToNextSet = (*geoLocItr)->relationalInd() == VIAGEOLOCREL_AND;

    break;

  case VIAGEOLOCREL_AND:

    //---------------------------------------------------------------
    // The relational Indicator is AND
    //
    // Fail the AND geoLoc (it's required).
    // Unless ......it's followed by an OR grouping:  DUB-VIE/LON/SNN
    //---------------------------------------------------------------

    advanceToNextSet = (*geoLocItr)->relationalInd() == BLANK ||
                       (*geoLocItr)->relationalInd() == VIAGEOLOCREL_ANDOR;

    break;

  case VIAGEOLOCREL_ANDOR:
    break;

  default:
    advanceToNextSet = true;
  }

  if (advanceToNextSet)
  {
    while (geoLocItr != geoLocEnd && setNumber == (*geoLocItr)->setNo())
    {
      ++geoLocItr;
    }
  }

  return !advanceToNextSet;
}

//----------------------------------------------------------------------------
//  checkLoc
//
//  Match the mileageRouteItem to the psr geo location.
//  Set the geoLocIterator to the next geoLoc item to be validated.
//----------------------------------------------------------------------------
bool
PermissibleSpecifiedRouting::checkLoc(
    MileageRoute& mileageRoute,
    MileageRouteItem& mRouteItem,
    int32_t setNumber,
    std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocItr,
    const std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocEnd,
    Indicator& groupRelationalInd) const
{
  if (mRouteItem.isDestinationInLoc((*geoLocItr)->loc()))
  {
    if ((*geoLocItr)->stopoverNotAllowed() == YES && mRouteItem.isStopover())
    {
      mRouteItem.psrStopNotAllowed() = true;
    }

    //------------------------------------------------------------------------
    // Check for OR condition which requires bumping to the next required loc
    //------------------------------------------------------------------------
    while (geoLocItr != (geoLocEnd - 1) && setNumber == (*geoLocItr)->setNo())
    {
      ++geoLocItr;

      if (geoLocItr == geoLocEnd || setNumber != (*geoLocItr)->setNo())
      {
        return true;
      }

      if ((*geoLocItr)->relationalInd() != VIAGEOLOCREL_OR)
      {
        groupRelationalInd = (*geoLocItr)->relationalInd();
        return true;
      }
    }

    ++geoLocItr; // Bump to end of geoLocs or start of next set
    return true; // Last geoLoc validated
  }

  return false;
}

//-------------------------------------------------------------------------------------
// PermissibleSpecifiedRouting::processGeoLocStopoverConditions
//
// If any mileageRouteItems were flagged as psrNoStopAllowed, or the total count of
// stopovers exceeds the maximum, set the psrMayApply indicator.
//
// Return false when the stopover conditions for this PSR are not met.
//-------------------------------------------------------------------------------------
bool
PermissibleSpecifiedRouting::processGeoLocStopoverConditions(MileageRoute& mileageRoute,
                                                             TpdPsr& psr) const
{

  MileageRouteItems::const_iterator mItr(mileageRoute.mileageRouteItems().begin());
  MileageRouteItems::const_iterator mEnd(mileageRoute.mileageRouteItems().end() - 1);

  for (; mItr != mEnd; ++mItr)
  {
    if ((*mItr).psrStopNotAllowed())
    {
      mileageRoute.psrMayApply() = true;
      return false;
    }

    else if ((*mItr).isStopover())
    {
      mileageRoute.psrMayApply() = true;
      mileageRoute.stopoverCount()++;
    }
  }

  if (psr.stopoverCnt() != -1)
  {
    if (mileageRoute.stopoverCount() > psr.stopoverCnt())
    {
      return false;
    }
  }

  return true;
}

//------------------------------------------------------------------------------------------------------------
// Check geoLocs in psr item to see if there are any other required locs
//------------------------------------------------------------------------------------------------------------
bool
PermissibleSpecifiedRouting::checkGeoLocs(
    MileageRoute& mileageRoute,
    int32_t setNumber,
    std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocItr,
    const std::vector<TpdPsrViaGeoLoc*>::const_iterator& geoLocEnd) const
{
  while (geoLocItr != geoLocEnd && setNumber == (*geoLocItr)->setNo())
  {
    if ((*geoLocItr)->relationalInd() == VIAGEOLOCREL_AND)
    {
      //---------------------------------------------
      // Advance to next set or end of geoLoc items
      //---------------------------------------------
      while (geoLocItr != geoLocEnd && setNumber == (*geoLocItr)->setNo())
      {
        ++geoLocItr;
      }
      return false;
    }

    ++geoLocItr;
  }

  return true;
}

//----------------------------------------------------------------------------
//  PermissibleSpecifiedRouting::processThruMktCxrs()
//
//  Process through market carrier requirements or exceptions.
//----------------------------------------------------------------------------
bool
PermissibleSpecifiedRouting::processThruMktCxrs(const MileageRoute& mileageRoute, TpdPsr& psr) const
{

  if (psr.carrier().empty() && !psr.thruMktCxrs().empty())
  {
    bool found = std::find(psr.thruMktCxrs().begin(),
                           psr.thruMktCxrs().end(),
                           mileageRoute.governingCarrier()) != psr.thruMktCxrs().end();

    if (found == (psr.thruMktCarrierExcept() == YES))
    {
      return false;
    }
  }

  else if (!psr.carrier().empty())
  {
    if (psr.carrier() != mileageRoute.governingCarrier())
    {
      return false;
    }
  }

  return true;
}

//----------------------------------------------------------------------------
//  PermissibleSpecifiedRouting::processViaMktSameCxr()
//
//  Process via market carrier requirements.
//----------------------------------------------------------------------------
bool
PermissibleSpecifiedRouting::processViaMktSameCarrier(const MileageRoute& mileageRoute, TpdPsr& psr)
    const
{

  if (psr.thruViaMktSameCxr() == NO)
  {
    return true;
  }

  //---------------------------------------------------------------------------------------
  // If Governing Carrier exists in the PSR, check all cxrs in the mileageRoute against it.
  //---------------------------------------------------------------------------------------
  bool carrierMatches = false;
  if (!psr.carrier().empty())
  {
    const CarrierCode& carrier(psr.carrier());
    carrierMatches = checkAllViaCarriers(mileageRoute, carrier);
  }

  //-------------------------------------------------------------------------
  // Compare carrier specified in PSR item to carriers in the mileageRoute.
  // Travel on all segments is valid on one of the specified cxrs, not via a
  // combination of carriers.
  //-------------------------------------------------------------------------
  else
  {
    std::vector<CarrierCode>::const_iterator cxrItr = psr.thruMktCxrs().begin();
    std::vector<CarrierCode>::const_iterator cxrEnd = psr.thruMktCxrs().end();

    for (; cxrItr != cxrEnd; ++cxrItr)
    {
      const CarrierCode& carrier(*cxrItr);
      if (psr.thruMktCarrierExcept() == YES)
      {
        carrierMatches = checkAnyViaCarrier(mileageRoute, carrier);
        if (!carrierMatches)
        {
          break;
        }
      }
      else
      {
        carrierMatches = checkAllViaCarriers(mileageRoute, carrier);
        if (carrierMatches)
        {
          break;
        }
      }
    }
  }

  return carrierMatches;
}

//----------------------------------------------------------------------------
//  PermissibleSpecifiedRouting::checkAllViaCarriers()
//
//  Check carrier requirements for through and via markets.
//  Return true when all carriers in the mileageRoute match the specified cxr.
//----------------------------------------------------------------------------
bool
PermissibleSpecifiedRouting::checkAllViaCarriers(const MileageRoute& mileageRoute,
                                                 const CarrierCode& carrier) const
{

  MileageRouteItems::const_iterator itr(mileageRoute.mileageRouteItems().begin());
  MileageRouteItems::const_iterator end(mileageRoute.mileageRouteItems().end());

  for (; itr != end; ++itr)
  {
    if (itr->segmentCarrier() != carrier)
    {
      return false;
    }
  }

  return true;
}

//----------------------------------------------------------------------------
//  PermissibleSpecifiedRouting::checkAnyViaCarrier()
//
//  Check carrier requirements for through and via markets when the carriers
//  are specified as an exception.
//
//  Return true when none of the mileageRouteItems contain the specified cxr.
//----------------------------------------------------------------------------
bool
PermissibleSpecifiedRouting::checkAnyViaCarrier(const MileageRoute& mileageRoute,
                                                const CarrierCode& carrier) const
{

  MileageRouteItems::const_iterator itr(mileageRoute.mileageRouteItems().begin());
  MileageRouteItems::const_iterator end(mileageRoute.mileageRouteItems().end());

  for (; itr != end; ++itr)
  {
    if (itr->segmentCarrier() == carrier)
    {
      return false;
    }
  }

  return true;
}

//----------------------------------------------------------------------------
//  PermissibleSpecifiedRouting::processViaCxrLocs()
//----------------------------------------------------------------------------
bool
PermissibleSpecifiedRouting::processViaCxrLocs(const MileageRoute& mileageRoute, TpdPsr& psr) const
{
  std::vector<TpdPsrViaCxrLoc*>::const_iterator viaCxrItr = psr.viaCxrLocs().begin();
  std::vector<TpdPsrViaCxrLoc*>::const_iterator end = psr.viaCxrLocs().end();

  std::set<CarrierCode> viaCxrSet;

  for (; viaCxrItr != end; ++viaCxrItr)
  {
    const TpdPsrViaCxrLoc& viaCxrLoc(**viaCxrItr);

    if (viaCxrLoc.loc1().loc().empty() || // No Loc1 and Loc2 specified, only carrier
        ((viaCxrLoc.loc1().locType() ==
          psr.loc1().locType()) && // Get around content GUI restriction
         (viaCxrLoc.loc1().loc() == psr.loc1().loc()) &&
         (viaCxrLoc.loc2().locType() == psr.loc2().locType()) &&
         (viaCxrLoc.loc2().loc() == psr.loc2().loc())))
    {
      // This item is for all travel via Cxr
      viaCxrSet.insert(viaCxrLoc.viaCarrier());
      continue;
    }

    MileageRouteItems::const_iterator mrItr(mileageRoute.mileageRouteItems().begin());
    MileageRouteItems::const_iterator mrEnd(mileageRoute.mileageRouteItems().end());

    for (; mrItr != mrEnd; ++mrItr)
    {
      const MileageRouteItem& mrItem = (*mrItr);

      if ((mrItem.isOriginInLoc(viaCxrLoc.loc1()) && mrItem.isDestinationInLoc(viaCxrLoc.loc2())) ||
          (mrItem.isOriginInLoc(viaCxrLoc.loc2()) && mrItem.isDestinationInLoc(viaCxrLoc.loc1())))
      {
        if (mrItem.segmentCarrier() != viaCxrLoc.viaCarrier())
          return false;
      }
    }
  }

  if (!viaCxrSet.empty())
  {
    MileageRouteItems::const_iterator mrItr(mileageRoute.mileageRouteItems().begin());
    MileageRouteItems::const_iterator mrEnd(mileageRoute.mileageRouteItems().end());
    for (; mrItr != mrEnd; ++mrItr)
    {
      if (!(*mrItr).segmentCarrier().empty() &&
          (viaCxrSet.find((*mrItr).segmentCarrier()) == viaCxrSet.end()))
        return false;
    }
  }

  return true;
}

//----------------------------------------------------------------------------
//  PermissibleSpecifiedRouting::processViaCxrLocExceptions()
//----------------------------------------------------------------------------
bool
PermissibleSpecifiedRouting::processViaCxrLocExceptions(const MileageRoute& mileageRoute,
                                                        TpdPsr& psr) const
{
  std::vector<TpdPsrViaExcept*>::const_iterator viaCxrItr = psr.viaExcepts().begin();
  std::vector<TpdPsrViaExcept*>::const_iterator end = psr.viaExcepts().end();

  for (; viaCxrItr != end; ++viaCxrItr)
  {

    MileageRouteItems::const_iterator mrItr(mileageRoute.mileageRouteItems().begin());
    MileageRouteItems::const_iterator mrEnd(mileageRoute.mileageRouteItems().end());

    for (; mrItr != mrEnd; ++mrItr)
    {
      const MileageRouteItem& mrItem = (*mrItr);

      if ((mrItem.isOriginInLoc((*viaCxrItr)->loc1()) &&
           mrItem.isDestinationInLoc((*viaCxrItr)->loc2())) ||
          (mrItem.isOriginInLoc((*viaCxrItr)->loc2()) &&
           mrItem.isDestinationInLoc((*viaCxrItr)->loc1())))
      {
        if (mrItem.segmentCarrier() == (*viaCxrItr)->viaCarrier())
          return false;
      }
    }
  }

  return true;
}

//----------------------------------------------------------------------------
//  PermissibleSpecifiedRouting::processFareType
//----------------------------------------------------------------------------
bool
PermissibleSpecifiedRouting::processFareType(const MileageRoute& mileageRoute, TpdPsr& psr) const
{
  return true;
}

//----------------------------------------------------------------------------
//  PermissibleSpecifiedRouting::reverseRoute
//
//----------------------------------------------------------------------------
void
PermissibleSpecifiedRouting::reverseRoute(const MileageRoute& mileageRoute,
                                          MileageRoute& reverseMileageRoute) const
{
  reverseMileageRoute.partialInitialize(mileageRoute);

  reverseMileageRoute.mileageRouteItems().clear();
  reverseMileageRoute.mileageRouteItems().reserve(mileageRoute.mileageRouteItems().size());

  MileageRouteItems::const_reverse_iterator i(mileageRoute.mileageRouteItems().rbegin());

  for (; i != mileageRoute.mileageRouteItems().rend(); i++)
  {
    MileageRouteItem mri;
    mri.city1() = i->city2();
    mri.multiTransportOrigin() = i->multiTransportDestination();
    mri.city2() = i->city1();
    mri.multiTransportDestination() = i->multiTransportOrigin();
    mri.isSurface() = i->isSurface();
    mri.isConstructed() = i->isConstructed();
    mri.tpm() = i->tpm();
    mri.tpd() = i->tpd();
    mri.mpm() = i->mpm();
    mri.tpmSurfaceSectorExempt() = i->tpmSurfaceSectorExempt();
    mri.southAtlanticExclusion() = i->southAtlanticExclusion();
    mri.segmentCarrier() = i->segmentCarrier();
    mri.isStopover() = i->isStopover();
    reverseMileageRoute.mileageRouteItems().push_back(mri);
  }
  // shift stopovers one item backward
  MileageRouteItems& items(reverseMileageRoute.mileageRouteItems());
  if (LIKELY(items.size() > 1))
  {
    MileageRouteItems::iterator j(++items.begin()), e(items.end());
    for (; j != e; ++j)
      (j - 1)->isStopover() = j->isStopover();
  }
}
} //tse
