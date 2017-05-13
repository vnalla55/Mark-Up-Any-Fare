//-------------------------------------------------------------------
//
//  File:        AtpcoConstructedFare.cpp
//  Created:     Feb 18, 2005
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class represents parts and extra fields for
//               one constructed fare (result of an add-on
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

#include "AddonConstruction/AtpcoConstructedFare.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/ConstructedCacheManager.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/FareInfo.h"

using namespace tse;

AtpcoConstructedFare::AtpcoConstructedFare() : ConstructedFare(), _fareClassPriority(0) {}

ConstructedFare*
AtpcoConstructedFare::clone(DataHandle& dataHandle) const
{
  AtpcoConstructedFare* cloneObj = nullptr;
  dataHandle.get(cloneObj);

  // lint -e{413}
  clone(*cloneObj);

  return cloneObj;
}

void
AtpcoConstructedFare::clone(AtpcoConstructedFare& cloneObj) const
{
  ConstructedFare::clone(cloneObj);

  cloneObj._dateIntervals = _dateIntervals;
  cloneObj._fareClassPriority = _fareClassPriority;
}

ConstructedFareInfo*
AtpcoConstructedFare::cloneToConstructedFareInfo()
{
  ConstructedCacheManager& ccm = ConstructedCacheManager::instance();

  ConstructedFareInfo* cfi = nullptr;
  ccm.get(cfi);

  // lint -e{413}
  cloneToConstructedFareInfo(*cfi);

  return cfi;
}

void
AtpcoConstructedFare::cloneToConstructedFareInfo(ConstructedFareInfo& cfi)
{
  ConstructedFare::cloneToConstructedFareInfo(cfi);

  defineFareClassPriority();
  cfi.atpFareClassPriority() = _fareClassPriority;
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
void
AtpcoConstructedFare::initialize(ConstructionJob* cj,
                                 const FareInfo& sf,
                                 const GatewayPair& gw,
                                 const bool oppositeSpecified)
{
  ConstructedFare::initialize(cj, sf, gw, oppositeSpecified);
  _dateIntervals.effInterval() = _prevEffInterval;
}
#endif

void
AtpcoConstructedFare::defineFareClassPriority()
{
  _fareClassPriority = 0;

  if (_origAddon != nullptr)
    _fareClassPriority += _origAddon->atpcoFareClassPriority();

  if (_destAddon != nullptr)
    _fareClassPriority += _destAddon->atpcoFareClassPriority();
}

void
AtpcoConstructedFare::setAddonInterval(const DateIntervalBase& addonInterval,
                                       const bool isOriginAddon)
{
  const AtpcoFareDateInterval& di = static_cast<const AtpcoFareDateInterval&>(addonInterval);

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  _dateIntervals.effInterval().defineIntersection(_dateIntervals.effInterval(), di.effInterval());
#else
  _dateIntervals.effInterval() = di.effInterval();
#endif

  _dateIntervals.setTrfXRefInterval(di.trfXRefInterval());

  _fareDisplayOnly |= (di.trfXRefInterval().inhibit() == INHIBIT_D);

  _dateIntervals.setCombFareClassInterval(di.combFareClassInterval(isOriginAddon), isOriginAddon);

  _dateIntervals.showCombFareClassInterval(isOriginAddon) =
      di.showCombFareClassInterval(isOriginAddon);
}
