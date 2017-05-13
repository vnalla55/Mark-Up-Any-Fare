//-------------------------------------------------------------------
//
//  File:        SitaGatewayPair.h
//  Created:     Jun 11, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class represents data and methods of SITA
//               process to build single- and double- ended
//               constructed fares
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

#include "AddonConstruction/GatewayPair.h"

namespace tse
{

class SITAAddonFareInfo;
class SITAFareInfo;

class AddonFareCortege;
class ConstructedFare;

class SitaGatewayPair : public GatewayPair
{
public:
  friend class SitaGatewayPairTest;
  // construction/destruction
  // ========================

  SitaGatewayPair() {};

  virtual ~SitaGatewayPair() {};
  virtual eGatewayPairType objectType() const override { return eSitaGatewayPair; }

  // main interface
  // ==== =========

  // accessors
  // =========

  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, GatewayPair);
  }

protected:
  ConstructedFare* getConstructedFare() override;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

  FareMatchCode
  matchAddonAndSpecified(AddonFareCortege& addonFare,
                         SpecifiedFare& specFare,
                         const bool isOriginAddon,
                         const bool oppositeSpecified,
                         AtpcoFareDateInterval& validInterval) override;

#else

  FareMatchCode
  matchAddonAndSpecified(AddonFareCortege& addonFare,
                         ConstructedFare& constrFare,
                         const bool isOriginAddon,
                         std::vector<DateIntervalBase*>& constrFareIntervals) override;

#endif

  FareMatchCode finalMatch() override;

  static FareMatchCode matchOWRT(const SITAAddonFareInfo& af, const SITAFareInfo& sf);

  static FareMatchCode matchRouting(const SITAAddonFareInfo& af, const SITAFareInfo& sf);

  static FareMatchCode matchRouteCode(const SITAAddonFareInfo& af, const SITAFareInfo& sf);

  static FareMatchCode matchTariffFamily(const SITAAddonFareInfo& af, const SITAFareInfo& sf);

  static FareMatchCode matchFareQualityCode(const SITAAddonFareInfo& af, const SITAFareInfo& sf);

  static FareMatchCode matchTariff(const SITAAddonFareInfo& af, const SITAFareInfo& sf);

  static FareMatchCode matchApplicableRule(const SITAAddonFareInfo& af, const SITAFareInfo& sf);

  FareMatchCode
  matchFareBasisOrGlobalClass(const SITAAddonFareInfo& af, const SITAFareInfo& sf) const;

  FareMatchCode matchGlobalClass(const SITAAddonFareInfo& af, const SITAFareInfo& sf) const;

  FareMatchCode matchGlobalDBEClasses(const SITAAddonFareInfo& af, const SITAFareInfo& sf) const;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

  static FareMatchCode matchAddonDirection(const SITAAddonFareInfo& af,
                                           const FareInfo& sfp,
                                           const bool oppositeSpecified,
                                           const bool isOriginAddon);

#else

  static FareMatchCode matchAddonDirection(const SITAAddonFareInfo& af,
                                           const ConstructedFare& sfp,
                                           const bool isOriginAddon);

#endif

  static FareMatchCode matchRuleTariff(const SITAAddonFareInfo& af, const SITAFareInfo& sf);

private:
  // Placed here so they wont be called
  // ====== ==== == ==== ==== == ======

  SitaGatewayPair(const SitaGatewayPair& rhs);
  SitaGatewayPair operator=(const SitaGatewayPair& rhs);

}; // End class SitaGatewayPair

} // End namespace tse

