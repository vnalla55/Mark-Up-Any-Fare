//-------------------------------------------------------------------
//
//  File:        VendorAtpco.cpp
//  Created:     Nov 17, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Atpco-specific part of add-on construction process
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

#include "AddonConstruction/VendorAtpco.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/AtpcoGatewayPair.h"
#include "AddonConstruction/ConstructedCacheManager.h"
#include "AddonConstruction/ConstructionJob.h"
#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "Common/TSELatencyData.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/Loc.h"
#include "Util/BranchPrediction.h"

using namespace tse;
using namespace std;

static Logger
logger("atseintl.AddonConstruction.VendorAtpco");

VendorAtpco::VendorAtpco() : ConstructionVendor(), _originGeoAppl(' '), _destinationGeoAppl(' ') {}

VendorAtpco::~VendorAtpco() {}

void
VendorAtpco::initialize(ConstructionJob* cjob)
{
  ConstructionVendor::initialize(cjob);

  _trfXrefMap.init(_cJob);
  _combFareClassMap.init(_cJob);

  if (UNLIKELY(_cJob->diag253() != nullptr))
  {
    _trfXrefMap.populate();
  }

  // define origin/destination geo application
  // Sabre My Fares process ignores this parameter

  _originGeoAppl = 'U';
  _destinationGeoAppl = 'U';

  // if origin(destination) is in US/Canada then
  // {
  //   GeoAppl of origin(destination) == 'U';
  //   GeoAppl of the opposite side  == 'N'
  // }
  // else if both origin and destination are not US/Canada then
  // {
  //   if origin.IATA_area == destination.IATA_area then
  //     both GeoAppl == 'U'
  //   else
  //   {
  //     GeoAppl of origin(destination) in lowest IATA area == 'U';
  //     GeoAppl of origin(destination) in highest IATA area == 'N';
  //   }
  // }

  if (_cJob->loc(CP_ORIGIN)->nation() == NATION_US || _cJob->loc(CP_ORIGIN)->nation() == NATION_CA)
    _destinationGeoAppl = 'N';

  else if (_cJob->loc(CP_DESTINATION)->nation() == NATION_US ||
           _cJob->loc(CP_DESTINATION)->nation() == NATION_CA)
    _originGeoAppl = 'N';

  else if (_cJob->loc(CP_ORIGIN)->area() < _cJob->loc(CP_DESTINATION)->area())
    _destinationGeoAppl = 'N';

  else if (_cJob->loc(CP_ORIGIN)->area() > _cJob->loc(CP_DESTINATION)->area())
    _originGeoAppl = 'N';
}

ConstructionVendor*
VendorAtpco::getNewVendor(ConstructionJob& cj)
{
  // lint --e{530}
  VendorAtpco* atpco;

  cj.dataHandle().get(atpco);

  atpco->initialize(&cj);

  return atpco;
}

CombFareClassMap*
VendorAtpco::getCombFareClassMap()
{
  return &_combFareClassMap;
}

std::shared_ptr<GatewayPair>
VendorAtpco::getNewGwPair()
{
  ConstructedCacheManager& ccm = ConstructedCacheManager::instance();

  AtpcoGatewayPair* atpcoGWPair = nullptr;

  ccm.get(atpcoGWPair);

  std::shared_ptr<AtpcoGatewayPair> newGwPair(atpcoGWPair);

  return newGwPair;
}

// the function matches together add-on tariffs of origin and
// destination add-on fares to see whether we can build
// double-ended fares for this gateway pair or not.

bool
VendorAtpco::matchAddonFares(AddonFareCortegeVec::iterator& firstOrigFare,
                             AddonFareCortegeVec::iterator& firstDestFare)
{
  bool okToBuildGwPair = false;

  AddonFareCortegeVec::iterator origFare = firstOrigFare;
  AddonFareCortegeVec::iterator endOfOrigMarket =
      firstOrigFare + (*firstOrigFare)->gatewayFareCount();

  AddonFareCortegeVec::iterator destFare;
  AddonFareCortegeVec::iterator endOfDestMarket =
      firstDestFare + (*firstDestFare)->gatewayFareCount();

  for (; origFare != endOfOrigMarket; origFare++)
  {
    destFare = firstDestFare;
    for (; destFare != endOfDestMarket; destFare++)
    {
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
      if (((*origFare)->addonFare()->owrt() == (*destFare)->addonFare()->owrt()) &&
          _trfXrefMap.matchAddonTariffs((*origFare)->addonFare()->addonTariff(),
                                        (*destFare)->addonFare()->addonTariff()))
#else
      if (_trfXrefMap.matchAddonTariffs((*origFare)->addonFare()->addonTariff(),
                                        (*destFare)->addonFare()->addonTariff()) &&
          ((*origFare)->addonFare()->owrt() == (*destFare)->addonFare()->owrt()))

#endif
      {
        okToBuildGwPair = true;

        break;
      }
    }
  }

  return okToBuildGwPair;
}

AddonZoneStatus
VendorAtpco::validateZones(const AddonFareInfo& addonFare,
                           ConstructionPoint cp,
                           TSEDateIntervalVec& zoneIntervals)
{
  const AddonZone zone = addonFare.arbZone();
  const TariffNumber tariff = addonFare.addonTariff();
  const TSEDateInterval& afi = addonFare.effInterval();

  AddonZoneMap& zoneMap = (cp == CP_ORIGIN ? _origAddonZones : _destAddonZones);

  if (!zoneMap.isPopulated())
    zoneMap.populate(*_cJob, cp);

  // this latency item is called a very large number of times, and
  // it slows processing down. Thus it is commented-out.
  // TSELatencyData metrics( _cJob->trx(), "ATPCO ADDON ZONE VALIDATION" );

  if (zoneMap.validateZones(zone, tariff, afi, zoneIntervals))
    if (LIKELY(!zoneIntervals.empty()))
      return AZ_PASS;

  const TrfXrefMap::TariffNumberSet* tns = _trfXrefMap.getAssociatedTariffSet(tariff);

  if (tns != nullptr)
  {
    TrfXrefMap::TariffNumberSet::const_iterator i = tns->begin();
    TrfXrefMap::TariffNumberSet::const_iterator ie = tns->end();

    for (; i != ie; ++i)
    {
      if (zoneMap.validateZones(zone, *i, afi, zoneIntervals))
        if (LIKELY(!zoneIntervals.empty()))
          return AZ_PASS;
    }
  }

  return (zoneIntervals.empty() ? AZ_FAIL : AZ_PASS);
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

bool
VendorAtpco::isGlobalDirValid(const CarrierCode& cxr,
                              TariffNumber addonTrf,
                              GlobalDirection globalDir)
{
  return _trfXrefMap.isGlobalDirValid(cxr, addonTrf, globalDir);
}

#endif

