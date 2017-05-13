//-------------------------------------------------------------------
//
//  File:        SitaConstructedFare.cpp
//  Created:     Feb 18, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class represents parts and extra fields for
//               one SITA constructed fare (result of an add-on
//               construction process)
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

#include "AddonConstruction/SitaConstructedFare.h"

#include "AddonConstruction/ConstructedCacheManager.h"
#include "Common/FallbackUtil.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/SITAConstructedFareInfo.h"
#include "Routing/RoutingConsts.h"

using namespace tse;

SitaConstructedFare::SitaConstructedFare() : ConstructedFare(), _throughMPMInd(' ') {}

ConstructedFare*
SitaConstructedFare::clone(DataHandle& dataHandle) const
{
  // lint --e{413}
  SitaConstructedFare* cloneObj = nullptr;
  dataHandle.get(cloneObj);

  clone(*cloneObj);

  return cloneObj;
}

void
SitaConstructedFare::clone(SitaConstructedFare& cloneObj) const
{
  cloneObj._dateIntervals = _dateIntervals;
  cloneObj._throughFareRouting = _throughFareRouting;
  cloneObj._throughMPMInd = _throughMPMInd;
  cloneObj._throughRule = _throughRule;

  ConstructedFare::clone(cloneObj);
}

ConstructedFareInfo*
SitaConstructedFare::cloneToConstructedFareInfo()
{
  // lint --e{413}
  SITAConstructedFareInfo* cfi = nullptr;

  ConstructedCacheManager& ccm = ConstructedCacheManager::instance();

  ccm.get(cfi);

  cloneToConstructedFareInfo(*cfi);

  return cfi;
}

void
SitaConstructedFare::cloneToConstructedFareInfo(ConstructedFareInfo& cfi)
{
  ConstructedFare::cloneToConstructedFareInfo(cfi);

  cfi.fareInfo().effInterval() = _dateIntervals.effInterval();

  // SITA extra fields from specified fare

  SITAFareInfo& toSitaSFI = static_cast<SITAFareInfo&>(cfi.fareInfo());
  SITAConstructedFareInfo& toSitaCFI = static_cast<SITAConstructedFareInfo&>(cfi);

  const SITAFareInfo& sitaSFI = static_cast<const SITAFareInfo&>(*_specifiedFare);

  toSitaSFI.routeCode() = sitaSFI.routeCode();
  toSitaSFI.dbeClass() = sitaSFI.dbeClass();
  toSitaSFI.fareQualCode() = sitaSFI.fareQualCode();
  toSitaSFI.tariffFamily() = sitaSFI.tariffFamily();
  toSitaSFI.cabotageInd() = sitaSFI.cabotageInd();
  toSitaSFI.govtAppvlInd() = sitaSFI.govtAppvlInd();
  toSitaSFI.constructionInd() = sitaSFI.constructionInd();
  toSitaSFI.multiLateralInd() = sitaSFI.multiLateralInd();
  toSitaSFI.viaCityInd() = sitaSFI.viaCityInd();
  toSitaSFI.viaCity() = sitaSFI.viaCity();
  toSitaSFI.airport1() = sitaSFI.airport1();
  toSitaSFI.airport2() = sitaSFI.airport2();

  // SITA extra fields from constructed fare

  toSitaCFI.throughFareRouting() = _throughFareRouting;
  toSitaCFI.throughMPMInd() = _throughMPMInd;
  toSitaCFI.throughRule() = _throughRule;

  if (toSitaCFI.throughFareRouting() != MILEAGE_ROUTING)
    toSitaSFI.routingNumber() = toSitaCFI.throughFareRouting();
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
void
SitaConstructedFare::initialize(ConstructionJob* cj,
                                const FareInfo& sf,
                                const GatewayPair& gw,
                                const bool oppositeSpecified)
{
  ConstructedFare::initialize(cj, sf, gw, oppositeSpecified);
  _dateIntervals.effInterval() = _prevEffInterval;
}

void
SitaConstructedFare::setAddonInterval(const DateIntervalBase& addonInterval,
                                      const bool /*isOriginAddon*/)
{
  _dateIntervals.effInterval().defineIntersection(_dateIntervals.effInterval(),
                                                  addonInterval.effInterval());
}
#else
void
SitaConstructedFare::setAddonInterval(const DateIntervalBase& addonInterval,
                                      const bool isOriginAddon)
{
  const SitaFareDateInterval& di = static_cast<const SitaFareDateInterval&>(addonInterval);

  _dateIntervals.effInterval() = di.effInterval();
}
#endif

