/*---------------------------------------------------------------------------
 *  File:    ShoppingOrchestrator.h
 *  Created: July 9, 2004
 *  Authors: David White, Adrienne A. Stipe
 *
 *  Change History:
 *
 *  Copyright Sabre 2004
 *    The copyright to the computer program(s) herein
 *    is the property of Sabre.
 *    The program(s) may be used and/or copied only with
 *    the written permission of Sabre or in accordance
 *    with the terms and conditions stipulated in the
 *    agreement/contract under which the program(s)
 *    have been supplied.
 *-------------------------------------------------------------------------*/
/*
 @class ShoppingOrchestrator Documentation
  The main shopping orchestrator for shopping
*--------------------------------------------------------------------------*/
#pragma once

#include "Common/TsePrimitiveTypes.h"

namespace tse
{

class FarePath;
class GroupFarePath;
class MetricsTrx;
class RexShoppingTrx;
class ShoppingTrx;
class TaxResponse;
class TseServer;
class ConfigMan;

class ShoppingOrchestrator
{

public:
  explicit ShoppingOrchestrator(tse::TseServer& server);
  ~ShoppingOrchestrator();

  bool process(MetricsTrx& trx);
  bool process(ShoppingTrx& trx);
  bool process(RexShoppingTrx& trx);

protected:
  ConfigMan& _config;

private:
  ShoppingOrchestrator(const ShoppingOrchestrator& rhs);
  ShoppingOrchestrator& operator=(const ShoppingOrchestrator& rhs);

  const TaxResponse* matchFarePath(FarePath* farePath, ShoppingTrx& trx);
  MoneyAmount updateFarePath(GroupFarePath* gfp,
                             FarePath* farePath,
                             const TaxResponse* taxResponse,
                             ShoppingTrx& trx);
};

} /* end namespace tse */

