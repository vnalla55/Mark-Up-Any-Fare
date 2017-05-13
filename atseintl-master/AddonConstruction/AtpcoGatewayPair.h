//-------------------------------------------------------------------
//
//  File:        AtpcoGatewayPair.h
//  Created:     Jun 11, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Class represents data and methods of ATPCO
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
class ConstructedFare;

class AtpcoGatewayPair : public GatewayPair
{
public:
  // construction/destruction
  // ========================

  AtpcoGatewayPair() {};
  virtual ~AtpcoGatewayPair() {};
  virtual eGatewayPairType objectType() const override { return eAtpcoGatewayPair; }

  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, GatewayPair);
  }

protected:
  ConstructedFare* getConstructedFare() override;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

public:
  FareMatchCode
  matchAddonFareClass(const AddonFareCortege& fare,
                      Indicator geoAppl,
                      const AddonFareClasses& fareclasses,
                      TSEDateInterval& validDI) const;

protected:
  virtual FareMatchCode
  matchAddonAndSpecified(AddonFareCortege& addonFare,
                         SpecifiedFare& specFare,
                         const bool oppositeSpecified,
                         const bool isOriginAddon,
                         AtpcoFareDateInterval& validInterval) override;

  FareMatchCode matchOWRT(const Indicator addonOwrt, const Indicator specOwrt);

  FareMatchCode matchAddonDirection(AddonFareCortege& addonFare,
                                    const FareInfo& specFare,
                                    const bool oppositeSpecified,
                                    const bool isOriginAddon);

  virtual AddonFareCortegeVec::iterator
  partition(AddonFareCortegeVec::iterator firstAddon,
            AddonFareCortegeVec::iterator endOfAddons) override;

  std::pair<AddonFareCortegeVec::iterator, AddonFareCortegeVec::iterator>
  getOwrtMatchingAddonRange(
      const FareInfo& sfi,
      AddonFareCortegeVec::iterator firstAddon,
      AddonFareCortegeVec::iterator endOfAddons,
      AddonFareCortegeVec::iterator roundTripMayNotBeHalvedAddonBound) override;

#else

  virtual FareMatchCode
  matchAddonAndSpecified(AddonFareCortege& addonFare,
                         ConstructedFare& constrFare,
                         const bool isOriginAddon,
                         std::vector<DateIntervalBase*>& constrFareIntervals) override;

  FareMatchCode matchOWRT(const Indicator addonOwrt, const Indicator specOwrt);

  FareMatchCode matchAddonDirection(AddonFareCortege& addonFare,
                                    ConstructedFare& constrFare,
                                    const bool isOriginAddon);

  void defineIntervalIntersection(const TSEDateInterval& cfValid,
                                  const InhibitedDateIntervalVec& xrefRecords,
                                  const AddonCombFareClassInfoVec& fClassCombRecords,
                                  const bool isOriginAddon,
                                  std::vector<DateIntervalBase*>& constrFareIntervals,
                                  FareMatchCode& matchResult);

  virtual AddonFareCortegeVec::iterator
  partition(const AddonFareCortegeVec::iterator& firstAddon,
            const AddonFareCortegeVec::iterator& endOfAddons) override;

  virtual std::pair<AddonFareCortegeVec::iterator, AddonFareCortegeVec::iterator> getAddonIterators(
      const FareInfo& sfi,
      const AddonFareCortegeVec::iterator& firstAddon,
      const AddonFareCortegeVec::iterator& endOfAddons,
      const AddonFareCortegeVec::iterator& roundTripMayNotBeHalvedAddonBound) override;

#endif

private:
  // Placed here so they wont be called
  // ====== ==== == ==== ==== == ======

  AtpcoGatewayPair(const AtpcoGatewayPair& rhs);
  AtpcoGatewayPair operator=(const AtpcoGatewayPair& rhs);

}; // End class AtpcoGatewayPair

} // End namespace tse

