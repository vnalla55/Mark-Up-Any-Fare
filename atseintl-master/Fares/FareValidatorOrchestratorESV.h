//----------------------------------------------------------------------------
//  File:        FareValidatorOrchestratorESV.h
//  Created:     2008-04-16
//
//  Description: Orchestrator class for ESV fares validation
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
#include "Fares/GovCxrValidatorESV.h"

#include <vector>

namespace tse
{
class TseServer;
class ShoppingTrx;

class FareValidatorOrchestratorESV
{
public:
  FareValidatorOrchestratorESV(ShoppingTrx& trx, TseServer& server);

  ~FareValidatorOrchestratorESV();

  bool process();

private:
  bool runAllGovCarrierThreads(std::deque<GovCxrValidatorESV>& tasks);

  void initializeMap();

  static Logger _logger;
  ShoppingTrx* _trx;
  TseServer* _tseServer;
  static const std::string _default_validation_order;
  std::vector<uint32_t> _rule_validation_order;
};

} // End namespace tse

