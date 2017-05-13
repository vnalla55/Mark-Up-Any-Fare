//-------------------------------------------------------------------
//  File:        DiagRequest.cpp
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//  Created:     May 28, 2005
//
//  Description:
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

#include "AddonConstruction/DiagRequest.h"

#include "AddonConstruction/AddonFareCortege.h"
#include "AddonConstruction/ConstructedFare.h"
#include "AddonConstruction/ConstructionJob.h"
#include "AddonConstruction/GatewayPair.h"
#include "DBAccess/AddonCombFareClassInfo.h"
#include "DBAccess/AddonFareInfo.h"
#include "DBAccess/ConstructedFareInfo.h"
#include "DBAccess/FareInfo.h"
#include "DBAccess/TariffCrossRefInfo.h"
#include "Diagnostic/ACDiagCollector.h"
#include "Diagnostic/DCFactory.h"

#include <iomanip>

namespace tse
{
static std::string
ANY("ANY");
static std::string
SIX_ASTERISK("******");
static std::string
FIVE_ASTERISK("*****");
static std::string
FIVE_X("XXXXX");

boost::mutex DiagRequest::_diagRequestMutex;

DiagRequest*
DiagRequest::instance(ConstructionJob& cj)
{
  // first to see if we want to create an instance of diag request.
  // this depends on two factors:
  //   1) user may want diagnostic only for specific market
  //   2) user may want diagnostic only for specific carrier
  // check them both and make a decision

  boost::lock_guard<boost::mutex> diagRequestGuard(_diagRequestMutex);

  DiagnosticTypes diagType = cj.trx().diagnostic().diagnosticType();

  std::map<std::string, std::string>::const_iterator e = cj.trx().diagnostic().diagParamMap().end();

  std::map<std::string, std::string>::const_iterator i;

  // /FM qualifier

  if (diagType != Diagnostic253)
  {
    i = cj.trx().diagnostic().diagParamMap().find(Diagnostic::FARE_MARKET);
    if (i != e)
    {
      LocCode boardCity = i->second.substr(0, 3);
      LocCode offCity = i->second.substr(3, 3);

      if (cj.reversedFareMarket())
      {
        if (((cj.origin() != offCity) && (cj.boardMultiCity() != offCity)) ||
            ((cj.destination() != boardCity) && (cj.offMultiCity() != boardCity)))
          return nullptr;
      }
      else
      {
        if (((cj.origin() != boardCity) && (cj.boardMultiCity() != boardCity)) ||
            ((cj.destination() != offCity) && (cj.offMultiCity() != offCity)))
          return nullptr;
      }
    }
  }

  // /CX qualifier

  i = cj.trx().diagnostic().diagParamMap().find(Diagnostic::DIAG_CARRIER);
  if (i != e)
    if (cj.carrier() != i->second)
      return nullptr;

  // if we are here user wants diagnostic for this particular
  // market and carrier. therefore, create a diag request

  DiagRequest* diagRequest = nullptr;
  cj.dataHandle().get(diagRequest);

  if (diagRequest != nullptr)
    diagRequest->parseDiagnosticQualifiers(cj);

  return diagRequest;
}

DiagRequest*
DiagRequest::instance259(ConstructionJob& cj)
{
  // diag259 applies to entire constructed cache.
  // no any special checks are needed. diagrequest
  // should be created in any event.

  DiagRequest* diagRequest = nullptr;
  cj.dataHandle().get(diagRequest);

  if (diagRequest != nullptr)
    diagRequest->parseDiagnostic259Qualifiers(cj);

  return diagRequest;
}

ACDiagCollector*
DiagRequest::createDiagCollector(ConstructionJob& cj)
{
  ACDiagCollector* diagCollector =
      dynamic_cast<ACDiagCollector*>(DCFactory::instance()->create(cj.trx()));

  if (diagCollector != nullptr)
  {
    diagCollector->initialize(&cj, this);
    diagCollector->enable(cj.trx().diagnostic().diagnosticType());
  }

  return diagCollector;
}

void
DiagRequest::reclaimDiagCollector(ACDiagCollector* diagCollector)
{
  if (diagCollector != nullptr)
  {
    diagCollector->flushMsg();
  }
}

bool
DiagRequest::isValidForDiag(const VendorCode& vendor) const
{
  return _diagVendor.empty() || (_diagVendor == vendor);
}

bool
DiagRequest::isValidForDiag(const GatewayPair& gatewayPair) const
{
  return (_gw1.empty() && _gw2.empty()) ||
         (gatewayPair.gateway1() == _gw1 && gatewayPair.gateway2() == _gw2) ||
         (gatewayPair.gateway1() == _gw2 && gatewayPair.gateway2() == _gw1);
}

bool
DiagRequest::isValidForDiag(const AddonFareInfo& addonFareInfo, const bool valid) const
{
  if (!valid && _validFaresOnly)
    return false;

  return (_diagVendor.empty() || _diagVendor == addonFareInfo.vendor()) &&
         ((_gw1.empty() && _gw2.empty()) || (_gw1 == addonFareInfo.gatewayMarket()) ||
          (_gw2 == addonFareInfo.gatewayMarket())) &&
         (_addonFareClass.empty() || addonFareInfo.fareClass() == _addonFareClass) &&
         (_addonFareTariff == 0 || addonFareInfo.addonTariff() == _addonFareTariff);
}

bool
DiagRequest::isValidForDiag(const FareInfo& fareInfo, const bool valid) const
{
  if (!valid && _validFaresOnly)
    return false;

  bool ok = (_diagVendor.empty() || _diagVendor == fareInfo.vendor()) &&
            (_fareClass.empty() || fareInfo.fareClass() == _fareClass) &&
            (_fareTariff == 0 || _fareTariff == fareInfo.fareTariff());

  if (ok && !_gw1.empty() && !_gw2.empty())
    ok = (fareInfo.market1() == _gw1 && fareInfo.market2() == _gw2) ||
         (fareInfo.market1() == _gw2 && fareInfo.market2() == _gw1);

  return ok;
}

bool
DiagRequest::isValidForDiag(const AddonFareInfo& addonFareInfo, const FareInfo& fareInfo) const
{
  bool addonIsOk = isValidForDiag(addonFareInfo);

  if (!_addonFareClass.empty() || _addonFareTariff != 0)
    if (!addonIsOk)
      return false;

  bool specifiedIsOk = isValidForDiag(fareInfo);

  if (!_fareClass.empty() || _fareTariff != 0)
    if (!specifiedIsOk)
      return false;

  return addonIsOk || specifiedIsOk;
}

bool
DiagRequest::isValidForDiag(const AddonFareInfo& originAddon,
                            const FareInfo& fareInfo,
                            const AddonFareInfo& destinationAddon) const
{
  bool originAddonIsOk = isValidForDiag(originAddon);
  bool destinationAddonIsOk = isValidForDiag(destinationAddon);

  if (!_addonFareClass.empty() || _addonFareTariff != 0)
    if (!originAddonIsOk || !destinationAddonIsOk)
      return false;

  bool specifiedIsOk = isValidForDiag(fareInfo);

  if (!_fareClass.empty() || _fareTariff != 0)
    if (!specifiedIsOk)
      return false;

  return originAddonIsOk || specifiedIsOk || destinationAddonIsOk;
}

bool
DiagRequest::isValidForDiag(const ConstructedFare& cf) const
{
  bool ok = isValidForDiag(*cf.specifiedFare());

  if (ok && !_gw1.empty() && !_gw2.empty())
    ok = (cf.gateway1() == _gw1 && cf.gateway2() == _gw2) ||
         (cf.gateway1() == _gw2 && cf.gateway2() == _gw1);

  bool okOrigAddon = false;
  if (ok && cf.origAddon() != nullptr)
    okOrigAddon = isValidForDiag(*(cf.origAddon()->addonFare()));

  bool okDestAddon = false;
  if (ok && cf.destAddon() != nullptr)
    okDestAddon = isValidForDiag(*(cf.destAddon()->addonFare()));

  return ok && (okOrigAddon || okDestAddon);
}

bool
DiagRequest::isValidForDiag(const ConstructedFareInfo& cfi, const bool valid) const
{
  bool ok = true;

  const FareInfo* fi = &(cfi.fareInfo());
  if (fi != nullptr)
    ok = isValidForDiag(*fi, valid);

  if (ok && !_gw1.empty() && !_gw2.empty())
    ok = (cfi.gateway1() == _gw1 && cfi.gateway2() == _gw2) ||
         (cfi.gateway1() == _gw2 && cfi.gateway2() == _gw1);

  if (ok && !_addonFareClass.empty())
    ok = (cfi.origAddonFareClass() == _addonFareClass) ||
         (cfi.destAddonFareClass() == _addonFareClass);

  if (ok && _addonFareTariff != 0)
    ok = (cfi.origAddonTariff() == _addonFareTariff) || (cfi.destAddonTariff() == _addonFareTariff);

  return ok;
}

bool
DiagRequest::isValidForDiag(const TariffCrossRefInfo& tcri) const
{
  return (_diagVendor.empty() || tcri.vendor() == _diagVendor) && (tcri.fareTariff() > 0) &&
         (_fareTariff == 0 || tcri.fareTariff() == _fareTariff) &&
         (_addonFareTariff == 0 || tcri.addonTariff1() == _addonFareTariff ||
          tcri.addonTariff2() == _addonFareTariff);
}

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

bool
DiagRequest::isValidForDiag(const AddonCombFareClassInfo& acfc) const
{
  return (_addonFareClass.empty() || acfc.addonFareClass() == _addonFareClass[0]) &&
         (_fareClass.empty() || acfc.fareClass() == _fareClass);
}

bool
DiagRequest::isValidForDiag(const VendorCode& vendor, TariffNumber fareTariff) const
{
  return (_diagVendor.empty() || vendor == _diagVendor) &&
         (_fareTariff == 0 || fareTariff == _fareTariff);
}

#else

bool
DiagRequest::isValidForDiag(const AddonCombFareClassInfo& acfc) const
{
  return (_diagVendor.empty() || acfc.vendor() == _diagVendor) &&
         (_fareTariff == 0 || acfc.fareTariff() == _fareTariff) &&
         (_addonFareClass.empty() || acfc.addonFareClass() == _addonFareClass) &&
         (_fareClass.empty() || acfc.fareClass() == _fareClass);
}

#endif

void
DiagRequest::parseDiagnosticQualifiers(ConstructionJob& cj)
{
  DiagnosticTypes diagType = cj.trx().diagnostic().diagnosticType();
  std::map<std::string, std::string>& diagParamMap = cj.trx().diagnostic().diagParamMap();

  if (diagParamMap.empty())
  {
    if (diagType == Diagnostic252)
    {
      _display252Addons = true;
      _display252Specifieds = true;
    }

    return;
  }

  std::map<std::string, std::string>::const_iterator e = diagParamMap.end();

  std::map<std::string, std::string>::const_iterator i;

  // vendor

  i = diagParamMap.find(Diagnostic::DIAG_VENDOR);
  if (i != e)
    _diagVendor = i->second;

  // gateway pair

  i = diagParamMap.find(Diagnostic::GATEWAY_PAIR);
  if (i != e)
  {
    _gw1 = i->second.substr(0, 3);
    _gw2 = i->second.substr(3, 3);
  }

  // only valid fares

  i = diagParamMap.find(Diagnostic::ALL_VALID);
  _validFaresOnly = (i != e);

  // force Addon Construction

  i = diagParamMap.find("NF");
  _forceAddonConstruction = (i == e);

  // fare class

  i = diagParamMap.find(Diagnostic::FARE_CLASS_CODE);
  if (i != e)
  {
    _fareClass = i->second;

    if (diagType == Diagnostic252)
      _display252Specifieds = true;
  }

  // add-on fare class

  i = diagParamMap.find(Diagnostic::ADDON_FARE_CLASS_CODE);
  if (i != e)
  {
    if (i->second == ANY)
      _addonFareClass = SIX_ASTERISK;

    else if (i->second.size() == 6)
    {
      if (i->second.substr(1, 5) == FIVE_X)
      {
        _addonFareClass += i->second[0];
        _addonFareClass += FIVE_ASTERISK;
      }
      else
        _addonFareClass = i->second;
    }

    else
      _addonFareClass = i->second;

    if (diagType == Diagnostic252)
      _display252Addons = true;
  }

  // fare tariff

  i = diagParamMap.find(Diagnostic::FARE_TARIFF);
  if (i != e)
  {
    _fareTariff = atoi(i->second.c_str());

    if (diagType == Diagnostic252)
      _display252Specifieds = true;
  }

  // add-on fare tariff

  i = diagParamMap.find(Diagnostic::ADDON_FARE_TARIFF);
  if (i != e)
  {
    _addonFareTariff = atoi(i->second.c_str());

    if (diagType == Diagnostic252)
      _display252Addons = true;
  }

  // special fares

  i = diagParamMap.find(Diagnostic::FARE_ASSIGNMENT);
  if (i != e)
  {
    _checkSpecial = true;
    _isSpecial = (i->second == "S");
  }

  // display detail

  i = diagParamMap.find(Diagnostic::DISPLAY_DETAIL);
  if (i != e)
    _showRealFareOrder = (i->second == "RO");

  // show date interval details

  if (diagType == Diagnostic254)
  {
    i = diagParamMap.find(Diagnostic::DATE_INTERVALS);
    cj.showDateIntervalDetails() = (i != e);
  }

  if (diagType == Diagnostic252)
  {
    // If FC,AC,FT,AT are all undefined then display them all

    if (!_display252Addons && !_display252Specifieds)
    {
      _display252Addons = true;
      _display252Specifieds = true;
    }
  }
}

void
DiagRequest::parseDiagnostic259Qualifiers(ConstructionJob& cj)
{
  std::map<std::string, std::string>& diagParamMap = cj.trx().diagnostic().diagParamMap();

  std::map<std::string, std::string>::const_iterator e = diagParamMap.end();

  std::map<std::string, std::string>::const_iterator i;

  _flashEventType = FE_NO_FLUSH_EVENT;

  // flush event type

  i = diagParamMap.find("FL");
  if (i != e)
  {
    if (i->second.substr(0, 2) == "AL")
      _flashEventType = FE_FLASH_ALL;

    else if (i->second.substr(0, 2) == "AD")
      _flashEventType = FE_FLASH_ADDON_FARE;

    else if (i->second.substr(0, 2) == "SP")
      _flashEventType = FE_FLASH_SPECIFIED_FARE;

    else if (i->second.substr(0, 2) == "VC")
      _flashEventType = FE_FLASH_BY_VENDOR_CARRIER;
  }

  // /VN qualifier

  i = diagParamMap.find(Diagnostic::DIAG_VENDOR);
  if (i != e)
    _diagVendor = i->second;

  // /CX qualifier

  i = diagParamMap.find(Diagnostic::DIAG_CARRIER);
  if (i != e)
    _diagCarrier = i->second;

  // flush market

  i = diagParamMap.find(Diagnostic::FARE_MARKET);
  if (i != e)
    if (i->second.size() == 6)
    {
      _flushMarket1 = i->second.substr(0, 3);
      _flushMarket2 = i->second.substr(3, 3);
    }

  if (_flashEventType == FE_FLASH_ADDON_FARE || _flashEventType == FE_FLASH_SPECIFIED_FARE)
  {
    if (_diagVendor.empty() || _diagCarrier.empty() || _flushMarket1.empty() ||
        _flushMarket2.empty())
      _flashEventType = FE_NO_FLUSH_EVENT;
  }

  else if (_flashEventType == FE_FLASH_BY_VENDOR_CARRIER)
  {
    if (_diagVendor.empty() || _diagCarrier.empty())
      _flashEventType = FE_NO_FLUSH_EVENT;
  }
}
}
