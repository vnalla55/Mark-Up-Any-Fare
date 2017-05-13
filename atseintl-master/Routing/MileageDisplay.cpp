//----------------------------------------------------------------------------
//
//  Copyright Sabre 2003
//
//      The copyright to the computer program(s) herein
//      is the property of Sabre.
//      The program(s) may be used and/or copied only with
//      the written permission of Sabre or in accordance
//      with the terms and conditions stipulated in the
//      agreement/contract under which the program(s)
//      have been supplied.
//
//----------------------------------------------------------------------------

#include "Routing/MileageDisplay.h"

#include "Common/Global.h"
#include "Common/LocUtil.h"
#include "Common/MCPCarrierUtil.h"
#include "Common/TseEnums.h"
#include "Common/TseCodeTypes.h"
#include "DataModel/Agent.h"
#include "DataModel/MileageTrx.h"
#include "DBAccess/Loc.h"
#include "DBAccess/TpdPsr.h"
#include "Routing/MileageRoute.h"
#include "Routing/MultiTransportRetriever.h"

#include <algorithm>
#include <vector>

namespace tse
{
MileageDisplay::MileageDisplay(MileageTrx& trx) : _trx(trx) {}

MileageDisplay::~MileageDisplay() {}

struct IsHidden : public std::unary_function<MileageTrx::MileageItem*, bool>
{
  bool operator()(const MileageTrx::MileageItem* i) const { return i->isHidden; }
};

struct LessByCityLocCode
    : public std::binary_function<MileageTrx::MileageItem*, MileageTrx::MileageItem*, bool>
{
  bool operator()(const MileageTrx::MileageItem* l, const MileageTrx::MileageItem* r) const
  {
    return l->cityLoc->loc() < r->cityLoc->loc();
  }
} lessByCityLocCode;

struct EqualByCityLocCode
    : public std::binary_function<MileageTrx::MileageItem*, MileageTrx::MileageItem*, bool>
{
  bool operator()(const MileageTrx::MileageItem* l, const MileageTrx::MileageItem* r) const
  {
    return l->cityLoc->loc() == r->cityLoc->loc();
  }
} equalByCityLocCode;

bool
MileageDisplay::displayMileageRequest(const MileageTrx& trx)
{
  const MultiTransportRetriever& multiTransport(
      tse::Singleton<MultiTransportRetriever>::instance());
  _trx.response().setf(std::ios::right, std::ios::adjustfield);
  if (trx.getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
  {
    _trx.response() << "VD " << std::endl;
  }

  _trx.response() << " " << std::endl << std::setw(3) << "WN";

  if (!trx.items().front()->cityLoc || !trx.items().back()->cityLoc)
  {
    return false;
  }
  LocCode origin(trx.items().front()->cityLoc->loc());
  LocCode destination(trx.items().back()->cityLoc->loc());
  bool originDestinationRetransitted(false), intermediateRetransitted(false);
  // std::vector<MileageTrx::MileageItem*>::const_iterator itr = trx.items().begin();
  std::vector<MileageTrx::MileageItem*> notHidden;
  std::remove_copy_if(
      trx.items().begin(), trx.items().end(), std::back_inserter(notHidden), IsHidden());
  std::vector<MileageTrx::MileageItem*>::const_iterator begin(notHidden.begin());
  std::vector<MileageTrx::MileageItem*>::const_iterator itr(notHidden.begin());
  std::vector<MileageTrx::MileageItem*>::const_iterator end(notHidden.end());
  uint16_t cityCount = 1;
  for (; itr != end; ++itr)
  {
    const MileageTrx::MileageItem& item(**itr);
    if (item.isHidden)
      continue;

    if (!item.cityLoc)
      return false;

    LocCode locCode(item.cityLoc->loc());
    locCode = multiTransport.retrieve(trx.dataHandle(), locCode, trx.inputDT());

    if (item.stopType == StopType::Connection)
      _trx.response() << " X/" << locCode;
    else
      _trx.response() << std::setw(4) << locCode;
    if (itr != end - 1)
    {
      _trx.response() << std::setw(3)
                      << (item.isSurface ? "//" : item.carrierCode.empty()
                                                      ? "YY"
                                                      : MCPCarrierUtil::swapToPseudo(
                                                            &trx, item.carrierCode));
      //            _trx.response() << std::setw(3)
      //	       << (item.isSurface ? "//" : item.carrierCode.empty() ? "YY" : item.carrierCode);
    }
    if (cityCount == 7)
    {
      _trx.response().setf(std::ios::right, std::ios::adjustfield);
      _trx.response() << " " << std::endl << std::setw(3) << std::setfill(' ') << " ";
      cityCount = 0;
    }
    if (itr != begin && itr != end - 1)
    {
      originDestinationRetransitted =
          originDestinationRetransitted || locCode == origin || locCode == destination;
    }
    ++cityCount;
  }
  if (notHidden.size() > 3)
  {
    std::vector<MileageTrx::MileageItem*> intermediates(notHidden.begin() + 1, notHidden.end() - 1);
    std::vector<MileageTrx::MileageItem*>::iterator nonAdjacentEnd =
        std::unique(intermediates.begin(), intermediates.end(), equalByCityLocCode);
    std::sort(intermediates.begin(), nonAdjacentEnd, lessByCityLocCode);
    intermediateRetransitted =
        std::adjacent_find(intermediates.begin(), nonAdjacentEnd, equalByCityLocCode) !=
        nonAdjacentEnd;
  }

  _trx.response() << std::setw(2) << "/" << std::setw(5) << _trx.inputDT().dateToString(DDMMMYY, "")
                  << std::endl;

  if (originDestinationRetransitted)
    displayOriginDestinationRetransitted();
  if (intermediateRetransitted)
    displayIntermediateRetransitted();

  _trx.response() << std::endl << "*************************************************************"
                  << std::endl;
  return true;
}

bool
MileageDisplay::displayOriginDestinationRetransitted()
{

  _trx.response() << "    ***  - CONTAINS RETRANSITTED ORIGIN/DESTINATION POINT" << std::endl;
  return true;
}

bool
MileageDisplay::displayIntermediateRetransitted()
{

  _trx.response() << "    ***  - CONTAINS RETRANSITTED INTERMEDIATE POINT" << std::endl;
  return true;
}

bool
MileageDisplay::displayHeader(const MileageTrx& trx)
{

  _trx.response().setf(std::ios::right, std::ios::adjustfield);
  _trx.response() << " " << std::endl << std::setw(7) << "CTY" << std::setw(5) << "GI"
                  << std::setw(6) << "TPM" << std::setw(7) << "CUM" << std::setw(7) << "MPM"
                  << std::setw(5) << "EMS" << std::setw(6) << "DED" << std::setw(6) << "LAST"
                  << std::setw(6) << "NEXT" << std::setw(6) << "25M" << std::endl;
  return true;
}

bool
MileageDisplay::displayGDPrompt(const MileageTrx& trx, const GDPrompt* gdPrompt)
{
  if (trx.getRequest()->ticketingAgent()->axessUser()) // JAL/AXESS agent
  {
    _trx.response() << "VE " << std::endl;
  }

  if (gdPrompt != nullptr)
  {
    if (gdPrompt->currentGD() == GlobalDirection::ZZ)
    {
      _trx.response() << " " << std::endl << "REQUIRE GI " << gdPrompt->origin()
                      << gdPrompt->destination() << " - TRY";
    }
    else
    {
      _trx.response() << " " << std::endl << "INVALID GI " << gdPrompt->origin()
                      << gdPrompt->destination() << " - TRY";
    }
    std::vector<GlobalDirection>::const_iterator itr(gdPrompt->alternateGDs().begin());
    std::vector<GlobalDirection>::const_iterator end(gdPrompt->alternateGDs().end());
    for (; itr != end; ++itr)
    {
      std::string gdStr;
      globalDirectionToStr(gdStr, *itr);
      _trx.response() << std::setw(3) << gdStr;
    }
    _trx.response() << " " << std::endl;
  }
  else
    _trx.response() << " " << std::endl << "INVALID GI" << std::endl;
  return true;
}

bool
MileageDisplay::displayConstructed()
{

  // TODO    _trx.response() << " @  CONSTRUCTED MILEAGE" << std::endl;
  return true;
}

bool
MileageDisplay::displaySouthAtlantic(const MileageRoute& mRoute)
{
  _trx.response() << " *  ";
  if (mRoute.specificCXR())
    _trx.response() << "CARRIER SPECIFIC ";

  _trx.response() << "TICKETED POINT MILEAGE EXCLUSION APPLIED" << std::endl;

  return true;
}
bool
MileageDisplay::displayEqualization(const MileageRoute& mRoute)
{
  LocCode& firstCity = mRoute.mileageRouteItems().front().origCityOrAirport()->loc();
  LocCode& lastCity = mRoute.mileageRouteItems().back().destCityOrAirport()->loc();

  if (firstCity == RIO_DE_JANEIRO || lastCity == RIO_DE_JANEIRO)
  {
    _trx.response() << " $  MILEAGE EQUALIZATION APPLIED - RIO MILEAGE SURCHARGE " << std::endl
                    << "    NEED NOT EXCEED SAO MILEAGE SURCHARGE " << std::endl;
  }
  else if (firstCity == SAO_PAULO || lastCity == SAO_PAULO)
  {
    _trx.response() << " $  MILEAGE EQUALIZATION APPLIED - SAO MILEAGE SURCHARGE " << std::endl
                    << "    NEED NOT EXCEED RIO MILEAGE SURCHARGE " << std::endl;
  }
  else
  {

    _trx.response() << " $  MILEAGE EQUALIZATION APPLIED " << std::endl;
  }
  return true;
}

bool
MileageDisplay::displayPSR()
{

  _trx.response() << " *  SPECIFIED ROUTING APPLIES" << std::endl;
  return true;
}

bool
MileageDisplay::displayPSRMayApply(const MileageRoute& mileageRoute)
{

  _trx.response() << " *  SPECIFIED ROUTING APPLIES WHEN NO STOPOVER AT ";

  MileageRouteItems::const_iterator mItr(mileageRoute.mileageRouteItems().begin());
  MileageRouteItems::const_iterator mEnd(mileageRoute.mileageRouteItems().end() - 1);

  bool firstTime = true;

  for (; mItr != mEnd; ++mItr)
  {
    if ((*mItr).isStopover())
    {
      if (!firstTime)
      {
        _trx.response() << " AND/OR ";
        _trx.response() << (*mItr).destCityOrAirport()->loc();
      }
      else
      {
        _trx.response() << (*mItr).destCityOrAirport()->loc() << std::endl;
        _trx.response() << "   ";
        firstTime = false;
      }
    }
  }

  _trx.response() << std::endl;

  return true;
}

bool
MileageDisplay::displayMileageUnavailable()
{

  _trx.response() << " **** MILEAGE DATA NOT AVAILABLE" << std::endl;
  return true;
}

bool
MileageDisplay::displayTPD()
{

  _trx.response() << " *  TICKETED POINT DEDUCTION APPLIED" << std::endl;
  return true;
}

bool
MileageDisplay::displaySurfaceSector(
    const std::vector<std::pair<const LocCode*, const LocCode*> >& surfaceSectors)
{
  std::vector<std::pair<const LocCode*, const LocCode*> >::const_iterator itr(
      surfaceSectors.begin());
  std::vector<std::pair<const LocCode*, const LocCode*> >::const_iterator end(surfaceSectors.end());
  for (; itr != end; ++itr)
  {
    const std::pair<const LocCode*, const LocCode*>& curr(*itr);
    _trx.response() << " $  MILEAGE BETWEEN " << *curr.first << " - " << *curr.second
                    << " MAY BE IGNORED PROVIDED TRAVEL IS" << std::endl
                    << "    VIA SURFACE TRANSPORTATION. MAY NOT BE THE POINT OF ORIGIN" << std::endl
                    << "    OR DESTINATION OF THE ROUTE OF TRAVEL. TO VIEW MILEAGE WITH"
                    << std::endl << "    SURFACE TRANSPORTATION APPLIED-INSERT // BETWEEN "
                    << *curr.first << " - " << *curr.second << std::endl;
  }
  return true;
}

bool
MileageDisplay::displayConditionalTPDs(
    const std::vector<const std::vector<const TpdPsrViaGeoLoc*>*>& conditionalTPDs)
{
  std::vector<const std::vector<const TpdPsrViaGeoLoc*>*>::const_iterator i(
      conditionalTPDs.begin());
  std::vector<const std::vector<const TpdPsrViaGeoLoc*>*>::const_iterator e(conditionalTPDs.end());
  for (; i != e; ++i)
  {
    _trx.response() << " *  TICKETED POINT DEDUCTION APPLIES WHEN NO STOPOVER AT" << std::endl
                    << std::setw(3) << " ";
    const std::vector<const TpdPsrViaGeoLoc*>& stops(**i);
    std::vector<const TpdPsrViaGeoLoc*>::const_iterator bb(stops.begin());
    std::vector<const TpdPsrViaGeoLoc*>::const_iterator ii(stops.begin());
    std::vector<const TpdPsrViaGeoLoc*>::const_iterator ee(stops.end());
    for (; ii != ee; ++ii)
    {
      const TpdPsrViaGeoLoc& loc(**ii);
      if (ii != bb)
      {
        if (loc.relationalInd() == VIAGEOLOCREL_AND)
          _trx.response() << " AND";
        else if (loc.relationalInd() == VIAGEOLOCREL_ANDOR)
          _trx.response() << " AND/OR";
        else if (loc.relationalInd() == VIAGEOLOCREL_OR || ii != bb)
          _trx.response() << " OR";
      }
      _trx.response() << std::setw(4) << loc.loc().loc();
    }
    _trx.response() << " " << std::endl;
  }
  return true;
}

struct ViaGeoLocsEqual : public std::binary_function<TpdPsrViaGeoLoc, TpdPsrViaGeoLoc, bool>
{
  bool operator()(const TpdPsrViaGeoLoc* l, const TpdPsrViaGeoLoc* r) const
  {
    return l->loc().loc() == r->loc().loc() && l->relationalInd() == r->relationalInd();
  }

  // lint --e{1509}
} viaGeoLocsEqual;

class ViaGeoLocListEquals : public std::unary_function<std::vector<const TpdPsrViaGeoLoc*>, bool>
{
public:
  ViaGeoLocListEquals(const std::vector<const TpdPsrViaGeoLoc*>& toCompare) : _toCompare(toCompare)
  {
  }
  bool operator()(const std::vector<const TpdPsrViaGeoLoc*>* l) const
  {
    if (l->size() != _toCompare.size())
      return false;
    return std::mismatch(l->begin(), l->end(), _toCompare.begin(), viaGeoLocsEqual) ==
           std::make_pair(l->end(), _toCompare.end());
  }

private:
  const std::vector<const TpdPsrViaGeoLoc*>& _toCompare;

  // lint --e{1509}
};

bool
MileageDisplay::displayMileageRoute(const MileageRoute& mileageRoute)
{
  const MileageRouteItems& mileageRouteItems(mileageRoute.mileageRouteItems());
  if (mileageRouteItems.empty())
  {
    return true;
  }
  uint16_t cumulativeMileage(0), segmentNo(0);
  bool isConstructed(false), southAtlantic(false), surfaceSector(false), tpd(false),
      psrApplies(false), psrMayApply(false), mileageMissing(false), wasInternational(false);
  std::vector<std::pair<const LocCode*, const LocCode*> > surfaceSectors;
  std::vector<const std::vector<const TpdPsrViaGeoLoc*>*> conditionalTPDs;
  MileageRouteItems::const_iterator itr(mileageRouteItems.begin());
  MileageRouteItems::const_iterator end(mileageRouteItems.end());
  displayMileageRouteItem(*itr);
  bool tpmExclusion(mileageRoute.southAtlanticExclusion());
  bool mileageEqualization(mileageRoute.mileageEqualizationApplies());
  NationCode originNation(mileageRouteItems.front().origCityOrAirport()->nation());
  bool faildDirectService = isFailedDirService(mileageRoute);

  for (; itr != end; ++itr)
  {
    const MileageRouteItem& item(*itr);
    ++segmentNo;
    if (!item.tpmSurfaceSectorExempt() || !item.isSurface())
    {
      if (tpmExclusion)
      {
        cumulativeMileage = item.southAtlanticExclusion() == true
                                ? cumulativeMileage
                                : (cumulativeMileage + item.tpm());
        cumulativeMileage =
            ((itr == end - 1) && item.southAtlanticExclusion()) ? item.tpm() : cumulativeMileage;
      }
      else
      {
        cumulativeMileage += item.tpm();
      }
    }
    wasInternational = wasInternational || item.destCityOrAirport()->nation() != originNation;
    displayMileageRouteItem(mileageRoute,
                            item,
                            cumulativeMileage - item.tpd(),
                            segmentNo,
                            wasInternational,
                            mileageEqualization && itr == end - 1);
    isConstructed = isConstructed || item.isConstructed();
    southAtlantic = southAtlantic || item.southAtlanticExclusion();
    psrApplies = psrApplies || item.psrApplies();
    psrMayApply = psrMayApply || item.psrMayApply();
    if (item.tpmSurfaceSectorExempt())
    {
      surfaceSector = true;
      surfaceSectors.push_back(
          std::make_pair(&item.origCityOrAirport()->loc(), &item.destCityOrAirport()->loc()));
    }
    tpd = tpd || item.tpd() > 0;
    if (item.tpd() == 0 && !faildDirectService && !item.condTpdViaGeoLocs().empty())

    {

      if (std::find_if(conditionalTPDs.begin(),
                       conditionalTPDs.end(),
                       ViaGeoLocListEquals(item.condTpdViaGeoLocs())) == conditionalTPDs.end())
        conditionalTPDs.push_back(&item.condTpdViaGeoLocs());
    }
    mileageMissing = mileageMissing || (wasInternational && item.mpm() == 0);
  }
  _trx.response() << " " << std::endl;

  if (mileageMissing)
    displayMileageUnavailable();
  if (mileageEqualization)
    displayEqualization(mileageRoute);
  if (southAtlantic)
  {
    displaySouthAtlantic(mileageRoute);
  }
  if (psrApplies)
  {
    displayPSR();
  }
  else if (psrMayApply)
  {
    displayPSRMayApply(mileageRoute);
  }
  if (surfaceSector)
    displaySurfaceSector(surfaceSectors);
  if (tpd)
    displayTPD();
  if (!conditionalTPDs.empty())
    displayConditionalTPDs(conditionalTPDs);

  return true;
}

bool
MileageDisplay::displayMileageRouteItem(const MileageRouteItem& item)
{
  // display first row
  _trx.response().setf(std::ios::right, std::ios::adjustfield);
  _trx.response() << " " << std::endl << std::setw(7) << item.origCityOrAirport()->loc()
                  << std::setw(1) << " " << item.origCityOrAirport()->area();
  return true;
}

bool
MileageDisplay::displayMileageRouteItem(const MileageRoute& mRoute,
                                        const MileageRouteItem& item,
                                        uint16_t cumMileage,
                                        uint16_t segmentNo,
                                        bool wasInternational,
                                        bool mileageEqualization)

{

  bool returnToCountryOfOrigin(false);
  if (segmentNo != 1)
  {
    const NationCode& routeOrigin =
        mRoute.mileageRouteItems().front().origCityOrAirport()->nation();
    const NationCode& itemOrigin = item.destCityOrAirport()->nation();
    returnToCountryOfOrigin = (itemOrigin == routeOrigin);
  }

  // display 2nd row and others
  std::string globalDirectionStr("- ");
  GlobalDirection gd(item.mpmGlobalDirection());
  if (item.mpm() > 0 && gd != GlobalDirection::XX && gd != GlobalDirection::ZZ &&
      !returnToCountryOfOrigin)
  {
    globalDirectionToStr(globalDirectionStr, gd);
  }

  _trx.response().setf(std::ios::right, std::ios::adjustfield);
  _trx.response() << std::endl << std::setw(2) << segmentNo << std::setw(1) << ".";
  _trx.response() << (item.isStopover() ? " " : "X");
  _trx.response() << std::setw(3) << item.destCityOrAirport()->loc();
  if (item.southAtlanticExclusion())
  {
    _trx.response() << "*" << std::setw(1) << item.destCityOrAirport()->area();
    _trx.response() << std::setw(3) << globalDirectionStr << std::setw(6) << " - ";
  }
  else
  {
    _trx.response() << (mileageEqualization ? "$" : " ");
    _trx.response() << std::setw(1) << item.destCityOrAirport()->area();

    if (item.tpm())
      _trx.response() << std::setw(3) << globalDirectionStr << std::setw(6)
                      << (item.isSurface() && item.tpmSurfaceSectorExempt() ? 0 : item.tpm());
    else
      _trx.response() << std::setw(3) << globalDirectionStr << std::setw(6) << "****";
  }
  if (item.tpmSurfaceSectorExempt())
  {
    _trx.response() << "$";
  }
  else if (item.isConstructed())
  {
    _trx.response() << "@";
  }
  else
  {
    _trx.response() << " ";
  }
  if (item.tpm())
    _trx.response() << std::setw(6) << cumMileage;
  else
    _trx.response() << std::setw(6) << "****";
  _trx.response() << (item.tpd() > 0 ||
                              (!isFailedDirService(mRoute) && !item.condTpdViaGeoLocs().empty())
                          ? "*"
                          : " ");
  if (item.mpm() > 0)
  {
    _trx.response() << std::setw(6) << item.mpm();
  }
  else
  {
    if (wasInternational)
    {
      _trx.response() << std::setw(6) << "****";
    }
    else
    {
      _trx.response() << std::setw(6) << " ";
    }
  }
  _trx.response() << " ";
  if (item.mpm() > 0)
  {
    uint16_t eMS = 0;
    displayEMS(item, cumMileage, eMS);
    // now display the DED column
    uint16_t ded = item.tpd();
    _trx.response() << std::setw(5) << ded;
    // surcharge
    displaySurchargeInfo(item, cumMileage, eMS);
  }
  else
    _trx.response() << " " << std::endl;
  return true;
}

bool
MileageDisplay::displaySurchargeInfo(const MileageRouteItem& mItem,
                                     uint16_t cumMileage,
                                     uint16_t eMS)
{

  float factor = 1.00;
  uint16_t last = 0;
  uint16_t next = 0;
  if (eMS > 0)
  {
    factor = factor + (float)(eMS - 5) / 100;
    last = static_cast<uint16_t>(cumMileage - (mItem.mpm() * factor));
    factor = 1.00;
    factor = factor + (float)eMS / 100;
    if (mItem.mpm() > 0)
    {
      next = static_cast<uint16_t>((mItem.mpm() * factor) - cumMileage);
    }
  }
  else
  {
    last = 0;
    next = (mItem.mpm() > 0 ? (mItem.mpm() - cumMileage) : 0);
  }

  _trx.response() << std::setw(6) << last;

  if (eMS != 30)
  {
    _trx.response() << std::setw(6) << next;
  }
  else
  {
    _trx.response() << "   EXC";
  }

  _trx.response() << std::setw(6) << static_cast<uint16_t>(mItem.mpm() * (1.25)) << std::endl;

  return true;
}

bool
MileageDisplay::displayEMS(const MileageRouteItem& mItem, uint16_t cumMileage, uint16_t& eMS)
{
  double tempEms =
      ((cumMileage > mItem.mpm() && mItem.mpm() > 0) ? (double(cumMileage) / double(mItem.mpm()))
                                                     : 0);

  if (tempEms == 0)
  {
    eMS = 0;
  }
  else if (1.00 < tempEms && tempEms <= 1.05000)
  {
    eMS = 5;
  }
  else if (1.05 < tempEms && tempEms <= 1.10000)
  {
    eMS = 10;
  }
  else if (1.10 < tempEms && tempEms <= 1.15000)
  {
    eMS = 15;
  }
  else if (1.15 < tempEms && tempEms <= 1.20000)
  {
    eMS = 20;
  }
  else if (1.20 < tempEms && tempEms <= 1.25000)
  {
    eMS = 25;
  }
  else
  {
    eMS = 30;
  }

  if (eMS < 30)
  {
    _trx.response() << std::setw(3) << eMS << "M"
                    << (mItem.psrApplies() || mItem.psrMayApply() ? "*" : " ");
  }

  else
  {
    _trx.response() << " EXC" << (mItem.psrApplies() || mItem.psrMayApply() ? "*" : " ");
  }

  return true;
}

bool
MileageDisplay::isFailedDirService(const MileageRoute& mileageRoute)
{
  const MileageRouteItems& mileageRouteItems(mileageRoute.mileageRouteItems());

  MileageRouteItems::const_iterator itr(mileageRouteItems.begin());
  MileageRouteItems::const_iterator end(mileageRouteItems.end());
  for (; itr != end; ++itr)
  {
    const MileageRouteItem& item(*itr);

    if (item.failedDirService())
      return true;
  }

  return false;
}
}
