//-------------------------------------------------------------------
//
//  File:        AddonZoneMapSmf.cpp
//  Created:     May 24, 2006
//  Authors:     Vadim Nikushin
//
//  Description: Class to validate AddonZone for given SMF AddonFare
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

#include "AddonConstruction/AddonZoneMapSmf.h"

#include "AddonConstruction/ConstructionJob.h"
#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/AddonZoneInfo.h"
#include "DBAccess/Loc.h"

using namespace std;

namespace tse
{

static Logger
logger("atseintl.AddonConstruction.AddonZoneMapSmf");

void
AddonZoneMapSmf::initialize(ConstructionJob* cjob)
{
  _cJob = cjob;
}

bool
AddonZoneMapSmf::validateZones(const ConstructionPoint cp,
                               const AddonZone zone,
                               const TariffNumber tariff,
                               const TSEDateInterval& afi,
                               TSEDateIntervalVec& zoneIntervals)
{
  // first to chek if the zone was validated

  const AddonZoneKey azk(zone, tariff);

  ZoneMapI z = _zoneMap.find(azk);
  if (z == _zoneMap.end())
  {
    // zone never been tried before. retrieve and validate it.

    TSEDateInterval zoneInterval;

    bool zoneValid = validateZones(cp, zone, tariff, zoneInterval);

    AddonZoneStatus azs(zoneValid, zoneInterval);
    z = _zoneMap.insert(_zoneMap.begin(), ZoneMapPair(azk, azs));
  }

  if (!z->second.first)
    return false;

  // the zone was validated and date interval exists.
  // build interval intersection for all found records

  // TODO: this code will not work for Historical Add-on Construction

  TSEDateInterval* ti;
  _cJob->dataHandle().get(ti);

  bool ret;
  if (_cJob->dataHandle().isHistorical())
    ret = ti->defineIntersectionH(afi, z->second.second);
  else
    ret = ti->defineIntersection(afi, z->second.second);
  if (ret)
  {
    zoneIntervals.push_back(ti);
    return true;
  }

  return false;
}

bool
AddonZoneMapSmf::validateZones(const ConstructionPoint cp,
                               const AddonZone zone,
                               const TariffNumber tariff,
                               TSEDateInterval& zoneInterval)
{
  TSELatencyData metrics(_cJob->trx(), "ADDON LOAD SMF ZONES");

  // TODO: this code will not work for Historical Add-on Construction

  // SMF sones for add-on construction may include only:
  // 1) records with city (airport) code
  // 2) records with nation code
  //
  // City (airport) can be included and/or excluded.
  // Nation can be included only.

  const std::vector<AddonZoneInfo*> zones = _cJob->dataHandle().getAddOnZone(
      _cJob->vendorCode(), _cJob->carrier(), tariff, zone, _cJob->ticketingDate());

  bool zoneValid = false;

  const Loc& loc = *(_cJob->loc(cp));

  std::vector<AddonZoneInfo*>::const_iterator i = zones.begin();
  std::vector<AddonZoneInfo*>::const_iterator ie = zones.end();

  for (; i != ie; ++i)
  {
    const AddonZoneInfo& azi = **i;

    const LocKey& market = azi.market();
    if (market.locType() == LOCTYPE_CITY)
    {
      if (loc.loc() == market.loc() || loc.city() == market.loc())
      {
        zoneValid = (azi.inclExclInd() == 'I');
        zoneInterval = azi.effInterval();

        break;
      }
    }
    else if (market.locType() == LOCTYPE_NATION)
    {
      if (loc.nation() == market.loc())
      {
        zoneValid = true;
        zoneInterval = azi.effInterval();
      }
    }
  }

  return zoneValid;
}

}
