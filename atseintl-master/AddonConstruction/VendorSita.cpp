//-------------------------------------------------------------------
//
//  File:        VendorSita.cpp
//  Created:     Nov 17, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Sita-specific part of add-on construction process
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
#include "AddonConstruction/VendorSita.h"

#include "AddonConstruction/ConstructedCacheManager.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/SitaGatewayPair.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/AddonFareInfo.h"

using namespace tse;
using namespace std;

static Logger
logger("atseintl.AddonConstruction.VendorSita");

ConstructionVendor*
VendorSita::getNewVendor(ConstructionJob& cj)
{
  VendorSita* sita;

  // lint -e{530}
  cj.dataHandle().get(sita);

  sita->initialize(&cj);

  return sita;
}

std::shared_ptr<GatewayPair>
VendorSita::getNewGwPair()
{
  ConstructedCacheManager& ccm = ConstructedCacheManager::instance();

  SitaGatewayPair* sitaGWPair = nullptr;

  ccm.get(sitaGWPair);

  std::shared_ptr<SitaGatewayPair> newGwPair(sitaGWPair);

  return newGwPair;
}

CombFareClassMap*
VendorSita::getCombFareClassMap()
{
  return nullptr;
}

AddonZoneStatus
VendorSita::validateZones(const AddonFareInfo& addonFare,
                          ConstructionPoint cp,
                          TSEDateIntervalVec& zoneIntervals)
{
  const AddonZone zone = addonFare.arbZone();
  const TariffNumber tariff = addonFare.addonTariff();
  const TSEDateInterval& afi = addonFare.effInterval();

  AddonZoneMap& zoneMap = (cp == CP_ORIGIN ? _origAddonZones : _destAddonZones);

  if (!zoneMap.isPopulated())
    zoneMap.populate(*_cJob, cp);

  TSELatencyData metrics(_cJob->trx(), "SITA ADDON ZONE VALIDATION");

  zoneMap.validateZones(zone, tariff, afi, zoneIntervals);

  if (UNLIKELY(zoneIntervals.empty() && tariff != 0))
    zoneMap.validateZones(zone, 0, afi, zoneIntervals);

  return (zoneIntervals.empty() ? AZ_FAIL : AZ_PASS);
}
