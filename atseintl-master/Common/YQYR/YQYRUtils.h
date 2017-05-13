#pragma once

#include "Common/GlobalDirectionFinderV2Adapter.h"
#include "Common/LocUtil.h"
#include "Common/PaxTypeUtil.h"
#include "Common/TseCodeTypes.h"
#include "Common/TsePrimitiveTypes.h"
#include "DataModel/PricingTrx.h"
#include "DataModel/TravelSeg.h"
#include "DBAccess/MileageSubstitution.h"
#include "DBAccess/PaxTypeMatrix.h"
#include "DBAccess/YQYRFees.h"
#include "Rules/RuleUtil.h"

namespace tse
{
class PricingTrx;

namespace YQYR
{
namespace YQYRUtils
{
inline bool
printDiag(PricingTrx& trx)
{
  return (trx.diagnostic().diagnosticType() == Diagnostic818 ||
          trx.diagnostic().diagnosticType() == Diagnostic827);
}

inline bool
checkLocation(const Loc& loc,
              const LocCode& lcode,
              LocTypeCode ltype,
              const VendorCode& vendor,
              const CarrierCode& cxr,
              const DateTime& ticketingDateTime)
{
  const ZoneType ztype = (ltype == 'Z') ? RESERVED : TAX_ZONE;

  if (ltype == 'U')
    ltype = 'Z';

  return LocUtil::isInLoc(loc,
                          ltype,
                          lcode,
                          vendor,
                          ztype,
                          LocUtil::SERVICE_FEE,
                          GeoTravelType::International,
                          cxr,
                          ticketingDateTime);
}

inline bool
checkLocation(const Loc& loc, const YQYRFees* fee, const DateTime& ticketingDateTime)
{
  return checkLocation(
      loc, fee->posLoc(), fee->posLocType(), fee->vendor(), fee->carrier(), ticketingDateTime);
}

inline bool
validateLocs(const Loc& journeyOrigin,
             const Loc& furthestPoint,
             const YQYRFees* fee,
             const DateTime& ticketingDateTime)
{
  char dir = (fee->journeyLoc1Ind() == 'A') ? '1' : ' ';

  const LocCode& feeJourneyLoc1(fee->journeyLoc1());
  const LocCode& feeJourneyLoc2(fee->journeyLoc2());
  const LocTypeCode feeJourneyLoc1Type(fee->journeyLocType1());
  const LocTypeCode feeJourneyLoc2Type(fee->journeyLocType2());
  const CarrierCode& carrier(fee->carrier());
  const VendorCode& vendor(fee->vendor());

  switch (dir)
  {
  case '1':
    if (feeJourneyLoc1.empty())
    {
      return checkLocation(
          furthestPoint, feeJourneyLoc2, feeJourneyLoc2Type, vendor, carrier, ticketingDateTime);
    }

    if (feeJourneyLoc2.empty())
    {
      return checkLocation(
          journeyOrigin, feeJourneyLoc1, feeJourneyLoc1Type, vendor, carrier, ticketingDateTime);
    }

    return checkLocation(journeyOrigin,
                         feeJourneyLoc1,
                         feeJourneyLoc1Type,
                         vendor,
                         carrier,
                         ticketingDateTime) &&
           checkLocation(furthestPoint,
                         feeJourneyLoc2,
                         feeJourneyLoc2Type,
                         vendor,
                         carrier,
                         ticketingDateTime);

  default:
    if (feeJourneyLoc1.empty())
    {
      return checkLocation(journeyOrigin,
                           feeJourneyLoc2,
                           feeJourneyLoc2Type,
                           vendor,
                           carrier,
                           ticketingDateTime) ||
             checkLocation(furthestPoint,
                           feeJourneyLoc2,
                           feeJourneyLoc2Type,
                           vendor,
                           carrier,
                           ticketingDateTime);
    }

    if (feeJourneyLoc2.empty())
    {
      return checkLocation(journeyOrigin,
                           feeJourneyLoc1,
                           feeJourneyLoc1Type,
                           vendor,
                           carrier,
                           ticketingDateTime) ||
             checkLocation(furthestPoint,
                           feeJourneyLoc1,
                           feeJourneyLoc1Type,
                           vendor,
                           carrier,
                           ticketingDateTime);
    }

    bool result = checkLocation(journeyOrigin,
                                feeJourneyLoc1,
                                feeJourneyLoc1Type,
                                vendor,
                                carrier,
                                ticketingDateTime) &&
                  checkLocation(furthestPoint,
                                feeJourneyLoc2,
                                feeJourneyLoc2Type,
                                vendor,
                                carrier,
                                ticketingDateTime);

    if (result)
      return true;

    return checkLocation(furthestPoint,
                         feeJourneyLoc1,
                         feeJourneyLoc1Type,
                         vendor,
                         carrier,
                         ticketingDateTime) &&
           checkLocation(journeyOrigin,
                         feeJourneyLoc2,
                         feeJourneyLoc2Type,
                         vendor,
                         carrier,
                         ticketingDateTime);
  }

  return false;
}

inline bool
matchFareBasisCode(const std::string& tktFareBasis, const std::string& ptfFareBasis)
{
  // TODO: Copy & Paste, needs polishing.
  const std::size_t tktFareBasisSize = tktFareBasis.size();
  const std::size_t ptfFareBasisSize = ptfFareBasis.size();

  if (ptfFareBasis.empty())
    return false;

  // length of the tkt fare basis before the first '/'
  std::string::size_type tktFbcLength = tktFareBasis.find("/");
  std::string::size_type ptfFbcLength = ptfFareBasis.find("/");

  // if no tkt designator, check only the fare class
  if (tktFbcLength == std::string::npos)
  {
    if (ptfFbcLength != std::string::npos)
      return false;

    if (!RuleUtil::matchFareClass(tktFareBasis.c_str(), ptfFareBasis.c_str()))
      return false;

    return true;
  }

  if (ptfFbcLength == std::string::npos && tktFbcLength != std::string::npos)
    return false;

  // matching the fare basis code
  std::string tktFbc;
  tktFbc.append(tktFareBasis, 0, tktFbcLength);

  std::string ptfFbc;
  ptfFbc.append(ptfFareBasis, 0, ptfFbcLength);

  if (tktFbc != "-" && !RuleUtil::matchFareClass(tktFbc.c_str(), ptfFbc.c_str()))
    return false;

  tktFbcLength++;

  // end of fare basis ?
  if (tktFbcLength >= tktFareBasisSize)
    return true;

  // matching the first ticket designator
  std::string tktDsgBoth = "";
  tktDsgBoth.append(tktFareBasis, tktFbcLength, tktFareBasisSize - tktFbcLength);
  std::string::size_type tktDsgOneLength = tktDsgBoth.find("/");
  std::string tktDsgOne = "";
  tktDsgOne.append(tktFareBasis, tktFbcLength, tktDsgOneLength);

  ptfFbcLength++;
  std::string ptfDsgBoth = "";
  ptfDsgBoth.append(ptfFareBasis, ptfFbcLength, ptfFareBasisSize - ptfFbcLength);
  std::string::size_type ptfDsgOneLength = ptfDsgBoth.find("/");
  std::string ptfDsgOne = "";
  ptfDsgOne.append(ptfFareBasis, ptfFbcLength, ptfDsgOneLength);

  if (tktDsgOneLength == std::string::npos)
  {
    if (!tktDsgOne.empty() && tktDsgOne[tktDsgOne.size() - 1] == '*')
    {
      if (tktDsgOne.size() == 1)
        return true;

      tktDsgOne[tktDsgOne.size() - 1] = '-';
    }

    if (!RuleUtil::matchFareClass(tktDsgOne.c_str(), ptfDsgOne.c_str()))
      return false;

    return true;
  }

  if (ptfDsgOneLength == std::string::npos && tktDsgOneLength != std::string::npos)
    return false;

  if (!tktDsgOne.empty() && tktDsgOne[tktDsgOne.size() - 1] == '*')
  {
    tktDsgOne[tktDsgOne.size() - 1] = '-';
  }

  if (!RuleUtil::matchFareClass(tktDsgOne.c_str(), ptfDsgOne.c_str()))
    return false;

  if (tktDsgOne.empty() && !ptfDsgOne.empty())
    return false;
  // matching the second ticket designator
  tktDsgOneLength++;
  std::string tktDsgTwo = "";
  tktDsgTwo.append(tktDsgBoth, tktDsgOneLength, tktDsgBoth.size() - tktDsgOneLength);
  ptfDsgOneLength++;
  std::string ptfDsgTwo = "";
  ptfDsgTwo.append(ptfDsgBoth, ptfDsgOneLength, ptfDsgBoth.size() - ptfDsgOneLength);
  if (!tktDsgTwo.empty() && tktDsgTwo[tktDsgTwo.size() - 1] == '*')
  {
    if (tktDsgTwo.size() == 1)
      return true;

    tktDsgTwo[tktDsgTwo.size() - 1] = '-';
  }

  if (!RuleUtil::matchFareClass(tktDsgTwo.c_str(), ptfDsgTwo.c_str()))
    return false;
  return true;
}

inline uint32_t
journeyMileage(const Loc* startLoc,
               const Loc* endLoc,
               const std::vector<TravelSeg*>& travelSegs,
               const DateTime& travelDate,
               PricingTrx& trx)
{
  GlobalDirection gd;

  GlobalDirectionFinderV2Adapter::getGlobalDirection(&trx, travelDate, travelSegs, gd);

  const Loc& loc1 = *startLoc;
  const Loc& loc2 = *endLoc;
  return LocUtil::getTPM(loc1, loc2, gd, travelDate, trx.dataHandle());
}

inline uint32_t
journeyMileage(const Loc* originLoc,
               const Loc* destinationLoc,
               const std::vector<GlobalDirection>& globals,
               const DateTime& travelDate,
               PricingTrx& trx)
{
  LocCode originLocCode = originLoc->city().empty() ? originLoc->loc() : originLoc->city();
  LocCode destinationLocCode =
      destinationLoc->city().empty() ? destinationLoc->loc() : destinationLoc->city();

  if (originLocCode == destinationLocCode)
    return 0;

  DataHandle dh(trx.ticketingDate());

  const std::vector<Mileage*>& tpmsTmp =
      dh.getMileage(originLocCode, destinationLocCode, travelDate, TPM);
  if (tpmsTmp.empty())
  {
    const MileageSubstitution* msOrig = dh.getMileageSubstitution(originLocCode, travelDate);
    const MileageSubstitution* msDest = dh.getMileageSubstitution(destinationLocCode, travelDate);
    if (msOrig)
      originLocCode = msOrig->publishedLoc();
    if (msDest)
      destinationLocCode = msDest->publishedLoc();
  }
  const std::vector<Mileage*>& tpms =
      tpmsTmp.empty() ? dh.getMileage(originLocCode, destinationLocCode, travelDate, TPM) : tpmsTmp;

  if (tpms.size() == 1)
  {
    return tpms.front()->mileage();
  }
  std::set<uint32_t> mileages;
  for (const auto mileage : tpms)
  {
    std::vector<GlobalDirection>::const_iterator it =
        std::find(globals.begin(), globals.end(), mileage->globaldir());
    if (it != globals.end())
    {
      mileages.insert(mileage->mileage());
    }
  }

  if (!mileages.empty())
    return *mileages.rbegin();

  // Now attempt to find a MPM
  const std::vector<Mileage*>& mpms =
      dh.getMileage(originLocCode, destinationLocCode, travelDate, MPM);

  if (mpms.size() == 1)
    return TseUtil::getTPMFromMPM(mpms.front()->mileage());

  for (const auto mileage : mpms)
  {
    std::vector<GlobalDirection>::const_iterator it =
        std::find(globals.begin(), globals.end(), mileage->globaldir());
    if (it != globals.end())
      mileages.insert(mileage->mileage());
  }

  if (!mileages.empty())
    return TseUtil::getTPMFromMPM(*mileages.rbegin());

  return TseUtil::greatCircleMiles(*originLoc, *destinationLoc);
}

inline bool
validatePaxType(const PaxTypeCode psgType, const PaxTypeCode feePsgType, PricingTrx& trx)
{
  if (feePsgType == psgType)
    return true;

  if (feePsgType == CHILD && PaxTypeUtil::isATPCoPaxRollupMatch(trx, psgType, feePsgType))
    return true;

  if (psgType != GV1)
    return false;

  const std::vector<const PaxTypeMatrix*>& sabre2ATP = trx.dataHandle().getPaxTypeMatrix(psgType);

  for (const auto paxTypeMatrix : sabre2ATP)
    if (paxTypeMatrix->atpPaxType() == feePsgType)
      return true;

  return false;
}

inline bool
isInternational(const std::vector<TravelSeg*>& segments)
{
  const NationCode origNation = segments.front()->origin()->nation();
  for (const auto segment : segments)
  {
    if (segment->destination()->nation() != origNation)
      return true;
  }

  return false;
}

inline bool
doesReturnToOrigin(const std::vector<TravelSeg*>& segments, const bool international)
{
  if (international)
  {
    const NationCode& origNation = segments.front()->origin()->nation();
    if (origNation == segments.back()->destination()->nation())
      return true;
  }
  else
  {
    if (segments.front()->boardMultiCity() == segments.back()->offMultiCity())
      return true;
  }

  return false;
}

inline bool
doesReturnToOrigin(const std::vector<TravelSeg*>& segments)
{
  return doesReturnToOrigin(segments, isInternational(segments));
}
}
}
}
