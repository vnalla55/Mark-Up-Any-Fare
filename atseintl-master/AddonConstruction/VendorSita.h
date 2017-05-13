//-------------------------------------------------------------------
//
//  File:        VendorSita.h
//  Created:     Nov 17, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Sita-specific part of add-on construction process
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

#include "AddonConstruction/AddonZoneMap.h"
#include "AddonConstruction/ConstructionVendor.h"


namespace tse
{
class ConstructionJob;
class GatewayPair;

class VendorSita : public ConstructionVendor
{

public:
  // construction/destruction
  // ========================

  VendorSita() {}
  virtual ~VendorSita() {}

  virtual CombFareClassMap* getCombFareClassMap() override;

  static ConstructionVendor* getNewVendor(ConstructionJob& cj);

protected:
  AddonZoneMap _origAddonZones;
  AddonZoneMap _destAddonZones;

  std::shared_ptr<GatewayPair> getNewGwPair() override;

  AddonZoneStatus validateZones(const AddonFareInfo& addonFare,
                                ConstructionPoint cp,
                                TSEDateIntervalVec& zoneIntervals) override;

private:

  // Placed here so they wont be called
  // ====== ==== == ==== ==== == ======

  VendorSita(const VendorSita& rhs);
  VendorSita operator=(const VendorSita& rhs);

}; // End class VendorSita

} // End namespace tse

