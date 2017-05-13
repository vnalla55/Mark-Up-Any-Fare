//----------------------------------------------------------------------------
//
//          The copyright to the computer program(s) herein
//          is the property of Sabre.
//          The program(s) may be used and/or copied only with
//          the written permission of Sabre or in accordance
//          with the terms and conditions stipulated in the
//          agreement/contract under which the program(s)
//          have been supplied.
//
//----------------------------------------------------------------------------

#pragma once

#include "Common/ValidatingCxrConst.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class TicketingCxrTrx;
class TicketingCxrDisplayTrx;

class Diag191Collector : public DiagCollector
{
  friend class Diag191CollectorTest;

public:
  Diag191Collector() {}
  explicit Diag191Collector(Diagnostic& root) : DiagCollector(root) {}

  Diag191Collector& operator<<(const TicketingCxrTrx& trx);
  void displaySettlementPlanInfo(const CountrySettlementPlanInfo& cspi);
  void displayCountrySettlementPlanInfo(const CountrySettlementPlanInfo& cspi);
  void displayInterlineAgreements(const CarrierCode& vcxr,
                                  const vcx::ValidatingCxrData& vcxrData,
                                  bool isVcxrPartOfItin,
                                  vcx::TicketType requestedTicketType,
                                  const std::string& failureText,
                                  const vcx::Pos* pos =nullptr);

  void print191Header(const PricingTrx* trx = nullptr,
                      const Itin* itin = nullptr,
                      const std::string& hashString = "",
                      bool isSopProcessing = false,
                      const SopIdVec* sops = nullptr);

  void print191Footer();
  void print191DefaultValidatingCarrier(const PricingTrx& trx , Itin & itin, bool isShopping);
  void printValidatingCxrList(const PricingTrx& trx, const Itin& itin, bool isSopProcessing = false);
  virtual void printDiagMsg(const PricingTrx& trx,
                            const Itin& itin,
                            const CarrierCode& specifiedVC = "");

  void printHeaderForPlausibilityReq(const TicketingCxrTrx& tcsTrx); // header for plausibility command
  void printHeaderForDisplayReq(const TicketingCxrDisplayTrx& tcsTrx); // header for display command
  void printPlausibilityResult(const TicketingCxrTrx& tcsTrx);
  void printPaperConflict(const TicketingCxrTrx& tcsTrx);
  std::string printSops(const PricingTrx* trx, const SopIdVec* sops);

};
}
