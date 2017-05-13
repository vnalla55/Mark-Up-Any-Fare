//-------------------------------------------------------------------
//
//  File:        SmfGatewayPair.h
//  Created:     May 24, 2006
//  Authors:     Vadim Nikushin
//
//  Description: Class represents data and methods of SMF
//               process to build single- and double- ended
//               constructed fares
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

#pragma once

#include "AddonConstruction/AtpcoGatewayPair.h"

namespace tse
{

class SmfGatewayPair : public AtpcoGatewayPair
{
  friend class SmfGatewayPairTest;

public:
  // construction/destruction
  // ========================

  SmfGatewayPair() : AtpcoGatewayPair() {};
  virtual ~SmfGatewayPair() {};
  virtual eGatewayPairType objectType() const override { return eSmfGatewayPair; }

  virtual void flattenize(Flattenizable::Archive& archive) override
  {
    FLATTENIZE_BASE_OBJECT(archive, AtpcoGatewayPair);
  }

protected:

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION

  virtual FareMatchCode
  matchAddonAndSpecified(AddonFareCortege& addonFare,
                         SpecifiedFare& specFare,
                         const bool oppositeSpecified,
                         const bool isOriginAddon,
                         AtpcoFareDateInterval& validInterval) override;

  FareMatchCode
  matchAddonFareClass(const AddonFareCortege& fare,
                      const AddonFareClasses& fareclasses,
                      TSEDateInterval& validDI) const;

#else

  virtual FareMatchCode
  matchAddonAndSpecified(AddonFareCortege& addonFare,
                         ConstructedFare& constrFare,
                         const bool isOriginAddon,
                         std::vector<DateIntervalBase*>& constrFareIntervals) override;

#endif

private:
  // Placed here so they wont be called
  // ====== ==== == ==== ==== == ======

  SmfGatewayPair(const SmfGatewayPair& rhs);
  SmfGatewayPair operator=(const SmfGatewayPair& rhs);

}; // End class SmfGatewayPair

} // End namespace tse

