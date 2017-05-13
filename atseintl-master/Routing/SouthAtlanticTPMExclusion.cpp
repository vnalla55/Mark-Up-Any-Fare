#include "Routing/SouthAtlanticTPMExclusion.h"

#include "Common/LocUtil.h"
#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "Common/Vendor.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/Loc.h"
#include "DBAccess/LocKey.h"
#include "DBAccess/TPMExclusion.h"
#include "Diagnostic/Diag452Collector.h"
#include "Routing/MileageRoute.h"
#include "Routing/MileageRouteItem.h"
#include "Routing/MultiTransportRetriever.h"
#include "Routing/RoutingConsts.h"

using namespace std;

namespace tse
{
static log4cxx::LoggerPtr
logger(log4cxx::Logger::getLogger("atseintl.Routing.SouthAtlanticTPMExclusion"));

namespace
{

// private utility functions (in anonymous namespace)

void
logParms(const MileageRoute& mRoute)
{
  std::string s;
  const MileageRouteItems& mileageRouteItems(mRoute.mileageRouteItems());
  MileageRouteItems::const_iterator i = mileageRouteItems.begin();
  for (; i != mileageRouteItems.end(); ++i)
  {
    s += i->city1()->loc();
    if (i->multiTransportOrigin() != nullptr && i->city1()->loc() != i->multiTransportOrigin()->loc())
      s += '/' + i->multiTransportOrigin()->loc();
    s += '-';
    s += i->city2()->loc();
    if (i->multiTransportDestination() != nullptr &&
        i->city2()->loc() != i->multiTransportDestination()->loc())
      s += '/' + i->multiTransportDestination()->loc();
    s += ' ';
  }
  LOG4CXX_DEBUG(logger, "apply called " << s);
}
}

bool
SouthAtlanticTPMExclusion::apply(MileageRoute& mRoute) const
{
  if (mRoute.mileageRouteItems().size() < 2) // There is no enough segments to apply
    return false;

  // new code goes here
  if (UNLIKELY(IS_DEBUG_ENABLED(logger)))
    logParms(mRoute);

  // need datahandle for new process validation
  if (UNLIKELY(nullptr == mRoute.dataHandle()))
    return false;

  Diag452Collector* diag = dynamic_cast<Diag452Collector*>(mRoute.diagnosticHandle());

  if (UNLIKELY(diag))
    diag->printHeader();

  MileageRoute mr;
  std::vector<std::pair<uint8_t, uint8_t> > idxMap;
  cloneMileageRoute(mRoute, mr, idxMap, *mRoute.dataHandle());
  mr.southAtlanticExclusion() = process(mr, diag);
  copyResponseMileageRoute(mr, mRoute, idxMap);

  if (UNLIKELY(diag))
    diag->printFooter();

  return mRoute.southAtlanticExclusion();
}

bool
SouthAtlanticTPMExclusion::process(MileageRoute& mRoute, Diag452Collector* diag) const
{
  bool ret = false;

  if (UNLIKELY(diag))
    diag->printFareMarketHeader(mRoute);

  const std::vector<TPMExclusion*>& tpms = getTPMExclus(mRoute);

  std::vector<TPMExclusion*>::const_iterator it = tpms.begin();
  std::vector<TPMExclusion*>::const_iterator ie = tpms.end();

  // try to match each exclusion in the sequence
  for (; it != ie; it++)
  {
    SouthAtlanticTPMExclusion::TPMExclusionFailCode failCode = matchTPMExclusion(**it, mRoute);

    if (checkCodeAndDoDiag(diag, mRoute, *it, failCode, ret))
      return ret;
  }
  return ret;
}
bool
SouthAtlanticTPMExclusion::checkCodeAndDoDiag(
    Diag452Collector* diag,
    MileageRoute& mRoute,
    TPMExclusion* tpm,
    const SouthAtlanticTPMExclusion::TPMExclusionFailCode failCode,
    bool& returnCode) const
{
  bool allDiag = false;

  // first, diag processing
  if (UNLIKELY(diag))
  {
    bool matchSeq = true;
    allDiag = diag->rootDiag()->diagParamMapItem(Diagnostic::DISPLAY_DETAIL) == "ALL";
    std::string seqReq = diag->rootDiag()->diagParamMapItem(Diagnostic::SEQ_NUMBER);
    if (seqReq.length())
    {
      std::ostringstream seq;
      seq << tpm->seqNo();
      matchSeq = (seqReq == seq.str());
    }
    // if match sequence and not fail on CRS (overridable by DDALL)
    if (matchSeq && ((failCode != FAILED_CRS && failCode != FAILED_MULTIHOST) || allDiag))
      *diag << *tpm << failCode;
  }

  // check return code if match
  if (MATCHED == failCode)
  {
    if (!returnCode)
    {
      mRoute.tpmExclusion() = tpm;
      returnCode = true;
    }
    // if DDALL then dispaly all records from TPM Exclusion, otherwise, finish on first match
    if (allDiag)
      return false;

    // else return true - done with the loop
    return true;
  }
  return false;
}

SouthAtlanticTPMExclusion::TPMExclusionFailCode
SouthAtlanticTPMExclusion::matchTPMExclusion(TPMExclusion& tpm, MileageRoute& mRoute) const
{
  MileageRouteItems::iterator sec1, sec2;

  if (UNLIKELY(!matchCRS(mRoute, tpm)))
    return FAILED_CRS;

  if (UNLIKELY(!matchMultiHost(mRoute, tpm)))
    return FAILED_MULTIHOST;

  if (!matchGlobalDirection(mRoute, tpm))
    return FAILED_GI;

  if (!matchYY(mRoute, tpm))
    return FAILED_NOT_APPL_TO_YY;

  if (UNLIKELY(!matchOlineSvcOnly(mRoute, tpm)))
    return FAILED_ONLINE_SERVICE_ONLY;

  if (!matchLocation1(mRoute, tpm))
    return FAILED_FC_LOC_1;

  if (!matchLocation2(mRoute, tpm))
    return FAILED_FC_LOC_2;

  if (!matchSector1(mRoute, tpm, sec1))
    return FAILED_SR_1;

  if (!matchSectorAppl(mRoute, tpm, sec1, true))
    return FAILED_SR_1_APPL;

  if (!matchSector2(mRoute, tpm, sec2))
    return FAILED_SR_2;

  // is sector1<sector2
  bool asc = sec1 < sec2;

  // are sectors consecutive, if not, then matched secotr 2 is invalid
  if (!matchConsecutive(mRoute, asc, sec1, sec2))
    return FAILED_SR_2;

  if (!matchSectorAppl(mRoute, tpm, sec2, false))
    return FAILED_SR_2_APPL;

  if (!matchViaPointOfIntrestr(mRoute, asc, tpm, sec1, sec2))
    return FAILED_VIA_POINT_RES;

  if (!matchConsecMustBeOnGov(mRoute, tpm, asc, sec1, sec2))
    return FAILED_CSR_ONLINE_ON_GOV;

  if (!matchSurfacePermitted(mRoute, tpm, asc, sec1, sec2))
    return FAILED_SURFACE_PER;

  markSectors(mRoute, asc, sec1, sec2);

  if (!tpm.carrier().empty())
    mRoute.specificCXR() = true;

  return MATCHED;
}

bool
SouthAtlanticTPMExclusion::matchGlobalDirection(MileageRoute& mRoute, const TPMExclusion& tpm) const
{
  return (tpm.globalDir() == GlobalDirection::ZZ) || (tpm.globalDir() == mRoute.globalDirection());
}

bool
SouthAtlanticTPMExclusion::matchYY(MileageRoute& mRoute, const TPMExclusion& tpm) const
{
  return ((tpm.notApplToYY() == NO) || !mRoute.isYYFare());
}

bool
SouthAtlanticTPMExclusion::matchOlineSvcOnly(MileageRoute& mRoute, const TPMExclusion& tpm) const
{
  if (UNLIKELY(tpm.onlineSrvOnly() == YES))
  {
    MileageRouteItems::iterator it = mRoute.mileageRouteItems().begin();
    MileageRouteItems::iterator ie = mRoute.mileageRouteItems().end();
    for (; it != ie; it++)
    {
      // skip surface
      if (it->isSurface())
        continue;
      if (it->segmentCarrier() != mRoute.governingCarrier())
        return false;
    }
  }
  return true;
}

bool
SouthAtlanticTPMExclusion::matchCRS(MileageRoute& mRoute, const TPMExclusion& tpm) const
{
  if (UNLIKELY(tpm.userApplType() == CRS_USER_APPL))
    return !mRoute.crs().empty() && tpm.userAppl() == mRoute.crs();

  return true;
}
bool
SouthAtlanticTPMExclusion::matchMultiHost(MileageRoute& mRoute, const TPMExclusion& tpm) const
{
  if (UNLIKELY(tpm.userApplType() == MULTIHOST_USER_APPL))
    return tpm.userAppl() == mRoute.multiHost();

  return true;
}
bool
SouthAtlanticTPMExclusion::matchLocation(const Loc* l,
                                         const LocTypeCode& locType,
                                         const LocCode& loc,
                                         MileageRoute& mRoute,
                                         CarrierCode cxr) const
{
  return LocUtil::isInLoc(*l,
                          locType,
                          loc,
                          Vendor::SABRE,
                          MANUAL,
                          LocUtil::OTHER,
                          GeoTravelType::International,
                          "",
                          mRoute.ticketingDT());
}

const std::vector<TPMExclusion*>&
SouthAtlanticTPMExclusion::getTPMExclus(MileageRoute& mRoute) const
{
  return mRoute.dataHandle()->getTPMExclus(mRoute.governingCarrier());
}
bool
SouthAtlanticTPMExclusion::matchLocation1(MileageRoute& mRoute, const TPMExclusion& tpm) const
{
  if (LIKELY(tpm.loc1type() != LOCTYPE_NONE))
  {
    // need some records to do validation
    if (UNLIKELY(mRoute.mileageRouteItems().empty()))
      return false;

    // use FC directionality
    const Loc* origin = mRoute.isOutbound() ? mRoute.mileageRouteItems().front().city1()
                                            : mRoute.mileageRouteItems().back().city2();
    const Loc* dest = mRoute.isOutbound() ? mRoute.mileageRouteItems().back().city2()
                                          : mRoute.mileageRouteItems().front().city1();
    bool match = false;

    match = matchLocation(origin, tpm.loc1type(), tpm.loc1(), mRoute, mRoute.governingCarrier());
    if (!match && (tpm.directionality() == BETWEEN))
      match = matchLocation(dest, tpm.loc1type(), tpm.loc1(), mRoute, mRoute.governingCarrier());
    return match;
  }
  return true;
}
bool
SouthAtlanticTPMExclusion::matchLocation2(MileageRoute& mRoute, const TPMExclusion& tpm) const
{
  if (LIKELY(tpm.loc2type() != LOCTYPE_NONE))
  {
    // need some records to do validation
    if (mRoute.mileageRouteItems().empty())
      return false;

    // use FC directionality
    const Loc* origin = mRoute.isOutbound() ? mRoute.mileageRouteItems().front().city1()
                                            : mRoute.mileageRouteItems().back().city2();
    const Loc* dest = mRoute.isOutbound() ? mRoute.mileageRouteItems().back().city2()
                                          : mRoute.mileageRouteItems().front().city1();
    bool match = false;

    match = matchLocation(dest, tpm.loc2type(), tpm.loc2(), mRoute, mRoute.governingCarrier());
    if (!match && (tpm.directionality() == BETWEEN))
      match = matchLocation(origin, tpm.loc2type(), tpm.loc2(), mRoute, mRoute.governingCarrier());
    return match;
  }
  return true;
}
bool
SouthAtlanticTPMExclusion::matchSector1(MileageRoute& mRoute,
                                        const TPMExclusion& tpm,
                                        MileageRouteItems::iterator& sec1) const
{

  MileageRouteItems::iterator it = mRoute.mileageRouteItems().begin();
  MileageRouteItems::iterator ie = mRoute.mileageRouteItems().end();
  for (; it != ie; it++)
  {
    const Loc* city1 = mRoute.isOutbound() ? it->city1() : it->city2();
    const Loc* city2 = mRoute.isOutbound() ? it->city2() : it->city1();

    if (matchLocation(city1, tpm.sec1Loc1Type(), tpm.sec1Loc1(), mRoute, it->segmentCarrier()) &&
        matchLocation(city2, tpm.sec1Loc2Type(), tpm.sec1Loc2(), mRoute, it->segmentCarrier()))
    {
      sec1 = it;
      return true;
    }
    // We need to take into consideration Direction.
    if ((tpm.directionality() == BETWEEN) &&
        (matchLocation(city2, tpm.sec1Loc1Type(), tpm.sec1Loc1(), mRoute, it->segmentCarrier()) &&
         matchLocation(city1, tpm.sec1Loc2Type(), tpm.sec1Loc2(), mRoute, it->segmentCarrier())))
    {
      sec1 = it;
      return true;
    }
  }
  return false;
}
bool
SouthAtlanticTPMExclusion::matchSector2(MileageRoute& mRoute,
                                        const TPMExclusion& tpm,
                                        MileageRouteItems::iterator& sec2) const
{

  MileageRouteItems::iterator it = mRoute.mileageRouteItems().begin();
  MileageRouteItems::iterator ie = mRoute.mileageRouteItems().end();
  for (; it != ie; it++)
  {

    const Loc* city1 = mRoute.isOutbound() ? it->city1() : it->city2();
    const Loc* city2 = mRoute.isOutbound() ? it->city2() : it->city1();

    if (matchLocation(city1, tpm.sec2Loc1Type(), tpm.sec2Loc1(), mRoute, it->segmentCarrier()) &&
        matchLocation(city2, tpm.sec2Loc2Type(), tpm.sec2Loc2(), mRoute, it->segmentCarrier()))
    {
      sec2 = it;
      return true;
    }
    // We need to take into consideration Direction.
    if ((tpm.directionality() == BETWEEN) &&
        (matchLocation(city2, tpm.sec2Loc1Type(), tpm.sec2Loc1(), mRoute, it->segmentCarrier()) &&
         matchLocation(city1, tpm.sec2Loc2Type(), tpm.sec2Loc2(), mRoute, it->segmentCarrier())))
    {
      sec2 = it;
      return true;
    }
  }
  return false;
}
bool
SouthAtlanticTPMExclusion::matchConsecutive(MileageRoute& mRoute,
                                            bool asc,
                                            const MileageRouteItems::iterator& sec1,
                                            const MileageRouteItems::iterator& sec2) const
{
  MileageRouteItems::iterator it = asc ? sec1 : sec2;
  MileageRouteItems::iterator ie = asc ? sec2 : sec1;

  // between sector1 and sector2 can be 0 or 1 segment
  return (it + 1 == ie) || (it + 2 == ie);
}
bool
SouthAtlanticTPMExclusion::matchSectorAppl(MileageRoute& mRoute,
                                           const TPMExclusion& tpm,
                                           const MileageRouteItems::iterator& sec,
                                           bool check_Sec1_appl) const
{
  bool rc = true;
  if (check_Sec1_appl)
  {
    // for nonstop, there can;t be any hiden loc
    if (tpm.sec1Appl() == 'N')
      rc = sec->hiddenLocs().empty();
  }
  else
  {
    // for nonstop, there can;t be any hiden loc
    if (tpm.sec2Appl() == 'N')
      rc = sec->hiddenLocs().empty();
  }
  return rc;
}
bool
SouthAtlanticTPMExclusion::matchViaPointOfIntrestr(MileageRoute& mRoute,
                                                   bool asc,
                                                   const TPMExclusion& tpm,
                                                   const MileageRouteItems::iterator& sec1,
                                                   const MileageRouteItems::iterator& sec2) const
{
  // T  Sector 1 Location 2 must be equal to Sector 2 Location 1;
  // G  GOV CXR single plane service required be-tween sectors;
  // A  any single plane service between Sector 1 and Sector 2 permitted.
  MileageRouteItems::iterator it1 = asc ? sec1 : sec2;
  MileageRouteItems::iterator it2 = asc ? sec2 : sec1;

  switch (tpm.viaPointRest())
  {
  case 'T':
    return multiCityDest(*it1) == multiCityOrig(*it2);

  case 'G':
    if (++it1 == it2)
      return false;
    return it1->segmentCarrier() == mRoute.governingCarrier() || it1->isSurface();

  case 'A':
    return it1 + 1 != it2;

  default:
    return false;
  };
  return true;
}
bool
SouthAtlanticTPMExclusion::matchConsecMustBeOnGov(MileageRoute& mRoute,
                                                  const TPMExclusion& tpm,
                                                  bool asc,
                                                  const MileageRouteItems::iterator& sec1,
                                                  const MileageRouteItems::iterator& sec2) const
{
  // Y  all travel within Sector 1 and Sector 2 must be via the governing carrier;
  // N  no restriction
  if (tpm.consecMustBeOnGovCxr() == YES)
  {
    MileageRouteItems::iterator it = asc ? sec1 : sec2;
    MileageRouteItems::iterator ie = asc ? sec2 : sec1;
    for (ie++; it != ie; it++)
    {
      if (!it->isSurface() && (it->segmentCarrier() != mRoute.governingCarrier()))
        return false;
    }
  }
  return true;
}

bool
SouthAtlanticTPMExclusion::matchSurfacePermitted(MileageRoute& mRoute,
                                                 const TPMExclusion& tpm,
                                                 bool asc,
                                                 const MileageRouteItems::iterator& sec1,
                                                 const MileageRouteItems::iterator& sec2) const
{
  // N  no surface sector permitted between Sector 1 and Sector 2;
  // Y  no restriction
  if (tpm.surfacePermitted() != NO)
    return true;

  MileageRouteItems::iterator it = asc ? sec1 : sec2;
  MileageRouteItems::iterator ie = asc ? sec2 : sec1;
  for (it++; it != ie; it++)
  {
    if (it->isSurface())
      return false;
  }
  return true;
}

void
SouthAtlanticTPMExclusion::markSectors(MileageRoute& mRoute,
                                       bool asc,
                                       const MileageRouteItems::iterator& sec1,
                                       const MileageRouteItems::iterator& sec2) const
{
  MileageRouteItems::iterator it = asc ? sec1 : sec2;
  MileageRouteItems::iterator ie = asc ? sec2 : sec1;
  for (ie++; it != ie; it++)
    it->southAtlanticExclusion() = true;
}
const LocCode&
SouthAtlanticTPMExclusion::multiCityOrig(const MileageRouteItem& mi)
{
  return mi.origCityOrAirport()->loc();
}
const LocCode&
SouthAtlanticTPMExclusion::multiCityDest(const MileageRouteItem& mi)
{
  return mi.destCityOrAirport()->loc();
}
const MultiTransportRetriever&
SouthAtlanticTPMExclusion::getMultTransretriever() const
{
  return tse::Singleton<MultiTransportRetriever>::instance();
}

void
SouthAtlanticTPMExclusion::cloneMileageRoute(const MileageRoute& src,
                                             MileageRoute& dst,
                                             std::vector<std::pair<uint8_t, uint8_t> >& idxMap,
                                             DataHandle& dataHandle) const
{
  const MultiTransportRetriever& multiTransport = getMultTransretriever();
  // clone mileage route data
  dst.clone(src);
  MileageRouteItems::const_iterator it = src.mileageRouteItems().begin();
  MileageRouteItems::const_iterator ie = src.mileageRouteItems().end();
  uint8_t i = 0, j = 0;
  // create segment index maping between two MileageRoutes
  for (; it != ie; it++, i++)
  {
    MileageRouteItem mit(*it);
    multiTransport.retrieve(mit, dataHandle);

    if (mit.origCityOrAirport()->loc() != mit.destCityOrAirport()->loc())
    {
      dst.mileageRouteItems().push_back(mit);
      idxMap.push_back(std::pair<uint8_t, uint8_t>(i, j));
      j++;
    }
  }
}
void
SouthAtlanticTPMExclusion::copyResponseMileageRoute(
    MileageRoute& src, MileageRoute& dst, const std::vector<std::pair<uint8_t, uint8_t> >& idxMap)
    const
{
  // copy response from temporary mileage route to the original
  dst.specificCXR() = src.specificCXR();
  dst.tpmExclusion() = src.tpmExclusion();
  dst.southAtlanticExceptionApplies() = src.southAtlanticExceptionApplies();
  dst.southAtlanticExclusion() = src.southAtlanticExclusion();
  // use segment index map to copy segment exclusion data "/T"
  std::vector<std::pair<uint8_t, uint8_t> >::const_iterator it = idxMap.begin();
  std::vector<std::pair<uint8_t, uint8_t> >::const_iterator ie = idxMap.end();
  for (; it != ie; it++)
  {
    dst.mileageRouteItems()[it->first].southAtlanticExclusion() =
        src.mileageRouteItems()[it->second].southAtlanticExclusion();
  }
}
}
