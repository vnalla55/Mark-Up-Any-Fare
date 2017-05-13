//----------------------------------------------------------------------------
//  File:        FareCollectorOrchestratorESV.h
//  Created:     2008-07-01
//
//  Description: ESV fare collector
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

#include "Common/Logger.h"
#include "Common/Thread/TseThreadingConst.h"

namespace tse
{
class FareCollectorOrchestrator;
class FareMarket;
class Itin;
class PaxTypeFare;
class PricingTrx;
class ShoppingTrx;
class TseServer;

class FareCollectorOrchestratorESV
{
public:
  FareCollectorOrchestratorESV(ShoppingTrx& trx,
                               TseServer& server,
                               TseThreadingConst::TaskId taskId);

  ~FareCollectorOrchestratorESV();

  bool process();

private:
  static void setupFareMarket(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  static void allFareMarketStepsESV(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  static void sortStep(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  static void removeDuplicateFares(PricingTrx& trx, Itin& itin, FareMarket& fm);

  static bool findPublishedFares(PricingTrx& trx, Itin& itin, FareMarket& fareMarket);

  static bool dupFare(const PaxTypeFare& ptf1, const PaxTypeFare& ptf2);

  static Logger _logger;
  ShoppingTrx* _trx;
  TseServer* _tseServer;
  const TseThreadingConst::TaskId _taskId;
};

} // End namespace tse

