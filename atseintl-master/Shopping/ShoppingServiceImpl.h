//----------------------------------------------------------------------------
//
//        File: ShoppingServiceImpl.h
// Description: Shopping service class
//     Created: 07/09/2004
//     Authors: Adrienne Stipe, David White
//
// Copyright Sabre 2004
//
//     The copyright to the computer program(s) herein
//     is the property of Sabre.
//     The program(s) may be used and/or copied only with
//     the written permission of Sabre or in accordance
//     with the terms and conditions stipulated in the
//     agreement/contract under which the program(s)
//     have been supplied.
//
//----------------------------------------------------------------------------
#pragma once

#include "DataModel/RexShoppingTrx.h"
#include "Service/Service.h"
#include "Shopping/ShoppingOrchestrator.h"

namespace tse
{
class Trx;
class TseServer;
class ShoppingServiceImpl;
class FlightFinderTrx;
class ConfigMan;

class ShoppingServiceImpl final : public Service
{

public:
  ShoppingServiceImpl(const std::string& name, TseServer& server);

  bool initialize(int argc = 0, char* argv[] = nullptr) override { return true; }

  // Add overrides of the various process() methods here

  bool process(ShoppingTrx& trx) override;
  bool process(MetricsTrx& trx) override;
  bool process(FlightFinderTrx& trx) override;
  bool process(RexShoppingTrx& trx) override;

private:
  ConfigMan& _config;

  ShoppingOrchestrator _orchestrator;
};
} /* end namespace tse */
