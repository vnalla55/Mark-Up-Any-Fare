//----------------------------------------------------------------------------
//  File:        Diag198Collector.h
//  Authors:     Mike Carroll
//  Created:     July 2004
//
//  Description: Diagnostic 198 formatter
//
//  Updates:
//          date - initials - description.
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
//----------------------------------------------------------------------------

#pragma once

#include "DataModel/CurrencyTrx.h"
#include "DataModel/MileageTrx.h"
#include "DataModel/PricingOptions.h"
#include "DataModel/PricingRequest.h"
#include "Diagnostic/DiagCollector.h"

namespace tse
{
class Agent;
class AncRequest;
class DiagnosticAutoIndent;
class DiagnosticIndent;
class FareDisplayRequest;
class PricingTrx;
class RexBaseTrx;

class Diag198Collector : public DiagCollector
{
public:
  friend class Diag198CollectorTest;
  //@TODO will be removed, once the transition is done
  explicit Diag198Collector(Diagnostic& root) : DiagCollector(root) {}
  Diag198Collector() = default;

  Diag198Collector& operator<<(const PricingTrx& trx) override;
  Diag198Collector& operator<<(const MileageTrx& trx);
  Diag198Collector& operator<<(const CurrencyTrx& trx);

protected:
  std::string formatAmount(MoneyAmount amount, const CurrencyCode& currencyCode) const;

private:
  void addRequest(const PricingRequest& request, const Trx & trx);
  void endRequest();
  void addOptions(const PricingTrx& trx);
  void addAgent(const Agent& agent);
  void addDefaultAgent(const PricingRequest& request);
  void addTicketingAgent(const PricingRequest& request, const Trx& trx);
  void addConsolidatorPlusUp(const Itin& itin);
  void addExchangePricingDates(const PricingTrx& trx);
  void addExchangeInfo(const PricingTrx& trx);
  void addExchangePlusUpInfo(const RexBaseTrx& trx);
  void addPassengerData(const PaxType* curPT,
                        std::size_t count,
                        DiagCollector& dc,
                        const PricingTrx& trx);
  void addExchangeCat31Info(const PricingTrx& trx);
  void addSnapInfo(const PricingTrx& trx);
  void addAncillaryPricingFeeInfo(const PricingTrx& trx);

  void addAncillaryPriceModifiersToItinInfo(const Itin* itin);
  void addAncillaryPriceModifiersGroup(const Itin::AncillaryPriceModifiersMap::value_type modifiersGroup,
                                       DiagnosticAutoIndent indent);
  void addAncillaryPriceModifier(const AncillaryPriceModifier &modifier,
                                 DiagnosticAutoIndent indent);
  void addAncillaryPriceModifierContent(const AncillaryPriceModifier &modifier,
                                        DiagnosticAutoIndent indent);

  void addAncRequestDetails(const PricingTrx& trx);
  void addONDInfo(const PricingTrx& trx);
  void addBrandInfo(const PricingRequest& request);
  void addItinInfo(const AncRequest& request, const Itin* itin);
  void addItinInfoAddOn(const AncRequest& request, const Itin* itin, const Itin* trxItin);
  void addItinInfo(const PricingTrx& trx);
  void addTravelSegInfo(const TravelSeg* itt);
  void addFlexFareGroupsDataInfo(const PricingTrx& trx);
  void addMultiTicketInd(const PricingTrx& trx);
  void addPbbInd(const PricingTrx& trx);
  void addBrandedFaresTNShoppingInd(const PricingTrx& trx);
  void addSearchForBrandsPricingInd(const PricingTrx& trx);
  void addValidatingCxrInfo(const PricingTrx& trx);
  void addChargesTaxes(const PricingTrx& trx);
  void addDiscountPlusUpInfo(const PricingRequest& request, const Trx& trx);
  void displayCabinInclusionCode(const FareDisplayRequest& request);
  // Tells if the stored trx object is a MIP branding trx
  bool isBrandingTrx() const;
  void addFFGMaxPenaltyData(MaxPenaltyInfo* maxPenaltyInfo);
};

} // namespace tse

