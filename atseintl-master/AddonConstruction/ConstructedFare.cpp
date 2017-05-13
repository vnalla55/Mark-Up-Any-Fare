//-------------------------------------------------------------------
//
//  File:        ConstructedFare.cpp
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

#include "AddonConstruction/ConstructedFare.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/ConstructedCacheManager.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/GatewayPair.h"
#include "Common/TseConsts.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/DataHandle.h"
#include "DBAccess/FareInfo.h"

using namespace tse;

ConstructedFare::ConstructedFare()
  : _cJob(nullptr),
    _origAddon(nullptr),
    _specifiedFare(nullptr),
    _destAddon(nullptr),
    _valid(true),
    _isDoubleEnded(false),
    _isOppositeSpecified(false),
    _fareDisplayOnly(false),
    _pricingOnly(false),
    _prevFareDisplayOnly(false)
{
}

const bool
ConstructedFare::isValid() const
{
  if (LIKELY(_valid))
  {
    if (_isDoubleEnded)
      return (_origAddon != nullptr) && (_destAddon != nullptr);

    else
      return (_origAddon != nullptr) || (_destAddon != nullptr);
  }

  return _valid;
}

void
ConstructedFare::clone(ConstructedFare& cloneObj) const
{
  cloneObj._cJob = _cJob;

  cloneObj._origAddon = _origAddon;
  cloneObj._specifiedFare = _specifiedFare;
  cloneObj._destAddon = _destAddon;

  cloneObj._valid = _valid;

  cloneObj._isDoubleEnded = _isDoubleEnded;
  cloneObj._isOppositeSpecified = _isOppositeSpecified;

  cloneObj._market1 = _market1;
  cloneObj._market2 = _market2;

  cloneObj._gateway1 = _gateway1;
  cloneObj._gateway2 = _gateway2;

  cloneObj._fareDisplayOnly = _fareDisplayOnly;
  cloneObj._pricingOnly = _pricingOnly;

  cloneObj._prevFareDisplayOnly = _prevFareDisplayOnly;
  cloneObj._prevEffInterval = _prevEffInterval;
}

void
ConstructedFare::cloneToConstructedFareInfo(ConstructedFareInfo& cfi)
{
  // ConstructedFareInfo fields

  if (_origAddon == nullptr)
    cfi.constructionType() = ConstructedFareInfo::SINGLE_DESTINATION;

  else if (_destAddon == nullptr)
    cfi.constructionType() = ConstructedFareInfo::SINGLE_ORIGIN;

  else
    cfi.constructionType() = ConstructedFareInfo::DOUBLE_ENDED;

  cfi.gateway1() = _gateway1;
  cfi.gateway2() = _gateway2;

  cfi.specifiedFareAmount() = _specifiedFare->fareAmount();
  cfi.constructedNucAmount() = 0;

  if (_origAddon != nullptr)
  {
    const AddonFareInfo& oa = *(_origAddon->addonFare());

    cfi.origAddonZone() = oa.arbZone();
    cfi.origAddonFootNote1() = oa.footNote1();
    cfi.origAddonFootNote2() = oa.footNote2();
    cfi.origAddonFareClass() = oa.fareClass();
    cfi.origAddonTariff() = oa.addonTariff();
    cfi.origAddonRouting() = oa.routing();

    if (oa.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
      cfi.origAddonAmount() = oa.fareAmt() / 2.0;
    else
      cfi.origAddonAmount() = oa.fareAmt();

    cfi.origAddonCurrency() = oa.cur();
    cfi.origAddonOWRT() = oa.owrt();
  }

  if (_destAddon != nullptr)
  {
    const AddonFareInfo& da = *(_destAddon->addonFare());

    cfi.destAddonZone() = da.arbZone();
    cfi.destAddonFootNote1() = da.footNote1();
    cfi.destAddonFootNote2() = da.footNote2();
    cfi.destAddonFareClass() = da.fareClass();
    cfi.destAddonTariff() = da.addonTariff();
    cfi.destAddonRouting() = da.routing();

    if (da.owrt() == ROUND_TRIP_MAYNOT_BE_HALVED)
      cfi.destAddonAmount() = da.fareAmt() / 2.0;
    else
      cfi.destAddonAmount() = da.fareAmt();

    cfi.destAddonCurrency() = da.cur();
    cfi.destAddonOWRT() = da.owrt();
  }

  // common ATPCO & SITA specified fare fields

  FareInfo& sfi = cfi.fareInfo();

  sfi.effInterval() = effInterval();

  sfi.vendor() = _specifiedFare->vendor();
  sfi.carrier() = _specifiedFare->carrier();

  sfi.market1() = _market1;
  sfi.market2() = _market2;

  sfi.originalFareAmount() = 0;
  sfi.fareAmount() = 0;

  sfi.fareClass() = _specifiedFare->fareClass();
  sfi.fareTariff() = _specifiedFare->fareTariff();

  sfi.noDec() = _specifiedFare->noDec();
  sfi.currency() = _specifiedFare->currency();

  sfi.footNote1() = _specifiedFare->footNote1();
  sfi.footNote2() = _specifiedFare->footNote2();

  sfi.owrt() = _specifiedFare->owrt();

  sfi.directionality() = defineDirectionality();

  sfi.ruleNumber() = _specifiedFare->ruleNumber();
  sfi.routingNumber() = _specifiedFare->routingNumber();
  sfi.globalDirection() = _specifiedFare->globalDirection();

  if (UNLIKELY(_fareDisplayOnly))
    sfi.inhibit() = INHIBIT_D;

  else if (UNLIKELY(_pricingOnly))
    sfi.inhibit() = INHIBIT_P;

  else
    sfi.inhibit() = INHIBIT_N;
}

void
ConstructedFare::initialize(ConstructionJob* cj,
                            const FareInfo& sf,
                            const GatewayPair& gw,
                            const bool oppositeSpecified)
{
  _cJob = cj;
  _specifiedFare = &sf;

  // lint -e{514}
  _prevFareDisplayOnly |= (sf.inhibit() == INHIBIT_D);

  _prevEffInterval = sf.effInterval();

  _isDoubleEnded = gw.isGw1ConstructPoint() && gw.isGw2ConstructPoint();

  _isOppositeSpecified = oppositeSpecified;

  _market1 = _cJob->origin();
  _market2 = _cJob->destination();

  _gateway1 = gw.gateway1();
  _gateway2 = gw.gateway2();
}

void
ConstructedFare::setAddon(const AddonFareCortege* addon,
                          const bool isOriginAddon,
                          const DateIntervalBase& addonInterval)
{
  origOrDestAddon(isOriginAddon) = addon;

  _fareDisplayOnly = _prevFareDisplayOnly || addon->fareDisplayOnly();

  setAddonInterval(addonInterval, isOriginAddon);
}

void
ConstructedFare::defineOriginDestination()
{
  // first to define market1 & market2

  if (_origAddon != nullptr)
    _market1 = _origAddon->addonFare()->interiorMarket();
  else if (_isOppositeSpecified)
    _market1 = _specifiedFare->market2();
  else
    _market1 = _specifiedFare->market1();

  if (_destAddon != nullptr)
    _market2 = _destAddon->addonFare()->interiorMarket();
  else if (_isOppositeSpecified)
    _market2 = _specifiedFare->market1();
  else
    _market2 = _specifiedFare->market2();

  // reverse fare if necessary

  if (_market1 > _market2)
  {
    LocCode tmpLoc = _market1;
    _market1 = _market2;
    _market2 = tmpLoc;

    tmpLoc = _gateway1;
    _gateway1 = _gateway2;
    _gateway2 = tmpLoc;

    const AddonFareCortege* tmpAddon = _origAddon;
    _origAddon = _destAddon;
    _destAddon = tmpAddon;

    _isOppositeSpecified = !_isOppositeSpecified;
  }
}

Directionality
ConstructedFare::defineDirectionality() const
{
  Directionality dl = _specifiedFare->directionality();

  if (_isOppositeSpecified && (dl != BOTH))
    dl = (dl == TO) ? FROM : TO;

  return dl;
}

void
ConstructedFare::adjustPrevValues()
{
  _prevFareDisplayOnly = _fareDisplayOnly;
  _prevEffInterval = effInterval();
}
