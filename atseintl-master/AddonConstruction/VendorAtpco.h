//-------------------------------------------------------------------
//
//  File:        VendorAtpco.h
//  Created:     Nov 17, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Atpco-specific part of add-on construction process
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
#include "AddonConstruction/CombFareClassMap.h"
#include "AddonConstruction/ConstructionVendor.h"
#include "AddonConstruction/TrfXrefMap.h"


namespace tse
{

class ConstructionJob;
class GatewayPair;

class VendorAtpco : public ConstructionVendor
{

public:
  // construction/destruction
  // ========================

  VendorAtpco();
  virtual ~VendorAtpco();

  // main interface
  // ==== =========

  virtual void initialize(ConstructionJob* cjob) override;
  virtual CombFareClassMap* getCombFareClassMap() override;

  static ConstructionVendor* getNewVendor(ConstructionJob& cj);

  // accessors
  // =========

  TrfXrefMap& trfXrefMap() { return _trfXrefMap; }

  const Indicator originGeoAppl() const { return _originGeoAppl; }
  const Indicator destinationGeoAppl() const { return _destinationGeoAppl; }

protected:
  TrfXrefMap _trfXrefMap;
  CombFareClassMap _combFareClassMap;

  AddonZoneMap _origAddonZones;
  AddonZoneMap _destAddonZones;

  Indicator _originGeoAppl;
  Indicator _destinationGeoAppl;

  std::shared_ptr<GatewayPair> getNewGwPair() override;

  bool matchAddonFares(AddonFareCortegeVec::iterator& firstOrigFare,
                       AddonFareCortegeVec::iterator& firstDestFare) override;

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

  VendorAtpco(const VendorAtpco& rhs);
  VendorAtpco operator=(const VendorAtpco& rhs);

}; // End class VendorAtpco

} // End namespace tse

