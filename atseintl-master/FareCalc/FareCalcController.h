//----------------------------------------------------------------
//
//  File:        FareCalcController
//  Authors:
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

#include "Common/TsePrimitiveTypes.h"

namespace tse
{
class FareCalcConfig;
class FareCalcCollector;
class Itin;
class PricingTrx;

class FareCalcController
{

public:
  static constexpr Indicator DISPLAY_BAGGAGE_ALLOWANCE = '2';

  explicit FareCalcController(PricingTrx& trx) : _trx(trx), _fcConfig(nullptr) {}

  virtual ~FareCalcController() {}

  virtual bool process();

  bool structuredRuleProcess();

  static uint32_t getActiveThreads();

protected:
  PricingTrx& _trx;
  const FareCalcConfig* _fcConfig;
  bool initFareConfig();

  bool performDiagnostic203();
  bool performDiagnostic853();
  bool performDiagnostic856();
  bool performDiagnostic894();
  bool performDiagnostic970();
  bool performDiagnostic980();
  bool performDiagnostic864();

  virtual void processItinFareCalc();

  virtual FareCalcCollector* createFareCalcCollector();

  FareCalcCollector* getFareCalcCollector(const Itin* itin);

  void performDiagnostic804();
  void performDiagnostic983();

private:
  /**
   * Price all itins in _trx.solItinGroupsMap in-place.
   * This function has produces no effect in IS transaction or if isCarnivalSumOfLocal option off.
   *
   * @project Solo Carnival
   */
  void assignPriceToSolItinGroupsMapItems();
};
} // tse namespace

