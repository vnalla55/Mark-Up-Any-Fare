//-------------------------------------------------------------------
//
//  Description: Add-on Construction Orchestrator
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
#include "AddonConstruction/ConstructionJob.h"
#include "Common/Thread/TseCallableTrxTask.h"

namespace tse
{
class PricingTrx;
class ConstructedFareInfoResponse;

class AddonConstructionOrchestrator
{
public:
  AddonConstructionOrchestrator() {};
  virtual ~AddonConstructionOrchestrator() {};

  static void classInit();

#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
  static bool process(PricingTrx& trx,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const LocCode& orig,
                      const LocCode& boardCity,
                      const LocCode& dest,
                      const LocCode& offCity,
                      const GlobalDirection globalDir,
                      const bool singleOverDouble,
                      ConstructedFareInfoResponse& response,
                      const DateTime& travelDate,
                      SpecifiedFareCache* specCache);
#else
  static bool process(PricingTrx& trx,
                      const VendorCode& vendor,
                      const CarrierCode& carrier,
                      const LocCode& orig,
                      const LocCode& boardCity,
                      const LocCode& dest,
                      const LocCode& offCity,
                      const bool singleOverDouble,
                      ConstructedFareInfoResponse& response,
                      const DateTime& travelDate);
#endif

  static bool process(ConstructionJob& cj);

  static bool processConstructionCacheFlush(ConstructionJob& cJob);

private:
  class ConstructionTask : public TseCallableTrxTask
  {
  public:
#ifndef DISABLE_ADDON_CONSTRUCTION_OPTIMIZATION
    ConstructionTask(PricingTrx& trx,
                     const DateTime& travelDate,
                     const DateTime& ticketingDate,
                     const LocCode& orig,
                     const LocCode& boardCity,
                     const LocCode& dest,
                     const LocCode& offCity,
                     const GlobalDirection globalDir,
                     const bool singleOverDouble,
                     SpecifiedFareCache* specCache);
#else
    ConstructionTask(PricingTrx& trx,
                     const DateTime& travelDate,
                     const DateTime& ticketingDate,
                     const LocCode& orig,
                     const LocCode& boardCity,
                     const LocCode& dest,
                     const LocCode& offCity,
                     const bool singleOverDouble);
#endif

    ~ConstructionTask();

    void performTask() override;

    void setVendorCode(const VendorCode& v) { _cJob.setVendorCode(v); }
    CarrierCode& carrier() { return _cJob.carrier(); }

    const ConstructedFareInfoResponse& response() const { return _cJob.response(); }

    void createDiagCollector() { _cJob.createDiagCollector(); }
    void reclaimDiagCollector() { _cJob.reclaimDiagCollector(); }

  protected:
    ConstructionJob _cJob;
  };

  typedef std::vector<ConstructionTask> ConstructionTaskList;

  AddonConstructionOrchestrator(const AddonConstructionOrchestrator& rhs);

  AddonConstructionOrchestrator& operator=(const AddonConstructionOrchestrator& rhs);
};

} // End of namespace tse

