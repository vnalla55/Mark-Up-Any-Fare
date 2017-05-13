//-------------------------------------------------------------------
//
//  File:        SmfGatewayPair.cpp
//  Created:     May 24, 2006
//  Authors:     Vadim Nikushin
//
//  Description: Class represents data and methods of SMF
//               process to build single- and double- ended
//               constructed fares
//
//  Copyright Sabre 2006
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

#include "AddonConstruction/SmfGatewayPair.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/ConstructedFare.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/VendorSmf.h"
#include "DBAccess/AddonFareInfo.h"

using namespace tse;
using namespace std;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

FareMatchCode
SmfGatewayPair::matchAddonAndSpecified(AddonFareCortege& addonFare,
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

  if (_cJob->dataHandle().isHistorical())
  {
    if (!cfValid.defineIntersectionH(addonFare.effInterval(), sfi.effInterval()))
      return FM_DATE_INTERVAL_MISMATCH;
  }
  else
  {
    if (!cfValid.defineIntersection(addonFare.effInterval(), sfi.effInterval()))
      return FM_DATE_INTERVAL_MISMATCH;
  }

  VendorSmf& vendor = *static_cast<VendorSmf*>(_vendor);
  Indicator specFareOWRT = sfi.owrt();

  // match OWRT

  FareMatchCode matchResult = matchOWRT(afi.owrt(), specFareOWRT);

  // match add-on direction  (add-on footnotes)

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchAddonDirection(addonFare, sfi, oppositeSpecified, isOriginAddon);

  // match tariffs

  if (matchResult == FM_GOOD_MATCH)
    matchResult = vendor.trfXrefMap().matchFareAndAddonTariff(
        sfi.fareTariff(), afi.addonTariff(), _cJob->dataHandle().isHistorical(), cfValid);

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
      // OWRT in PO5 table could be only:
      //     1 == ONE_WAY
      //     2 == ROUND_TRIP
      //
      // so, we need to modify spec. fare OWRT in certain cases

      if (specFareOWRT == ONE_WAY_MAYNOT_BE_DOUBLED)
        specFareOWRT = ONE_WAY_MAY_BE_DOUBLED;

      if (!_cJob->dataHandle().isHistorical())
      {
        const AddonFareClasses* addonFareClasses =
          isOriginAddon ? specFare._origAddonFareClasses : specFare._destAddonFareClasses;
        if (addonFareClasses)
        {
          matchResult = matchAddonFareClass(addonFare, *addonFareClasses, cfValid);
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
              matchResult = matchAddonFareClass(addonFare, *addonFareClasses, cfValid);
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
        if (LIKELY(cmap != nullptr))
        {
          matchResult = cmap->matchFareClassesHistorical(sfi,
                                                         addonFare,
                                                         ' ',
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
      validInterval.showCombFareClassInterval(isOriginAddon);
    }
    else
    {
      matchResult = FM_GOOD_MATCH;
    }
  }

  return matchResult;
}


FareMatchCode
SmfGatewayPair::matchAddonFareClass(const AddonFareCortege& fare,
                                    const AddonFareClasses& fareClasses,
                                    TSEDateInterval& validDI) const
{
  for (AddonCombFareClassInfo* fcl: fareClasses)
  {
    if (fcl->addonFareClass() == fare.addonFare()->fareClass()[0] &&
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

#else

FareMatchCode
SmfGatewayPair::matchAddonAndSpecified(AddonFareCortege& addonFare,
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

  if (_cJob->dataHandle().isHistorical())
  {
    if (!cfValid.defineIntersectionH(addonFare.effInterval(), constrFare.prevEffInterval()))
      return FM_DATE_INTERVAL_MISMATCH;
  }
  else
  {
    if (!cfValid.defineIntersection(addonFare.effInterval(), constrFare.prevEffInterval()))
      return FM_DATE_INTERVAL_MISMATCH;
  }

  VendorSmf& vendor = *static_cast<VendorSmf*>(_vendor);
  Indicator specFareOWRT = sfi.owrt();

  // match OWRT

  FareMatchCode matchResult = matchOWRT(afi.owrt(), specFareOWRT);

  // match add-on direction  (add-on footnotes)

  if (matchResult == FM_GOOD_MATCH)
    matchResult = matchAddonDirection(addonFare, constrFare, isOriginAddon);

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

    // SMF process doesnt care about geoappl index.
    // we use blank instead of real value here

    CombFareClassMap* cmap(_vendor ? _vendor->getCombFareClassMap() : nullptr);
    if (cmap != nullptr)
    {
      matchResult =
          cmap->matchFareClasses(constrFare, addonFare, ' ', specFareOWRT, fClassCombRecords);
    }
  }

  // define intersection between TrfXref and FareClassComb intervals

  if (matchResult == FM_GOOD_MATCH || matchResult == FM_COMB_FARE_EXACT_MATCH)
    defineIntervalIntersection(
        cfValid, xrefRecords, fClassCombRecords, isOriginAddon, constrFareIntervals, matchResult);

  return matchResult;
}

#endif

