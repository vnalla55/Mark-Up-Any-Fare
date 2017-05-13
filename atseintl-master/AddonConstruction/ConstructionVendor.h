//-------------------------------------------------------------------
//
//  File:        ConstructionVendor.h
//  Created:     May 14, 2004
//  Authors:     Konstantin Sidorin, Vadim Nikushin
//
//  Description: Base class for any vendor add-on construction process
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
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/Thread/TseThreadingConst.h"
#include "DBAccess/ConstructedFareInfo.h"

namespace tse
{
class ConstructionJob;
class AddonFareInfo;
class ConstructedCacheDataWrapper;
class CombFareClassMap;

/**
*  Parallel cache warmup - optimization for historical FD / RD elased time
*/
class ConstructionVendorTask : public TseCallableTrxTask
{
public:
  ConstructionVendorTask() {}
  ConstructionVendorTask(PricingTrx& t, CacheGatewayPairVec::iterator it) : _gatewayPair(it)
  {
    trx(&t);
  }
  void performTask() override;

private:
  CacheGatewayPairVec::iterator _gatewayPair;
};

class ConstructionVendor
{
  friend class ConstructionVendorTest;

public:
  ConstructionVendor() = default;
  virtual ~ConstructionVendor() = default;

  ConstructionVendor(const ConstructionVendor&) = delete;
  ConstructionVendor& operator=(const ConstructionVendor&) = delete;

  // main interface
  // ==== =========

  virtual void initialize(ConstructionJob* cjob);

  AddonZoneStatus addAddonFare(ConstructionPoint cp, const AddonFareInfo& af);

  void addAddonFareRw(const AddonFareInfo& af);

  bool construction(ConstructedCacheDataWrapper& dw);

  bool reconstruction(ConstructedCacheDataWrapper& dw, CacheGatewayPairVec& gwToReconstruct);

  // accessors
  // =========

  ConstructionJob& cJob() { return *_cJob; }
  const ConstructionJob& cJob() const { return *_cJob; }

  const VendorCode& vendor() const { return _vendor; }

  AddonFareCortegeVec& addonFares(ConstructionPoint cp)
  {
    return (cp == CP_ORIGIN ? _origAddonFareCorteges : _destAddonFareCorteges);
  }

  const AddonFareCortegeVec& addonFares(ConstructionPoint cp) const
  {
    return (cp == CP_ORIGIN ? _origAddonFareCorteges : _destAddonFareCorteges);
  }

  const CacheGatewayPairVec& gateways() const { return _gateways; }

  virtual CombFareClassMap* getCombFareClassMap() = 0;

protected:
  ConstructionJob* _cJob = nullptr;
  VendorCode _vendor;

  AddonFareCortegeVec _origAddonFareCorteges;
  AddonFareCortegeVec _destAddonFareCorteges;

  CacheGatewayPairVec _gateways;

  bool markUpAddonFares(ConstructionPoint cp);

  virtual std::shared_ptr<GatewayPair> getNewGwPair() = 0;

  bool buildSEGateways(ConstructionPoint firstCP);

  bool buildDEGateways();

  bool buildDEGatewaysRw();

  bool assignFaresToGateways(CacheGatewayPairVec& gwToReconstruct);

  // next function is useless for SITA becouse almost all
  // SITA add-ons have tariff == 0. Add-ons will be matched anyway.
  // makes sence to implement for ATPCO only

  virtual bool matchAddonFares(AddonFareCortegeVec::iterator& firstOrigFare,
                               AddonFareCortegeVec::iterator& firstDestFare)
  {
    return true;
  }

  virtual AddonZoneStatus validateZones(const AddonFareInfo& addonFare,
                                        ConstructionPoint cp,
                                        TSEDateIntervalVec& zoneIntervals) = 0;

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  virtual bool isGlobalDirValid(const CarrierCode&,
                                TariffNumber /*addonTariff*/,
                                GlobalDirection)
  {
    return true;
  }
#endif

  void sortAddonFares();

private:
  static TseThreadingConst::TaskId _taskId;
}; // End class ConstructionVendor

} // End namespace tse
