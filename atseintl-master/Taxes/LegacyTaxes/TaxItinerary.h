// -------------------------------------------------------------------------
//  Copyright Sabre 2004
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ---------------------------------------------------------------------------

#pragma once

#include "Common/TSELatencyData.h"
#include "Common/Thread/TseCallableTrxTask.h"
#include "Common/TseCodeTypes.h"
#include "Taxes/LegacyTaxes/TaxMap.h"
#include <iostream>

namespace tse
{
class DiagCollector;
class PricingTrx;
class Itin;
class FarePath;
class NetRemitFarePath;

class TaxItinerary : public TseCallableTrxTask
{
  friend class TaxItineraryTest;
  class Impl; // hides all implementation details

public:
  using TaxCodes = std::vector<TaxCode>;

public:
  void initialize(PricingTrx& trx, Itin& itin, TaxMap::TaxFactoryMap& taxFactoryMap);

  void accumulator();

  virtual void performTask() override { accumulator(); }
  void processNetRemitFarePath(FarePath* farePath, DiagCollector* diag, bool a=true);
  void processFarePathPerCxr(FarePath* farePath, DiagCollector* diag);
  void processFarePathPerCxr_new(FarePath& farePath, DiagCollector* diag, const FarePath* originalFarePath = nullptr);

private:

  // const FarePath* originalFarePath   if farePath is adjusted, here is the original version
  FarePath* processFarePath(FarePath* farePath, DiagCollector* diag);

  void analyzeValCxrNetRemitFP(FarePath* farePath, NetRemitFarePath* netRemitFp);
  void removeCurrentTaxResponse(FarePath& farePath) const;
  void removeTaxResponseFromItin(FarePath& farePath, TaxResponse* taxResponse) const;

  bool collectTaxResponses(FarePath& farePath,
                                   DiagCollector* diag,
                                   const std::vector<CountrySettlementPlanInfo*>& cspiCol,
                                   std::vector<TaxResponse*>& taxResponses,
                                   const FarePath* originalFarePath);

  bool collectTaxResponsesForMultipleSpWithTCH(FarePath& farePath,
                                               DiagCollector* diag,
                                               CountrySettlementPlanInfo* cspiTCH,
                                               const std::vector<CountrySettlementPlanInfo*>& cspiCol,
                                               std::vector<TaxResponse*>& taxResponses,
                                               const FarePath* originalFarePath);

  TaxResponse* collectTaxResponse(FarePath& farePath,
                                  DiagCollector* diag,
                                  CountrySettlementPlanInfo* cspi,
                                  const FarePath* originalFarePath);

  void storeTaxResponses(FarePath& farePath,
                         std::vector<TaxResponse*>& taxResponse,
                         bool hasTCHForMultipleSp) const;

  void getCountrySettlementPlanInfoForValidatingCxr(
      FarePath& farePath,
      std::vector<CountrySettlementPlanInfo*>& cxrSps) const;

  CountrySettlementPlanInfo* getNonTCHCountrySettlementPlanInfo(
      const std::vector<CountrySettlementPlanInfo*>& cspiCol) const;

  PricingTrx* _trx = nullptr;
  Itin* _itin = nullptr;
  TaxMap::TaxFactoryMap* _taxFactoryMap = nullptr;
};
}
