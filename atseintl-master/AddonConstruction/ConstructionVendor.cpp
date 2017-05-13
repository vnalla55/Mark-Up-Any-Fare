//-------------------------------------------------------------------
//
//  File:        ConstructionVendor.cpp
//  Created:     May 14, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Base class for any vendor add-on construction process
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

#include "AddonConstruction/ConstructionVendor.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/AtpcoGatewayPair.h"
#include "AddonConstruction/ConstructedCacheDataWrapper.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/GatewayPair.h"
#include "AddonConstruction/SitaGatewayPair.h"
#include "AddonConstruction/VendorAtpco.h"
#include "Common/Global.h"
#include "Common/Logger.h"
#include "Common/Thread/TseRunnableExecutor.h"
#include "Common/TseUtil.h"
#include "DataModel/PricingTrx.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/Loc.h"
#include "Diagnostic/Diag251Collector.h"

namespace tse
{
FALLBACK_DECL(removeDynamicCastForAddonConstruction);

static Logger
logger("atseintl.AddonConstruction.ConstructionVendor");

TseThreadingConst::TaskId ConstructionVendor::_taskId = TseThreadingConst::GATEWAY_TASK;

void
ConstructionVendor::initialize(ConstructionJob* cjob)
{
  _vendor = cjob->vendorCode();
  _cJob = cjob;
}

AddonZoneStatus
ConstructionVendor::addAddonFare(ConstructionPoint cp, const AddonFareInfo& af)
{
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  if (!isGlobalDirValid(af.carrier(), af.addonTariff(), _cJob->getGlobalDirection()) &&
      _cJob->getGlobalDirection() != GlobalDirection::ZZ &&
      _cJob->getGlobalDirection() != GlobalDirection::XX)
    return AZ_FAIL;
#endif

  TSEDateIntervalVec zoneIntervals;

  AddonZoneStatus zs =
      validateZones(af, (cp == CP_ORIGIN ? CP_DESTINATION : CP_ORIGIN), zoneIntervals);

  if (zs == AZ_PASS)
  {
    AddonFareCortege* afc = nullptr;
    Indicator splittedPart = '0';

    TSEDateIntervalVec::iterator i = zoneIntervals.begin();
    TSEDateIntervalVec::iterator ie = zoneIntervals.end();

    for (; i != ie; ++i)
    {
      _cJob->dataHandle().get(afc);

      // lint --e{413}
      afc->initialize(*_cJob, &af, **i, splittedPart++);

      addonFares(cp).push_back(afc);
    }
  }

  return zs;
}

void
ConstructionVendor::addAddonFareRw(const AddonFareInfo& af)
{
  AddonFareCortege* afc = nullptr;
  TSEDateInterval* afi = nullptr;

  _cJob->dataHandle().get(afc);
  _cJob->dataHandle().get(afi);

  afc->initialize(*_cJob, &af, *afi, '0');

  addonFares(CP_ORIGIN).push_back(afc);
  addonFares(CP_DESTINATION).push_back(afc);
}

void
ConstructionVendorTask::performTask()
{
  (*_gatewayPair)->prepareData();
}

// an entry point to build all fares for the vendor

bool
ConstructionVendor::construction(ConstructedCacheDataWrapper& dw)
{
  LOG4CXX_INFO(logger, "Entered ConstructionVendor::process(...) ");

  LOG4CXX_INFO(logger, "Vendor - " << _vendor);
  /*
    LOG4CXX_INFO(logger, _origMarketLoc->addonFareCortege().size()
      <<" AddOnFares for Origin - "<<_origMarketLoc->locCode());

    LOG4CXX_INFO(logger, _destMarketLoc->addonFareCortege().size()
      <<" AddOnFares for Destination - "<<_destMarketLoc->locCode());
  */

  // markup add-on fares

  sortAddonFares();

  bool ok = markUpAddonFares(CP_ORIGIN);

  if (LIKELY(ok))
    ok = markUpAddonFares(CP_DESTINATION);

  if (LIKELY(ok))
  {
    if (LIKELY(!_cJob->trx().getOptions()->isRtw()))
    {
      // build vector of gateway to process single-ended fares
      ok = buildSEGateways(CP_ORIGIN);

      if (LIKELY(ok))
        ok = buildSEGateways(CP_DESTINATION);

      // build vector of gateway to process double-ended fares
      if (LIKELY(ok))
        ok = buildDEGateways();
    }
    else
    {
      ok = buildDEGatewaysRw();
    }
  }

  // process each gateway pair in the vector

  if (LIKELY(ok))
  {
    // loooooop through all gateway pairs

    CacheGatewayPairVec::iterator gatewaysIt = _gateways.begin();

    // optimization workaround - execute database queries in parallel rather than in sequence
    std::vector<ConstructionVendorTask> tasks;
    for (; gatewaysIt != _gateways.end(); ++gatewaysIt)
    {
      tasks.push_back(ConstructionVendorTask(_cJob->trx(), gatewaysIt));
    }

    TseRunnableExecutor taskExecutor(_taskId);
    std::vector<ConstructionVendorTask>::iterator constructionTaskIt;
    for (constructionTaskIt = tasks.begin(); constructionTaskIt != tasks.end();
         ++constructionTaskIt)
    {
      taskExecutor.execute(*constructionTaskIt);
    }
    taskExecutor.wait();

    for (gatewaysIt = _gateways.begin(); gatewaysIt != _gateways.end(); ++gatewaysIt)
    {
      (*gatewaysIt)->process(dw.ccFares());

      (*gatewaysIt)->clear();

      dw.gateways().push_back(*gatewaysIt);
    }

    Diag251Collector* diag251{ nullptr };
    if (!fallback::removeDynamicCastForAddonConstruction(&_cJob->trx()))
    {
      diag251 = _cJob->diagnostic<Diag251Collector>();
    }
    else
    {
      diag251 = _cJob->diag251();
    }
    if (UNLIKELY(diag251 != nullptr))
      diag251->writeVendor(*this);
  }

  return ok;
}

// an entry point to rebuild invalidated fares for the vendor

bool
ConstructionVendor::reconstruction(ConstructedCacheDataWrapper& dw,
                                   CacheGatewayPairVec& gwToReconstruct)
{
  // markup add-on fares and assign them to existing gateways

  sortAddonFares();

  markUpAddonFares(CP_ORIGIN);
  markUpAddonFares(CP_DESTINATION);

  assignFaresToGateways(gwToReconstruct);

  // process each gateway pair in the vector

  CacheGatewayPairVec::iterator gatewaysIt = gwToReconstruct.begin();
  CacheGatewayPairVec::iterator gatewaysItE = gwToReconstruct.end();
  for (; gatewaysIt != gatewaysItE; ++gatewaysIt)
  {
    (*gatewaysIt)->process(dw.ccFares());

    (*gatewaysIt)->clear();
  }

  return true;
}

// the function scans vector of addon fare corteges
// and fills lastGatewayFareNum for each cortege

bool
ConstructionVendor::markUpAddonFares(ConstructionPoint cp)
{
  AddonFareCortegeVec& addonFC = addonFares(cp);
  if (addonFC.empty())
    return true;

  AddonFareCortegeVec::iterator afc = addonFC.begin();
  AddonFareCortegeVec::iterator gwFirstFare = afc;

  LocCode gw;
  unsigned int gatewayFareCount = 0;

  unsigned int seqNum = 0;

  for (; afc != addonFC.end(); afc++)
  {
    (*afc)->seqNum() = seqNum++;

    if ((*afc)->addonFare()->gatewayMarket() != gw)
    {
      if (gatewayFareCount > 0)
      {
        for (; gwFirstFare != afc; gwFirstFare++)
          (*gwFirstFare)->gatewayFareCount() = gatewayFareCount;

        gatewayFareCount = 0;
      }

      gw = (*afc)->addonFare()->gatewayMarket();
    }

    gatewayFareCount++;
  }

  if (LIKELY(gatewayFareCount > 0))
  {
    for (; gwFirstFare != afc; gwFirstFare++)
      (*gwFirstFare)->gatewayFareCount() = gatewayFareCount;
  }

  return true;
}

// the function builds set of gateways to construct single-ended fares
bool
ConstructionVendor::buildSEGateways(ConstructionPoint firstCP)
{
  AddonFareCortegeVec& addonFC = addonFares(firstCP);
  AddonFareCortegeVec::iterator afc = addonFC.begin();

  bool isGw1ConstructPoint = (firstCP == CP_ORIGIN);

  const Loc* locAddOn = nullptr;
  const Loc* otherLoc = nullptr;
  bool isInternational = false;

  while (afc != addonFC.end())
  {
    if (isGw1ConstructPoint)
    {
      locAddOn =
          _cJob->dataHandle().getLoc((*afc)->addonFare()->gatewayMarket(), _cJob->ticketingDate());
      otherLoc = _cJob->loc(CP_DESTINATION);
      isInternational = !locAddOn || !otherLoc || LocUtil::isInternational(*locAddOn, *otherLoc);
    }
    else
    {
      otherLoc = _cJob->loc(CP_ORIGIN);
      locAddOn =
          _cJob->dataHandle().getLoc((*afc)->addonFare()->gatewayMarket(), _cJob->ticketingDate());
      isInternational = !locAddOn || !otherLoc || LocUtil::isInternational(*otherLoc, *locAddOn);
    }

    if (isInternational)
    {
      std::shared_ptr<GatewayPair> newGwPair = getNewGwPair();

      if (isGw1ConstructPoint)
        newGwPair->initialize(_cJob,
                              this,
                              (*afc)->addonFare()->gatewayMarket(),
                              _cJob->loc(CP_DESTINATION)->loc(),
                              isGw1ConstructPoint,
                              !isGw1ConstructPoint,
                              (*afc)->seqNum(),
                              (*afc)->gatewayFareCount(),
                              0,
                              0);
      else
        newGwPair->initialize(_cJob,
                              this,
                              _cJob->loc(CP_ORIGIN)->loc(),
                              (*afc)->addonFare()->gatewayMarket(),
                              isGw1ConstructPoint,
                              !isGw1ConstructPoint,
                              0,
                              0,
                              (*afc)->seqNum(),
                              (*afc)->gatewayFareCount());

      _gateways.push_back(newGwPair);
    }

    afc += (*afc)->gatewayFareCount();
  }

  return true;
}

// the function builds set of gateways to construct double-ended fares

bool
ConstructionVendor::buildDEGateways()
{
  AddonFareCortegeVec& origAFC = addonFares(CP_ORIGIN);
  AddonFareCortegeVec& destAFC = addonFares(CP_DESTINATION);

  if (origAFC.empty() || destAFC.empty())
    return true;

  AddonFareCortegeVec::iterator firstOrigGwFare = origAFC.begin();
  AddonFareCortegeVec::iterator firstDestGwFare;
  const Loc* City1 = nullptr;
  const Loc* City2 = nullptr;
  bool isInternational = false;

  while (firstOrigGwFare != origAFC.end())
  {
    firstDestGwFare = destAFC.begin();
    while (firstDestGwFare != destAFC.end())
    {
      const LocCode& gw1 = (*firstOrigGwFare)->addonFare()->gatewayMarket();

      const LocCode& gw2 = (*firstDestGwFare)->addonFare()->gatewayMarket();

      City1 = _cJob->dataHandle().getLoc(gw1, _cJob->ticketingDate());
      City2 = _cJob->dataHandle().getLoc(gw2, _cJob->ticketingDate());
      isInternational = !City1 || !City2 || LocUtil::isInternational(*City1, *City2);

      bool ok = (gw1 != gw2);

      if (ok && isInternational)
      {
        CacheGatewayPairVec::iterator gwP = _gateways.begin();
        CacheGatewayPairVec::iterator gwPE = _gateways.end();

        for (; gwP != gwPE; ++gwP)
          if (((*gwP)->gateway1() == gw1 && (*gwP)->gateway2() == gw2) ||
              ((*gwP)->gateway1() == gw2 && (*gwP)->gateway2() == gw1))
          {
            ok = false;
            break;
          }
      }

      if (ok && isInternational)
      {
        if (matchAddonFares(firstOrigGwFare, firstDestGwFare))
        {
          std::shared_ptr<GatewayPair> newGwPair = getNewGwPair();

          newGwPair->initialize(_cJob,
                                this,
                                gw1,
                                gw2,
                                true,
                                true,
                                (*firstOrigGwFare)->seqNum(),
                                (*firstOrigGwFare)->gatewayFareCount(),
                                (*firstDestGwFare)->seqNum(),
                                (*firstDestGwFare)->gatewayFareCount());

          _gateways.push_back(newGwPair);
        }
      }

      firstDestGwFare += (*firstDestGwFare)->gatewayFareCount();
    }

    firstOrigGwFare += (*firstOrigGwFare)->gatewayFareCount();
  }

  return true;
}

bool
ConstructionVendor::buildDEGatewaysRw()
{
  AddonFareCortegeVec& origAFC = addonFares(CP_ORIGIN);
  AddonFareCortegeVec& destAFC = addonFares(CP_DESTINATION);

  if (origAFC.empty() || destAFC.empty())
    return true;

  AddonFareCortegeVec::iterator firstOrigGwFare = origAFC.begin();
  AddonFareCortegeVec::iterator firstDestGwFare = destAFC.begin();

  while (firstOrigGwFare != origAFC.end() && firstDestGwFare != destAFC.end())
  {
    const LocCode& gw1 = (*firstOrigGwFare)->addonFare()->gatewayMarket();
    const LocCode& gw2 = (*firstDestGwFare)->addonFare()->gatewayMarket();

    if (gw1 == gw2)
    {
      std::shared_ptr<GatewayPair> newGwPair = getNewGwPair();

      newGwPair->initialize(_cJob,
                            this,
                            gw1,
                            gw2,
                            true,
                            true,
                            (*firstOrigGwFare)->seqNum(),
                            (*firstOrigGwFare)->gatewayFareCount(),
                            (*firstDestGwFare)->seqNum(),
                            (*firstDestGwFare)->gatewayFareCount());

      _gateways.push_back(newGwPair);
    }

    firstDestGwFare += (*firstDestGwFare)->gatewayFareCount();
    firstOrigGwFare += (*firstOrigGwFare)->gatewayFareCount();
  }

  return true;
}

bool
ConstructionVendor::assignFaresToGateways(CacheGatewayPairVec& gwToReconstruct)
{
  AddonFareCortegeVec& origAFC = addonFares(CP_ORIGIN);
  AddonFareCortegeVec& destAFC = addonFares(CP_DESTINATION);

  if (origAFC.empty() && destAFC.empty())
    return true;

  AddonFareCortegeVec::iterator firstGwFare;

  CacheGatewayPairVec::iterator i = gwToReconstruct.begin();
  CacheGatewayPairVec::iterator ie = gwToReconstruct.end();
  for (; i != ie; ++i)
  {
    if ((*i)->isGw1ConstructPoint())
    {
      firstGwFare = origAFC.begin();

      while (firstGwFare != origAFC.end())
      {
        if ((*firstGwFare)->addonFare()->gatewayMarket() == (*i)->gateway1())
        {
          (*i)->gw1FirstFare() = (*firstGwFare)->seqNum();
          (*i)->gw1FareCount() = (*firstGwFare)->gatewayFareCount();

          break;
        }

        firstGwFare += (*firstGwFare)->gatewayFareCount();
      }
    }

    if ((*i)->isGw2ConstructPoint())
    {
      firstGwFare = destAFC.begin();

      while (firstGwFare != destAFC.end())
      {
        if ((*firstGwFare)->addonFare()->gatewayMarket() == (*i)->gateway2())
        {
          (*i)->gw2FirstFare() = (*firstGwFare)->seqNum();
          (*i)->gw2FareCount() = (*firstGwFare)->gatewayFareCount();

          break;
        }

        firstGwFare += (*firstGwFare)->gatewayFareCount();
      }
    }
  }
  return true;
}

bool
addonFareSort(const AddonFareCortege* lptf, const AddonFareCortege* rptf)
{
  if (UNLIKELY(lptf == nullptr))
    return true;

  if (UNLIKELY(rptf == nullptr))
    return false;

  return (lptf->addonFare()->gatewayMarket() < rptf->addonFare()->gatewayMarket());
}

void
ConstructionVendor::sortAddonFares()
{
  sort(addonFares(CP_ORIGIN).begin(), addonFares(CP_ORIGIN).end(), addonFareSort);

  sort(addonFares(CP_DESTINATION).begin(), addonFares(CP_DESTINATION).end(), addonFareSort);
}
}
