//-------------------------------------------------------------------
//
//  File:        VendorSmf.cpp
//  Created:     May 23, 2006
//  Authors:     Vadim Nikushin
//
//  Description: Smf-specific part of add-on construction process
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

#include "AddonConstruction/VendorSmf.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/ConstructedCacheManager.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/SmfGatewayPair.h"
#include "Common/FallbackUtil.h"
#include "Common/Logger.h"
#include "Common/TseConsts.h"
#include "Common/TSELatencyData.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/AddonFareInfo.h"
#include "Diagnostic/Diag253Collector.h"
#include "Util/BranchPrediction.h"

namespace tse
{
FALLBACK_DECL(removeDynamicCastForAddonConstruction);

static Logger
logger("atseintl.AddonConstruction.VendorSmf");

VendorSmf::VendorSmf() : ConstructionVendor() {}

void
VendorSmf::initialize(ConstructionJob* cjob)
{
  ConstructionVendor::initialize(cjob);

  _trfXrefMap.init(_cJob);
  _combFareClassMap.init(_cJob);

  _origAddonZones.initialize(_cJob);
  _destAddonZones.initialize(_cJob);

  Diag253Collector* dc{ nullptr };
  if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
  {
    dc = _cJob->diagnostic<Diag253Collector>();
  }
  else
  {
    dc = _cJob->diag253();
  }
  if (UNLIKELY(nullptr != dc))
  {
    _trfXrefMap.populate();
  }

  // SMF doesn't use geo application indicator.
  // nothing to define here then
}

CombFareClassMap*
VendorSmf::getCombFareClassMap()
{
  return &_combFareClassMap;
}

ConstructionVendor*
VendorSmf::getNewVendor(ConstructionJob& cj)
{
  // lint --e{530}
  VendorSmf* smf;

  cj.dataHandle().get(smf);

  smf->initialize(&cj);

  return smf;
}

std::shared_ptr<GatewayPair>
VendorSmf::getNewGwPair()
{
  ConstructedCacheManager& ccm = ConstructedCacheManager::instance();

  SmfGatewayPair* smfGWPair = nullptr;

  ccm.get(smfGWPair);

  std::shared_ptr<SmfGatewayPair> newGwPair(smfGWPair);

  return newGwPair;
}

AddonZoneStatus
VendorSmf::validateZones(const AddonFareInfo& addonFare,
                         ConstructionPoint cp,
                         TSEDateIntervalVec& zoneIntervals)
{
  TSELatencyData metrics(_cJob->trx(), "SMF ADDON ZONE VALIDATION");

  AddonZoneMapSmf& zoneMap = (cp == CP_ORIGIN ? _origAddonZones : _destAddonZones);

  zoneMap.validateZones(
      cp, addonFare.arbZone(), addonFare.addonTariff(), addonFare.effInterval(), zoneIntervals);

  return (zoneIntervals.empty() ? AZ_FAIL : AZ_PASS);
}


#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

bool
VendorSmf::isGlobalDirValid(const CarrierCode& cxr,
                            TariffNumber addonTrf,
                            GlobalDirection globalDir)
{
  return _trfXrefMap.isGlobalDirValid(cxr, addonTrf, globalDir);
}

#endif
}
