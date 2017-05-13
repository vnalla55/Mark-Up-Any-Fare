//-------------------------------------------------------------------
//
//  File:        AddonZoneUtil.cpp
//  Created:     Feb 25, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class to validate AddonZone for given AddonFare
//
//  Copyright Sabre 2005
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

#include "AddonConstruction/AddonZoneMap.h"

#include "AddonConstruction/ConstructionJob.h"
#include "Common/Logger.h"
#include "Common/TSELatencyData.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/AddonZoneInfo.h"
#include "Diagnostic/Diag251Collector.h"
#include "Util/BranchPrediction.h"

using namespace std;
using namespace tse;

namespace tse
{
FALLBACK_DECL(removeDynamicCastForAddonConstruction);
}

static Logger
logger("atseintl.AddonConstruction.AddonZoneMap");

void
AddonZoneMap::populate(ConstructionJob& cJob, const ConstructionPoint constructionPoint)
{
  LOG4CXX_INFO(logger, "Entered AddonZoneMap::populate(...)");

  _cJob = &cJob;

  populate(cJob.od(constructionPoint), false);

  if (cJob.od(constructionPoint) != cJob.odCity(constructionPoint))
    populate(cJob.odCity(constructionPoint), true);

  // diag 251

  Diag251Collector* diag251{ nullptr };
  if (!fallback::removeDynamicCastForAddonConstruction(&cJob.trx()))
  {
    diag251 = cJob.diagnostic<Diag251Collector>();
  }
  else
  {
    diag251 = cJob.diag251();
  }

  if (UNLIKELY(diag251 != nullptr))
  {
    ZoneMap::iterator i = _zoneMap.begin();
    ZoneMap::iterator ie = _zoneMap.end();

    for (; i != ie; ++i)
      diag251->writeArbZone(
          cJob.od(constructionPoint), i->first.zone(), i->first.tariff(), i->second);

    diag251->writeArbZoneFooter();
  }

  LOG4CXX_INFO(logger,
               _zoneMap.size() << " Addon Zones retrieved for " << cJob.vendorCode() << "-"
                               << cJob.carrier() << "-" << cJob.od(constructionPoint));

  _populated = true;
}

bool
AddonZoneMap::validateZones(const AddonZone zone,
                            const TariffNumber tariff,
                            const TSEDateInterval& afi,
                            TSEDateIntervalVec& zoneIntervals) const
{
  // find a range of records for specified tariff & zone

  const AddonZoneKey azk(zone, tariff);

  ZoneMapCIPair zRecs = _zoneMap.equal_range(azk);

  // equal_range returns position where an element can be inserted.
  // but it might not exist as well. so, two more checks...

  if (zRecs.first == _zoneMap.end())
    return false;

  else if (zRecs.first->first != azk)
    return false;

  // build interval intersection for all found records

  TSEDateInterval* ti = nullptr;

  for (ZoneMapCI i = zRecs.first; i != zRecs.second; ++i)
  {
    if (LIKELY(ti == nullptr))
      _cJob->dataHandle().get(ti);

    bool ret;
    if (UNLIKELY(_cJob->dataHandle().isHistorical()))
      ret = ti->defineIntersectionH(afi, i->second);
    else
      ret = ti->defineIntersection(afi, i->second);
    if (LIKELY(ret))
    {
      zoneIntervals.push_back(ti);
      ti = nullptr;
    }
  }

  return true;
}

void
AddonZoneMap::populate(const LocCode& location, bool collapseDuplicates)
{
  const AddonZoneInfoVec& aZones = getAddOnZones(location);

  AddonZoneInfoVec::const_iterator i = aZones.begin();
  AddonZoneInfoVec::const_iterator ie = aZones.end();

  bool addRecord = true;
  for (; i != ie; ++i)
  {
    AddonZoneKey azk((*i)->zone(), (*i)->fareTariff());

    if (collapseDuplicates)
    {
      addRecord = true;

      ZoneMapIPair zRecs = _zoneMap.equal_range(azk);

      if (zRecs.first != _zoneMap.end())
        if (zRecs.first->first == azk)
        {
          // build interval union of new interval
          // with all found records

          TSEDateInterval ti;
          for (ZoneMapI j = zRecs.first; j != zRecs.second; ++j)
            if (LIKELY(ti.defineUnion((*i)->effInterval(), j->second)))
            {
              j->second = ti;
              addRecord = false;
            }
        }
    }

    if (addRecord)
      _zoneMap.insert(std::pair<AddonZoneKey, TSEDateInterval>(azk, (*i)->effInterval()));
  }
}

const AddonZoneInfoVec&
AddonZoneMap::getAddOnZones(const LocCode& location)
{
  if (_cJob->isAtpco())
  {
    TSELatencyData metrics(_cJob->trx(), "ADDON LOAD ATPCO ZONES");

    return _cJob->dataHandle().getAddOnZone(
        _cJob->vendorCode(), _cJob->carrier(), location, _cJob->travelDate());
  }
  else
  {
    TSELatencyData metrics(_cJob->trx(), "ADDON LOAD SITA ZONES");

    return _cJob->dataHandle().getAddonZoneSITA(location, _cJob->vendorCode(), _cJob->carrier());
  }
}
