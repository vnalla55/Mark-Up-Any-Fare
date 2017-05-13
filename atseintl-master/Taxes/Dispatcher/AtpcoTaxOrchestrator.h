// ----------------------------------------------------------------------------
//
//  Copyright Sabre 2013
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the  program(s)
//          have been supplied.
//
// ----------------------------------------------------------------------------
#pragma once

#include "Diagnostic/Diagnostic.h"
#include "Taxes/Dispatcher/TaxOrchestrator.h"

namespace tax
{
class Request;
}

namespace tse
{
class DCFactory;
class FarePath;
class Itin;
class LegacyTaxProcessor;
class TaxResponse;

class AtpcoTaxOrchestrator final : public TaxOrchestrator
{
  friend class AtpcoTaxOrchestratorTest;

public:
  AtpcoTaxOrchestrator(const std::string& name, TseServer& srv);
  ~AtpcoTaxOrchestrator();
  virtual bool process(PricingTrx& trx) override;
  virtual bool process(TaxTrx& trx) override;
  virtual bool process(FareDisplayTrx& fareDisplayTrx) override;
  virtual bool process(AncillaryPricingTrx& trx) override;
  virtual bool process(ExchangePricingTrx& trx) override;
  virtual bool process(RexPricingTrx& trx) override;
  virtual bool process(RexExchangeTrx& trx) override;
  virtual bool process(RefundPricingTrx& trx) override;
  template <typename TrxType>
  bool processTrx(TrxType& trxType);

private:
  LegacyTaxProcessor* _legacyTaxProcessor;

  bool atpcoProcess(PricingTrx& trx);
  void copyTaxResponse(PricingTrx& trx);
  bool initialDiagnostic(PricingTrx& trx);
  void createTaxResponse(PricingTrx&, tse::Itin&, tse::FarePath&, DCFactory&);
  void createTaxResponses(PricingTrx&, tse::Itin*, tse::FarePath*, DCFactory*);
  void initTseTaxResponses(PricingTrx& trx);
  void setDefaultValidatingCarrier(PricingTrx& trx) const;

  AtpcoTaxOrchestrator(const AtpcoTaxOrchestrator&);
  AtpcoTaxOrchestrator& operator=(const AtpcoTaxOrchestrator&);

  bool requestDiagnostic(tse::PricingTrx& trx) const;
  void printDiagnosticMessage(Trx& trx,
                              DiagnosticTypes diagnostic,
                              const std::string msg,
                              tax::Request* request = nullptr) const;
};

} // namespace tse
