//-------------------------------------------------------------------
//  File:        DiagRequest.h
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

#pragma once

#include "AddonConstruction/ConstructionDefs.h"

#include <boost/thread/mutex.hpp>

namespace tse
{

class ConstructionJob;

class FareInfo;
class AddonFareInfo;
class GatewayPair;
class TariffCrossRefInfo;
class AddonCombFareClassInfo;
class ConstructedFareInfo;

class ACDiagCollector;

class DiagRequest final
{
public:
  enum FlashEventType
  { FE_NO_FLUSH_EVENT,
    FE_FLASH_ADDON_FARE,
    FE_FLASH_SPECIFIED_FARE,
    FE_FLASH_BY_VENDOR_CARRIER,
    FE_FLASH_ALL };

  DiagRequest() = default;
  DiagRequest(const DiagRequest&) = delete;
  DiagRequest operator=(const DiagRequest&) = delete;

  static DiagRequest* instance(ConstructionJob& cj);
  static DiagRequest* instance259(ConstructionJob& cj);

  // main interface
  // ==== =========

  ACDiagCollector* createDiagCollector(ConstructionJob& cj);
  void reclaimDiagCollector(ACDiagCollector* diagCollector);

  bool isValidForDiag(const VendorCode& vendor) const;
  bool isValidForDiag(const GatewayPair& gatewayPair) const;

  bool isValidForDiag(const AddonFareInfo& addonFareInfo, const bool valid = true) const;

  bool isValidForDiag(const FareInfo& fareInfo, const bool valid = true) const;

  bool isValidForDiag(const AddonFareInfo& addonFareInfo, const FareInfo& fareInfo) const;

  bool isValidForDiag(const AddonFareInfo& originAddon,
                      const FareInfo& fareInfo,
                      const AddonFareInfo& destinationAddon) const;

  bool isValidForDiag(const ConstructedFare& cf) const;

  bool isValidForDiag(const ConstructedFareInfo& cfi, const bool valid = true) const;

  bool isValidForDiag(const TariffCrossRefInfo& tcri) const;
  bool isValidForDiag(const AddonCombFareClassInfo& acfc) const;
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  bool isValidForDiag(const VendorCode& vendor, TariffNumber fareTariff) const;
#endif

  // accessors
  // =========

  const FlashEventType flashEventType() const { return _flashEventType; }

  const VendorCode& diagVendor() const { return _diagVendor; }

  const CarrierCode& diagCarrier() const { return _diagCarrier; }

  const LocCode& flushMarket1() const { return _flushMarket1; }

  const LocCode& flushMarket2() const { return _flushMarket2; }

  const bool forceAddonConstruction() const { return _forceAddonConstruction; }

  const bool showRealFareOrder() const { return _showRealFareOrder; }

  const bool display252Addons() const { return _display252Addons; }

  const bool display252Specifieds() const { return _display252Specifieds; }

protected:
  // qualifiers for diagnostic
  // ========== === ==========
  //
  // qualifier    meaning       251 252 253 254 255 257 259
  //    FM     [FareMarket]      x   x   x   x   x   x
  //    VN     [Vendor]          x   x   x   x   x   x
  //    CX     [Carrier]         x   x   x   x   x   x
  //    GW     [Gateways]            x       x   x   x
  //    AV     [AllValidFares]       x
  //    FC     [FareClass]           x   x   x   x   x
  //    AC     [AddonFareClass]      x   x   x   x   x
  //    FT     [FareTariff]          x       x   x   x
  //    AT     [AddonFareTariff]     x       x   x   x
  //    DI     [DateIntervals]               x
  //    FL     [Flush]                                   x

  FlashEventType _flashEventType = FlashEventType::FE_NO_FLUSH_EVENT;

  VendorCode _diagVendor;
  CarrierCode _diagCarrier;

  LocCode _flushMarket1;
  LocCode _flushMarket2;

  LocCode _gw1;
  LocCode _gw2;

  FareClassCode _fareClass;
  FareClassCode _addonFareClass;

  TariffNumber _fareTariff = 0;
  TariffNumber _addonFareTariff = 0;

  bool _forceAddonConstruction = true;
  bool _validFaresOnly = false;

  bool _showRealFareOrder = false;
  bool _checkSpecial = false;
  bool _isSpecial = false;

  bool _display252Addons = false;
  bool _display252Specifieds = false;

  void parseDiagnosticQualifiers(ConstructionJob& cj);

  void parseDiagnostic259Qualifiers(ConstructionJob& cj);

private:
  static boost::mutex _diagRequestMutex;
};
} // namespace tse

