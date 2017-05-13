//-------------------------------------------------------------------
//
//  File:        AtpcoGatewayPair.cpp
//  Created:     Jun 11, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class represents data and methods of ATPCO
//               process to build single- and double- ended
//               constructed fares
//
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//-------------------------------------------------------------------

#include "AddonConstruction/AtpcoGatewayPair.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/AtpcoConstructedFare.h"
#include "AddonConstruction/CombFareClassMap.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/DateInterval.h"
#include "AddonConstruction/FareDup.h"
#include "AddonConstruction/TrfXrefMap.h"
#include "AddonConstruction/VendorAtpco.h"
#include "Common/TsePrimitiveTypes.h"
#include "Common/TseStringTypes.h"
#include "DBAccess/AddonCombFareClassInfo.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/FareInfo.h"
#include "Diagnostic/Diag257Collector.h"

#include <vector>

using namespace tse;
using namespace std;

ConstructedFare*
AtpcoGatewayPair::getConstructedFare()
{
  AtpcoConstructedFare* cf;

  // lint -e{530}
  _cJob->dataHandle().get(cf);

  return cf;
}

FareMatchCode
AtpcoGatewayPair::matchOWRT(const Indicator addonOwrt, const Indicator specOwrt)
{

  FareMatchCode matchResult = FM_GOOD_MATCH;

  switch (specOwrt)
  {
  case ONE_WAY_MAY_BE_DOUBLED:
  case ONE_WAY_MAYNOT_BE_DOUBLED:
    if (addonOwrt == ROUND_TRIP_MAYNOT_BE_HALVED)
      matchResult = FM_OWRT;

    break;

  case ROUND_TRIP_MAYNOT_BE_HALVED:
    if (addonOwrt != ROUND_TRIP_MAYNOT_BE_HALVED)
      matchResult = FM_OWRT;

    break;

  default:
    matchResult = FM_OWRT;
  }

  return matchResult;
}

namespace
{
struct RoundTripMayNotBeHalved
{
  bool operator()(AddonFareCortege* addonFare)
  {
    const AddonFareInfo& afi = *addonFare->addonFare();
    return afi.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED;
  }
};
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

FareMatchCode
AtpcoGatewayPair::matchAddonAndSpecified(AddonFareCortege& addonFare,
                                         SpecifiedFare& specFare,
                                         const bool oppositeSpecified,
                                         const bool isOriginAddon,
                                         AtpcoFareDateInterval& validInterval)
{
  const AddonFareInfo& afi = *addonFare.addonFare();
  const FareInfo& sfi = *specFare._specFare;

  // trap for debuging. please do not remove =======================>
  //
  //  if( sfi.fareClass() == "YUS" &&
  //      afi.fareClass() == "YUS"
  //      af.gatewayMarket() == "SVG" &&
  //      afi.interiorMarket() == "OSL"
  //    )
  //    cout << "!!!\n";
  //
  // >=================================================== end of trap

  TSEDateInterval cfValid;

  if (UNLIKELY(_cJob->dataHandle().isHistorical()))
  {
    if (!cfValid.defineIntersectionH(addonFare.effInterval(), sfi.effInterval()))
      return FM_DATE_INTERVAL_MISMATCH;
  }
  else
  {
    if (UNLIKELY(!cfValid.defineIntersection(addonFare.effInterval(), sfi.effInterval())))
      return FM_DATE_INTERVAL_MISMATCH;
  }

  VendorAtpco& vendor = *static_cast<VendorAtpco*>(_vendor);

  // match add-on direction  (add-on footnotes)
  FareMatchCode matchResult = matchAddonDirection(addonFare, sfi, oppositeSpecified, isOriginAddon);

  // match tariffs

  if (matchResult == FM_GOOD_MATCH)
  {
    matchResult = vendor.trfXrefMap().matchFareAndAddonTariff(sfi.fareTariff(),
                                                              afi.addonTariff(),
                                                              _cJob->dataHandle().isHistorical(),
                                                              cfValid);
  }

  // match fare classes

  if (matchResult == FM_GOOD_MATCH)
  {
    if (addonFare.addonFareClass() == AddonFareCortege::REGULAR)
    {
      // exact match only
      if (sfi.fareClass() == afi.fareClass())
        matchResult = FM_COMB_FARE_EXACT_MATCH;
      else
        matchResult = FM_COMB_FARE_CLASS;
    }
    // match via AddonCombFareClass table
    else
    {
      Indicator geoAppl = isOriginAddon ? vendor.originGeoAppl() : vendor.destinationGeoAppl();

      if (!_cJob->dataHandle().isHistorical())
      {
        const AddonFareClasses* addonFareClasses =
          isOriginAddon ? specFare._origAddonFareClasses : specFare._destAddonFareClasses;
        if (addonFareClasses)
        {
          matchResult = matchAddonFareClass(addonFare, geoAppl, *addonFareClasses, cfValid);
        }
        else
        {
          CombFareClassMap* cmap(_vendor ? _vendor->getCombFareClassMap() : nullptr);
          if (cmap != nullptr)
          {
            if (const AddonFareClasses* addonFareClasses = cmap->matchSpecifiedFare(sfi))
            {
              if (isOriginAddon)
                specFare._origAddonFareClasses = addonFareClasses;
              else
                specFare._destAddonFareClasses = addonFareClasses;
              matchResult = matchAddonFareClass(addonFare, geoAppl, *addonFareClasses, cfValid);
            }
            else
            {
              matchResult = FM_COMB_FARE_CLASS;
            }
          }
        }
      }
      else //(_cJob->dataHandle().isHistorical())
      {
        CombFareClassMap* cmap(_vendor ? _vendor->getCombFareClassMap() : nullptr);
        if (cmap != nullptr)
        {
          Indicator specFareOWRT = sfi.owrt();
          // OWRT in PO5 table could be only:
          //     1 == ONE_WAY
          //     2 == ROUND_TRIP
          //
          // so, we need to modify spec. fare OWRT in certain cases
          if (specFareOWRT == ONE_WAY_MAYNOT_BE_DOUBLED)
            specFareOWRT = ONE_WAY_MAY_BE_DOUBLED;

          matchResult = cmap->matchFareClassesHistorical(sfi,
                                                         addonFare,
                                                         geoAppl,
                                                         specFareOWRT,
                                                         cfValid);
        }
      }
    }
  }

  // define intersection between TrfXref and FareClassComb intervals
  //
  // Dates in TariffCrossRef and AddonCombFareClass are actually unused.
  // All records in those tables are valid since creation to infinity.
  if (matchResult == FM_GOOD_MATCH || matchResult == FM_COMB_FARE_EXACT_MATCH)
  {
    validInterval.effInterval() = cfValid;
    validInterval.splittedPart() = '0';
    InhibitedDateInterval inhibitDI;
    cfValid.cloneDateInterval(inhibitDI);
    inhibitDI.inhibit() = INHIBIT_N;
    validInterval.setTrfXRefInterval(inhibitDI);
    if (matchResult != FM_COMB_FARE_EXACT_MATCH)
    {
      validInterval.setCombFareClassInterval(cfValid, isOriginAddon);
      validInterval.showCombFareClassInterval(isOriginAddon) = true;
    }
    else
    {
      matchResult = FM_GOOD_MATCH;
    }
  }

  return matchResult;
}


FareMatchCode
AtpcoGatewayPair::matchAddonDirection(AddonFareCortege& addonFare,
                                      const FareInfo& specFare,
                                      const bool oppositeSpecified,
                                      const bool isOriginAddon)
{
  // trap for debuging. please do not remove =======================>
  //
  // if( addonFare->addonFare()->fareClass() == "C*****" &&
  //    addonFare->addonFare()->gatewayMarket() == "NYC" &&
  //    addonFare->addonFare()->interiorMarket() == "SBA"
  //  )
  //  OK = true;
  //
  // >=================================================== end of trap

  // directionality of spec. fire is defined in terms city1/city2.
  // needs to be redifined in terms gateway1/gateway2,
  // so sfDir means:
  //   TO   - to gateway1
  //   FROM - from gateway1

  Directionality sfDir = specFare.directionality();
  if (oppositeSpecified && (sfDir != BOTH))
    sfDir = (sfDir == TO) ? FROM : TO;

  if (UNLIKELY(sfDir == BOTH))
    return FM_GOOD_MATCH;

  // add-on directionality means:
  //   FOOTNOTE_TO   - to gateway
  //   FOOTNOTE_FROM - from gateway
  //
  // combinations for origin add-on:
  // --------------------------+--------------------------+----------
  //     origin add-on dir.    ! spec.fire directionality ! validity
  // --------------------------+--------------------------+----------
  //  FOOTNOTE_FROM (int<-gw1) !     TO   (gw1<-gw2)      ! valid
  //  FOOTNOTE_FROM (int<-gw1) !     FROM (gw1->gw2)      ! not valid
  //  FOOTNOTE_TO   (int->gw1) !     TO   (gw1<-gw2)      ! not valid
  //  FOOTNOTE_TO   (int->gw1) !     FROM (gw1->gw2)      ! valid
  // --------------------------+--------------------------+----------
  //
  // combinations for destination add-on:
  // --------------------------+--------------------------+----------
  //  spec.fire directionality !  destination add-on dir. ! validity
  // --------------------------+--------------------------+----------
  //      TO   (gw1<-gw2)      ! FOOTNOTE_FROM (gw2->int) ! not valid
  //      FROM (gw1->gw2)      ! FOOTNOTE_FROM (gw2->int) ! valid
  //      TO   (gw1<-gw2)      ! FOOTNOTE_TO   (gw2<-int) ! valid
  //      FROM (gw1->gw2)      ! FOOTNOTE_TO   (gw2<-int) ! not valid
  // --------------------------+--------------------------+----------

  bool OK = true;
  if (addonFare.addonFootNote() == AddonFareCortege::FOOTNOTE_TO)
    OK = (isOriginAddon && sfDir == FROM) || (!isOriginAddon && sfDir == TO);

  else if (addonFare.addonFootNote() == AddonFareCortege::FOOTNOTE_FROM)
    OK = (isOriginAddon && sfDir == TO) || (!isOriginAddon && sfDir == FROM);

  return (OK ? FM_GOOD_MATCH : FM_ADDON_DIRECTION);
}


FareMatchCode
AtpcoGatewayPair::matchAddonFareClass(const AddonFareCortege& fare,
                                      Indicator geoAppl,
                                      const AddonFareClasses& fareClasses,
                                      TSEDateInterval& validDI) const
{
  for (AddonCombFareClassInfo* fcl: fareClasses)
  {
    if (fcl->addonFareClass() == fare.addonFare()->fareClass()[0] &&
        (fcl->geoAppl() == ' ' || fcl->geoAppl() == geoAppl) &&
        fcl->expireDate() >= _date)
    {
      if (fcl->createDate() > validDI.createDate())
        validDI.createDate() = fcl->createDate();
      if (LIKELY(!_cJob->fallbackConstructedFareEffectiveDate()))
      {
        if (fcl->effDate() > validDI.effDate())
          validDI.effDate() = fcl->effDate();
      }
      return FM_GOOD_MATCH;
    }
  }
  return FM_COMB_FARE_CLASS;
}


AddonFareCortegeVec::iterator
AtpcoGatewayPair::partition(AddonFareCortegeVec::iterator firstAddon,
                            AddonFareCortegeVec::iterator endOfAddons)
{
  return std::partition(firstAddon, endOfAddons, RoundTripMayNotBeHalved());
}

std::pair<AddonFareCortegeVec::iterator, AddonFareCortegeVec::iterator>
AtpcoGatewayPair::getOwrtMatchingAddonRange(
    const FareInfo& sfi,
    AddonFareCortegeVec::iterator firstAddon,
    AddonFareCortegeVec::iterator endOfAddons,
    AddonFareCortegeVec::iterator roundTripMayNotBeHalvedAddonBound)
{

  AddonFareCortegeVec::iterator firstAddonPartition;
  AddonFareCortegeVec::iterator endOfAddonsPartition;

  if (sfi.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
  {
    firstAddonPartition = firstAddon;
    endOfAddonsPartition = roundTripMayNotBeHalvedAddonBound;
  }
  else
  {
    firstAddonPartition = roundTripMayNotBeHalvedAddonBound;
    endOfAddonsPartition = endOfAddons;
  }

  return std::make_pair(firstAddonPartition, endOfAddonsPartition);
}

#else

FareMatchCode
AtpcoGatewayPair::matchAddonAndSpecified(AddonFareCortege& addonFare,
                                         ConstructedFare& constrFare,
                                         const bool isOriginAddon,
                                         vector<DateIntervalBase*>& constrFareIntervals)
{
  const AddonFareInfo& afi = *addonFare.addonFare();
  const FareInfo& sfi = *constrFare.specifiedFare();

  // trap for debuging. please do not remove =======================>
  //
  //  if( sfi.fareClass() == "YUS" &&
  //      afi.fareClass() == "YUS"
  //      af.gatewayMarket() == "SVG" &&
  //      afi.interiorMarket() == "OSL"
  //    )
  //    cout << "!!!\n";
  //
  // >=================================================== end of trap

  TSEDateInterval cfValid;

  if (UNLIKELY(_cJob->dataHandle().isHistorical()))
  {
    if (!cfValid.defineIntersectionH(addonFare.effInterval(), constrFare.prevEffInterval()))
      return FM_DATE_INTERVAL_MISMATCH;
  }
  else
  {
    if (UNLIKELY(!cfValid.defineIntersection(addonFare.effInterval(), constrFare.prevEffInterval())))
      return FM_DATE_INTERVAL_MISMATCH;
  }

  VendorAtpco& vendor = *static_cast<VendorAtpco*>(_vendor);
  Indicator specFareOWRT = sfi.owrt();

  // match add-on direction  (add-on footnotes)

  FareMatchCode matchResult = matchAddonDirection(addonFare, constrFare, isOriginAddon);

  // match tariffs

  InhibitedDateIntervalVec xrefRecords;
  if (matchResult == FM_GOOD_MATCH)
    matchResult = vendor.trfXrefMap().matchFareAndAddonTariff(
        sfi.fareTariff(), afi.addonTariff(), xrefRecords);

  // match fare classes

  AddonCombFareClassInfoVec fClassCombRecords;
  if (matchResult == FM_GOOD_MATCH)
  {
    // OWRT in PO5 table could be only:
    //     1 == ONE_WAY
    //     2 == ROUND_TRIP
    //
    // so, we need to modify spec. fare OWRT in certain cases

    if (specFareOWRT == ONE_WAY_MAYNOT_BE_DOUBLED)
      specFareOWRT = ONE_WAY_MAY_BE_DOUBLED;

    Indicator geoAppl = isOriginAddon ? vendor.originGeoAppl() : vendor.destinationGeoAppl();
    CombFareClassMap* cmap(_vendor ? _vendor->getCombFareClassMap() : nullptr);
    if (LIKELY(cmap != nullptr))
    {
      matchResult =
          cmap->matchFareClasses(constrFare, addonFare, geoAppl, specFareOWRT, fClassCombRecords);
    }
  }

  // define intersection between TrfXref and FareClassComb intervals

  if (matchResult == FM_GOOD_MATCH || matchResult == FM_COMB_FARE_EXACT_MATCH)
  {
    defineIntervalIntersection(
        cfValid, xrefRecords, fClassCombRecords, isOriginAddon, constrFareIntervals, matchResult);
  }
  return matchResult;
}

FareMatchCode
AtpcoGatewayPair::matchAddonDirection(AddonFareCortege& addonFare,
                                      ConstructedFare& constrFare,
                                      const bool isOriginAddon)
{
  // trap for debuging. please do not remove =======================>
  //
  // if( addonFare->addonFare()->fareClass() == "C*****" &&
  //    addonFare->addonFare()->gatewayMarket() == "NYC" &&
  //    addonFare->addonFare()->interiorMarket() == "SBA"
  //  )
  //  OK = true;
  //
  // >=================================================== end of trap

  // directionality of spec. fire is defined in terms city1/city2.
  // needs to be redifined in terms gateway1/gateway2,
  // so sfDir means:
  //   TO   - to gateway1
  //   FROM - from gateway1

  Directionality sfDir = constrFare.defineDirectionality();

  if (UNLIKELY(sfDir == BOTH))
    return FM_GOOD_MATCH;

  // add-on directionality means:
  //   FOOTNOTE_TO   - to gateway
  //   FOOTNOTE_FROM - from gateway
  //
  // combinations for origin add-on:
  // --------------------------+--------------------------+----------
  //     origin add-on dir.    ! spec.fire directionality ! validity
  // --------------------------+--------------------------+----------
  //  FOOTNOTE_FROM (int<-gw1) !     TO   (gw1<-gw2)      ! valid
  //  FOOTNOTE_FROM (int<-gw1) !     FROM (gw1->gw2)      ! not valid
  //  FOOTNOTE_TO   (int->gw1) !     TO   (gw1<-gw2)      ! not valid
  //  FOOTNOTE_TO   (int->gw1) !     FROM (gw1->gw2)      ! valid
  // --------------------------+--------------------------+----------
  //
  // combinations for destination add-on:
  // --------------------------+--------------------------+----------
  //  spec.fire directionality !  destination add-on dir. ! validity
  // --------------------------+--------------------------+----------
  //      TO   (gw1<-gw2)      ! FOOTNOTE_FROM (gw2->int) ! not valid
  //      FROM (gw1->gw2)      ! FOOTNOTE_FROM (gw2->int) ! valid
  //      TO   (gw1<-gw2)      ! FOOTNOTE_TO   (gw2<-int) ! valid
  //      FROM (gw1->gw2)      ! FOOTNOTE_TO   (gw2<-int) ! not valid
  // --------------------------+--------------------------+----------

  bool OK = true;
  if (addonFare.addonFootNote() == AddonFareCortege::FOOTNOTE_TO)
    OK = (isOriginAddon && sfDir == FROM) || (!isOriginAddon && sfDir == TO);

  else if (addonFare.addonFootNote() == AddonFareCortege::FOOTNOTE_FROM)
    OK = (isOriginAddon && sfDir == TO) || (!isOriginAddon && sfDir == FROM);

  return (OK ? FM_GOOD_MATCH : FM_ADDON_DIRECTION);
}

void
AtpcoGatewayPair::defineIntervalIntersection(const TSEDateInterval& cfValid,
                                             const InhibitedDateIntervalVec& xrefRecords,
                                             const AddonCombFareClassInfoVec& fClassCombRecords,
                                             const bool isOriginAddon,
                                             vector<DateIntervalBase*>& constrFareIntervals,
                                             FareMatchCode& matchResult)
{
  AtpcoFareDateInterval* adi = nullptr;
  Indicator splittedPart = '0';

  InhibitedDateIntervalVec::const_iterator i = xrefRecords.begin();

  for (; i != xrefRecords.end(); ++i)
  {
    if (LIKELY(adi == nullptr))
    {
      // lint --e{413}
      _cJob->dataHandle().get(adi);
      adi->splittedPart() = splittedPart++;
    }

    adi->effInterval() = cfValid;

    if (LIKELY(adi->setTrfXRefInterval(**i)))
    {
      if (matchResult == FM_COMB_FARE_EXACT_MATCH)
        constrFareIntervals.push_back(adi);
      else
      {
        AddonCombFareClassInfoVec::const_iterator j = fClassCombRecords.begin();

        for (; j != fClassCombRecords.end(); ++j)
        {
          if (UNLIKELY(adi == nullptr))
          {
            // lint --e{413}
            _cJob->dataHandle().get(adi);
            adi->splittedPart() = splittedPart++;
          }

          adi->effInterval() = cfValid;
          adi->setTrfXRefInterval(**i);

          if (LIKELY(adi->setCombFareClassInterval((*j)->effInterval(), isOriginAddon)))
          {
            adi->showCombFareClassInterval(isOriginAddon) = true;
            constrFareIntervals.push_back(adi);
            adi = nullptr;
          }
        }
      }
    }
  }

  matchResult = (constrFareIntervals.empty() ? FM_TRF_COMB_INTERVAL_MISMATCH : FM_GOOD_MATCH);
}

AddonFareCortegeVec::iterator
AtpcoGatewayPair::partition(const AddonFareCortegeVec::iterator& firstAddon,
                            const AddonFareCortegeVec::iterator& endOfAddons)
{
  return std::partition(firstAddon, endOfAddons, RoundTripMayNotBeHalved());
}

std::pair<AddonFareCortegeVec::iterator, AddonFareCortegeVec::iterator>
AtpcoGatewayPair::getAddonIterators(
    const FareInfo& sfi,
    const AddonFareCortegeVec::iterator& firstAddon,
    const AddonFareCortegeVec::iterator& endOfAddons,
    const AddonFareCortegeVec::iterator& roundTripMayNotBeHalvedAddonBound)
{

  AddonFareCortegeVec::iterator firstAddonPartition;
  AddonFareCortegeVec::iterator endOfAddonsPartition;

  if (sfi.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
  {
    firstAddonPartition = firstAddon;
    endOfAddonsPartition = roundTripMayNotBeHalvedAddonBound;
  }
  else
  {
    firstAddonPartition = roundTripMayNotBeHalvedAddonBound;
    endOfAddonsPartition = endOfAddons;
  }

  return std::make_pair(firstAddonPartition, endOfAddonsPartition);
}

#endif

