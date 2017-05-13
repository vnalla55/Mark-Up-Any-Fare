//-------------------------------------------------------------------
//
//  File:        VendorSmf.h
//  Created:     May 23, 2006
//  Authors:     Vadim Nikushin
//
//  Description: Smf-specific part of add-on construction process
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

#include "AddonConstruction/AddonZoneMapSmf.h"
#include "AddonConstruction/CombFareClassMap.h"
#include "AddonConstruction/ConstructionVendor.h"
#include "AddonConstruction/TrfXrefMap.h"


namespace tse
{

class ConstructionJob;
class GatewayPair;

class VendorSmf : public ConstructionVendor
{

public:
  // construction/destruction
  // ========================

  VendorSmf();
  virtual ~VendorSmf() {};

  // main interface
  // ==== =========

  virtual void initialize(ConstructionJob* cjob) override;
  virtual CombFareClassMap* getCombFareClassMap() override;

  static ConstructionVendor* getNewVendor(ConstructionJob& cj);

  // accessors
  // =========

  TrfXrefMap& trfXrefMap() { return _trfXrefMap; }

protected:
  TrfXrefMap _trfXrefMap;
  CombFareClassMap _combFareClassMap;

  AddonZoneMapSmf _origAddonZones;
  AddonZoneMapSmf _destAddonZones;

  std::shared_ptr<GatewayPair> getNewGwPair() override;

  AddonZoneStatus validateZones(const AddonFareInfo& addonFare,
                                ConstructionPoint cp,
                                TSEDateIntervalVec& zoneIntervals) override;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  bool isGlobalDirValid(const CarrierCode& cxr,
                        TariffNumber addonTariff,
                        GlobalDirection globalDir) override;
#endif

private:

  // Placed here so they wont be called
  // ====== ==== == ==== ==== == ======

  VendorSmf(const VendorSmf& rhs);
  VendorSmf operator=(const VendorSmf& rhs);

}; // End class VendorSmf

} // End namespace tse

