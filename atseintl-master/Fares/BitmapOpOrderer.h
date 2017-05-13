//----------------------------------------------------------------------------
//  File:        BitmapOpOrderer.h
//  Created:     2005-01-01
//
//  Description: Booking code, routing and flight related validation
//
//  Updates:
//
//  Copyright Sabre 2008
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

#include "BookingCode/BCETuning.h"
#include "DataModel/ItinIndex.h"
#include "DataModel/ShoppingTrx.h"
#include "Fares/FareOrchestrator.h"
#include "Fares/RoutingController.h"
#include "Routing/RoutingInfo.h"


#include <iostream>
#include <vector>

namespace tse
{
class FareMarket;
class FareMarketRuleController;
class MetricsTrx;
class PricingTrx;
class TravelRoute;

class BitmapOpOrderer
{
private:
  std::vector<char> _operationOrder;

private:
  bool doGlobalDirection(PaxTypeFare& curFare,
                         const uint32_t& bitIndex,
                         const GlobalDirection globalDirection);

  bool doBookingCodeValidation(ShoppingTrx& trx,
                               PaxTypeFare& curFare,
                               FareMarket& fM,
                               const uint32_t& bitIndex,
                               Itin& journeyItin,
                               std::vector<BCETuning>& bceTuningData);

  bool doBitmapCatValidations(PricingTrx& pTrx,
                              PaxTypeFare& curFare,
                              FareMarket& fM,
                              FareMarketRuleController& ruleController,
                              const uint32_t& bitIndex,
                              Itin& journeyItin,
                              ItinIndex::ItinCellInfo& curItinCellInfo);

  bool doQualCat4Validations(PricingTrx& pTrx,
                             PaxTypeFare& curFare,
                             FareMarketRuleController& cat4RuleController,
                             const uint32_t& bitIndex,
                             Itin& journeyItin);

  bool doRouting(ShoppingTrx& trx,
                 PaxTypeFare& curFare,
                 FareMarket& fM,
                 ItinIndex::ItinCellInfo& curItinCellInfo,
                 const uint32_t& bitIndex,
                 ShoppingRtgMap& rtMap);

  void doPublishedRouting(ShoppingTrx& trx,
                          FareMarket& fM,
                          const uint32_t& bitIndex,
                          ShoppingRtgMap& rtMap);

public:
  BitmapOpOrderer(const tse::ConfigMan& config);

  void performBitmapOperations(ShoppingTrx& trx,
                               PaxTypeFare& curFare,
                               FareMarket& fM,
                               const uint32_t& bitIndex,
                               Itin& journeyItin,
                               ItinIndex::ItinCellInfo& curItinCellInfo,
                               const bool bookingCodeBeforeRule,
                               FareMarketRuleController& ruleController,
                               FareMarketRuleController& cat4RuleController,
                               ShoppingRtgMap& rtMap,
                               std::vector<BCETuning>& bceTuningData,
                               bool bkcProcesed = false,
                               bool rtgProcessed = false,
                               bool glbProcessed = false);
  void doPublishedRoutingDelayed(ShoppingTrx& trx,
                                 RoutingController& shoppingRoutingController,
                                 FareMarket& fM,
                                 PaxTypeFare& curFare,
                                 ShoppingRtgMap& rtMap);
};
} // namespace tse

